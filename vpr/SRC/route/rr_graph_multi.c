#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "vpr_types.h"
#include "globals.h"
#include "rr_graph_util.h"
#include "rr_graph.h"
#include "rr_graph2.h"
#include "rr_graph_multi.h"
#include "rr_graph_sbox.h"
#include "check_rr_graph.h"
#include "rr_graph_timing_params.h"
#include "rr_graph_indexed_data.h"
#include "vpr_utils.h"

/* 
 * Function that does num_cuts horizontal cuts to the chip,
 * percent_wires_cut% of the wires crossing these cuts are removed
 * (the edges which cross the cut are removed) and the remaining
 * wires on this section have their delay increased by delay_increase
 */
void cut_rr_graph_edges(INP int nodes_per_chan, INP t_seg_details * seg_details,
			INOUTP t_rr_node * L_rr_node, INP t_ivec *** L_rr_node_indices,
			INP enum e_directionality directionality, INP int L_percent_wires_cut,
			INP int L_num_cuts, INP int L_delay_increase);


/* ---------------------------------------------------------------------------
 * 				Functions begin 
 * ---------------------------------------------------------------------------*/

/*
 * Function which cuts the edges which cross the cut for a given wire in the CHANY
 */
void cut_rr_yedges(INP int cut_location, INP int inode, INOUTP t_rr_node *L_rr_node){
	
	int iedge, d_node, num_removed;
	int tmp;

	num_removed = 0;

	/* mark and remove the edges */
	for(iedge = 0; iedge < L_rr_node[inode].num_edges; iedge++){
		d_node = L_rr_node[inode].edges[iedge];
		if(d_node == -1)
			continue;
		/* crosses the cut line, cut this edge */
		if((L_rr_node[inode].direction == INC_DIRECTION && L_rr_node[d_node].ylow > cut_location)
		|| (L_rr_node[inode].direction == DEC_DIRECTION && L_rr_node[d_node].ylow <= cut_location)){
			L_rr_node[d_node].fan_in--;
			num_removed++;
			for(tmp = iedge; tmp+1 < L_rr_node[inode].num_edges; tmp++){
				L_rr_node[inode].edges[tmp] = L_rr_node[inode].edges[tmp+1];
				L_rr_node[inode].switches[tmp] = L_rr_node[inode].switches[tmp+1];
			}
			L_rr_node[inode].edges[tmp] = -1; /* tmp = num_edges-1 */
			L_rr_node[inode].switches[tmp] = -1;

			iedge--; /* need to check the value just pulled into current pos */
		}
		else{
			/*printf(">>>> Did not cut this edge because it does not cross the boundary <<<<\n");*/
		}
	}

	/* fill the rest of the array with -1 for safety */
	for(iedge = L_rr_node[inode].num_edges - num_removed; iedge < L_rr_node[inode].num_edges; iedge++){
		L_rr_node[inode].edges[iedge] = -1;
		L_rr_node[inode].switches[iedge] = -1;
	}
	L_rr_node[inode].num_edges -= num_removed;
	/* finished removing the edges */
}

/*
 * Function which cuts the edges which cross the cut for a given wire in the CHANX
 */
void cut_rr_xedges(INP int cut_location, INP int inode, INOUTP t_rr_node *L_rr_node){
	
	int iedge, d_node, num_removed;
	int tmp;

	num_removed = 0;

	/* mark and remove the edges */
	for(iedge = 0; iedge < L_rr_node[inode].num_edges; iedge++){
		d_node = L_rr_node[inode].edges[iedge];
		if(d_node == -1)
			continue;
		/* crosses the cut line, cut this edge, CHANX is always supposed to be
		 * below the cutline */
		if(L_rr_node[d_node].ylow > cut_location
		&& L_rr_node[d_node].type == CHANY){
			L_rr_node[d_node].fan_in--;
			num_removed++;
			for(tmp = iedge; tmp+1 < L_rr_node[inode].num_edges; tmp++){
				L_rr_node[inode].edges[tmp] = L_rr_node[inode].edges[tmp+1];
				L_rr_node[inode].switches[tmp] = L_rr_node[inode].switches[tmp+1];
			}
			L_rr_node[inode].edges[tmp] = -1; /* tmp = num_edges-1 */
			L_rr_node[inode].switches[tmp] = -1;

			iedge--; /* need to check the value just pulled into current pos */
		}
		else{
			/*printf(">>>> Did not cut this edge because it does not cross the boundary <<<<\n");*/
		}
	}

	/* fill the rest of the array with -1 for safety */
	for(iedge = L_rr_node[inode].num_edges - num_removed; iedge < L_rr_node[inode].num_edges; iedge++){
		L_rr_node[inode].edges[iedge] = -1;
		L_rr_node[inode].switches[iedge] = -1;
	}
	L_rr_node[inode].num_edges -= num_removed;
	/* finished removing the edges */
}


/*
 * Function which increases the delay of the edges which cross the cut for a given wire in the CHANY
 */
void increase_delay_rr_yedges(INP int cut_location, INP int inode, INOUTP t_rr_node *L_rr_node){
	
	int iedge, d_node;

	/* mark and remove the edges */
	for(iedge = 0; iedge < L_rr_node[inode].num_edges; iedge++){
		d_node = L_rr_node[inode].edges[iedge];

		/* crosses the cut line, increase delay */
		if((L_rr_node[inode].direction == INC_DIRECTION && L_rr_node[d_node].ylow > cut_location)
		|| (L_rr_node[inode].direction == DEC_DIRECTION && L_rr_node[d_node].ylow <= cut_location)){
			/* increased_delay_edge_map is the array which stores
			 * what is the corresponding version with increased delay
			 * for each kind of switch, so to increase the delay I just
			 * need to substitute the switch type of the edge with the
			 * increased delay one */
			if(L_rr_node[inode].switches[iedge] ==
				increased_delay_edge_map[L_rr_node[inode].switches[iedge]]){
				//printf("Switch type:%d\n", L_rr_node[inode].switches[iedge]);
			}
			else{
				//printf("Switch type:%d\n", L_rr_node[inode].switches[iedge]);
			}
			/*assert(L_rr_node[inode].switches[iedge] !=
				increased_delay_edge_map[L_rr_node[inode].switches[iedge]]);*/
			L_rr_node[inode].switches[iedge] =
				increased_delay_edge_map[L_rr_node[inode].switches[iedge]];
		}
	}
}

/* 
 * Function that does num_cuts horizontal cuts to the chip,
 * percent_wires_cut% of the wires crossing these cuts are removed
 * (the edges which cross the cut are removed) and the remaining
 * wires on this section have their delay increased by delay_increase (ns)
 */
void cut_rr_graph_edges(INP int nodes_per_chan, INP t_seg_details * seg_details,
		INOUTP t_rr_node * L_rr_node, INP t_ivec *** L_rr_node_indices,
		INP enum e_directionality directionality, INP int L_percent_wires_cut,
		INP int L_num_cuts, INP int L_delay_increase){

	int num_wires_cut, cur_wires_cut_up, cur_wires_cut_down;
	int num_wires_cut_border;
	int cut_step;
	int itrack, inode;
	int i, j, counter;
	int tj;
	int step;

	if(directionality == BI_DIRECTIONAL) /* Ignored for now TODO */
		return;

	/* Number of wires that should be cut at each horizontal cut */
	num_wires_cut = nodes_per_chan * L_percent_wires_cut;
	assert(L_percent_wires_cut == 0 || num_wires_cut >= nodes_per_chan); /* to catch overflows */
	num_wires_cut = num_wires_cut / 100;

	/* Number of wires to be cut in the case when ylow = ycut+1 and
	 * direction = DEC_DIRECTION (wire is going down and has edges 
	 * in the switch box going through the cut */
	num_wires_cut_border = num_wires_cut / (2*4); // 4 = wirelength

	/* the interval at which the cuts should be made */
	cut_step = ny / (L_num_cuts + 1);


	/* Number of cuts already made */
	counter = 0;
	if(num_wires_cut == 0)
		step = 100000000;
	else
		step = nodes_per_chan / num_wires_cut;

	for(j = cut_step; j < ny && counter < L_num_cuts; j+=cut_step){
		for(i = 0; i <= nx; i++){
			cur_wires_cut_up = cur_wires_cut_down = 0;

			for(itrack = 0; itrack < nodes_per_chan; itrack++){
				inode = get_rr_node_index(i, j, CHANY, itrack, L_rr_node_indices);

				if(L_rr_node[inode].direction == INC_DIRECTION){
					if((itrack/2) % step != 0 || 
					cur_wires_cut_up >= num_wires_cut/2){
						/* Increase delay of this wire */
						increase_delay_rr_yedges(j, inode, L_rr_node);
						continue;
					}

					cur_wires_cut_up++;
				}
				else{
					assert(L_rr_node[inode].direction == DEC_DIRECTION);
					if((itrack/2) % step != 0 || 
					cur_wires_cut_down >= num_wires_cut/2){
						/* Increase delay of this wire */
						increase_delay_rr_yedges(j, inode, L_rr_node);
						continue;
					}

					cur_wires_cut_down++;
				}

				/* actually cut the edges */
				cut_rr_yedges(j, inode, L_rr_node); 
			}

			assert(cur_wires_cut_down >= num_wires_cut/2);
			assert(cur_wires_cut_up >= num_wires_cut/2);

			/* Now cut edges at the switch box, from CHANX to CHANY
			   and from CHANY to CHANY (the case where ylow = y_cut+1
			   and direction = DEC_DIRECTION) */
			tj = j+1;
			if(tj >= ny)
				continue;

			cur_wires_cut_down = 0;
			/* From CHANY to other channels when ylow = ycut+1 */
			for(itrack = 0; itrack < nodes_per_chan; itrack++){
				inode = get_rr_node_index(i, tj, CHANY, itrack, L_rr_node_indices);

				if(L_rr_node[inode].direction == DEC_DIRECTION
				&& L_rr_node[inode].ylow == tj){
					if(cur_wires_cut_down < num_wires_cut_border){
						cut_rr_yedges(j, inode, L_rr_node);
						cur_wires_cut_down++;
					}
					else{
						/* Increase delay of this wire*/
						increase_delay_rr_yedges(j, inode, L_rr_node);
					}
				}
			}

			/* From CHANX to CHANY, cut only the edges at the switches */
			if(i > 0 && i < nx){
				for(itrack = 0; itrack < nodes_per_chan; itrack++){
					inode = get_rr_node_index(i, j, CHANX, itrack, L_rr_node_indices);

					//printf("Cutting edges between CHANX and CHANY\n");
					cut_rr_xedges(j, inode, L_rr_node);
				}
			}
		}
		counter++;
	}
	assert(counter == L_num_cuts);
}


void cut_rr_graph_capacity(INP int nodes_per_chan, INP t_seg_details * seg_details,
		INOUTP t_rr_node * L_rr_node, INP t_ivec *** L_rr_node_indices,
		INP enum e_directionality directionality, INP int L_percent_wires_cut,
		INP int L_num_cuts, INP int L_delay_increase){

	int num_wires_cut, cur_wires_cut_up, cur_wires_cut_down;
	int cut_step;
	int itrack, inode;
	int i, j, counter;
	int step;

	if(directionality == BI_DIRECTIONAL) /* Ignored for now TODO */
		return;

	/* Number of wires that should be cut at each horizontal cut */
	num_wires_cut = nodes_per_chan * L_percent_wires_cut;
	assert(L_percent_wires_cut == 0 || num_wires_cut >= nodes_per_chan); /* to catch overflows */
	num_wires_cut = num_wires_cut / 100;

	/* the interval at which the cuts should be made */
	cut_step = ny / (L_num_cuts + 1);
	/*printf(">>>>>>>>>>>>>Got to the beginning of cut <<<<<<<<<<<<<<<\n");
	printf("nodes_per_chan:%d num_wires_cut:%d\n", nodes_per_chan, num_wires_cut);*/

	counter = 0;
	if(num_wires_cut == 0)
		step = 100000000;
	else
		step = nodes_per_chan / num_wires_cut;

	/* old version which simply reduces the capacity to 0 */
	/* version that cuts wires equally from each channel, in steps */
	counter = 0;
	step = nodes_per_chan / num_wires_cut;
	for(j = cut_step; j < ny && counter < L_num_cuts; j+=cut_step){
		for(i = 0; i <= nx; i++){
			cur_wires_cut_up = cur_wires_cut_down = 0;
			for(itrack = 0; itrack < nodes_per_chan; itrack++){
				inode = get_rr_node_index(i, j, CHANY, itrack, L_rr_node_indices);

				if(directionality == BI_DIRECTIONAL){
					if(cur_wires_cut_down >= num_wires_cut)
						break;
					if(itrack % step == 0){
						cur_wires_cut_down++;
						/* Just reducing the capacity to 0, may want to remove edge to reduce memory later */
						L_rr_node[inode].capacity = 0;
					}
				}
				else{
					if(L_rr_node[inode].direction == INC_DIRECTION){
						if(cur_wires_cut_up >= num_wires_cut/2)
							continue;
						if((itrack/2) % step == 0){
							cur_wires_cut_up++;
							/* Just reducing the capacity to 0, may want to remove edge to reduce memory later */
							L_rr_node[inode].capacity = 0;
						}
					}
					else{
						assert(L_rr_node[inode].direction == DEC_DIRECTION);
						if(cur_wires_cut_down >= num_wires_cut/2)
							continue;
						if((itrack/2) % step == 0){
							cur_wires_cut_down++;
							/* Just reducing the capacity to 0, may want to remove edge to reduce memory later */
							L_rr_node[inode].capacity = 0;
						}
					}
				}
			}
			if(directionality != BI_DIRECTIONAL){
				assert(cur_wires_cut_down >= num_wires_cut/2);
				assert(cur_wires_cut_up >= num_wires_cut/2);
			}
		}
		counter++;
	}
	assert(counter == L_num_cuts);
}
