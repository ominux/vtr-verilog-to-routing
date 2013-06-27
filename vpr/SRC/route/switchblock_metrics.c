
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "vpr_types.h"
#include "vpr_utils.h"
#include "switchblock_metrics.h"


static void get_num_fpga_blocks(OUTP int *num_fpga_blocks, INP int num_block_types, 
		INP struct s_grid_tile **fpga_grid, INP int size_x, INP int size_y);

static float get_pin_homogeneity(INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent);

static float get_wire_homogeneity(INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP boolean both_sides);



int get_num_wire_types(INP int num_segments, INP t_segment_inf *segment_inf){

	int num_wire_types = 0;

	if (num_segments > 1){
		/* Segments of one length are grouped together in the channel.	*
		*  In the future we can determine if any of these segments will	*
		*  encounter the pathological step size case, and determine if	*
		*  we need to perturb based on the segment's frequency (if 	*
		*  frequency is small we should not perturb - testing has found	*
		*  that perturbing a channel when unnecessary increases needed	*
		*  W to achieve the same delay); but for now we just return.	*/
	} else {
		/* There are as many wire start points as the value of L */
		num_wire_types = segment_inf[0].length;
	}

	return num_wire_types;
}


/* Gets the connection block homogeneity according to the pin<->wire connections
*  specified in tracks_connected_to_pin. Only done for the pin_type specified. */
t_conn_block_homogeneity get_conn_block_homogeneity(INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int *Fc_array, INP int nodes_per_chan, INP int num_segments, 
		INP t_segment_inf *segment_inf){
	
	/* can not have diversity for open pins */
	assert(OPEN != pin_type);	
	
	int i, Fc, iclass, pin, num_pin_type_pins;
	i = Fc = iclass = pin = num_pin_type_pins = 0;

	t_conn_block_homogeneity conn_block_homogeneity;
	
	conn_block_homogeneity.pin_homogeneity = 0;
	conn_block_homogeneity.wire_homogeneity = 0;

	int num_wire_types = get_num_wire_types(num_segments, segment_inf);
	
	/* get number of pin_type_pins */
	if (DRIVER == pin_type){
		num_pin_type_pins = block_type->num_drivers;
	} else if (RECEIVER == pin_type){
		num_pin_type_pins = block_type->num_receivers;
	} else {
		assert(FALSE);
	}
	
	/* Find max Fc */
	for (pin = 0; pin < block_type->num_pins; ++pin) {
		iclass = block_type->pin_class[pin];
		if (Fc_array[i] > Fc && block_type->class_inf[iclass].type == pin_type) {
			Fc = Fc_array[pin];
		}
	}

	if (0 == block_type->index){
		/* can not have homogeneity for empty blocks */
	} else if (num_wire_types <= 0){
		/* so far we can only compute homogeneity for single-segment (i.e. track length) fpga's	*/
	} else if (Fc == 0){
		/* can not have homogeneity for open pins */
	} else {
		/* get the diversity metrics */
		boolean both_sides = FALSE;
		if (0 == strcmp("clb", block_type->name)){	/* weirdly, 0 indicates true */
			/* many CLBs are adjacent to eachother, so connections from one CLB	*
			*  will interfere with its neighbor. We'd like to take this into	*
			*  account for the applicable metrics.					*/
			both_sides = TRUE;
		} else {
			both_sides = FALSE;
		}

		conn_block_homogeneity.pin_homogeneity = get_pin_homogeneity(block_type, tracks_connected_to_pin, 
								pin_type, Fc, nodes_per_chan, num_wire_types,
								num_pin_type_pins, 1);

		conn_block_homogeneity.wire_homogeneity = get_wire_homogeneity(block_type, tracks_connected_to_pin,
								pin_type, Fc, nodes_per_chan, num_wire_types,
								num_pin_type_pins, 1, both_sides); 
	}	

	return conn_block_homogeneity;
}

/* Returns the wire homogeneity of a block's connection to tracks */
static float get_wire_homogeneity(INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP boolean both_sides){
	
	float total_wire_homogeneity = 0;
	int **wire_conns = (int **) alloc_matrix(0, 3, 0, nodes_per_chan - 1, sizeof(int));
	float *wire_homogeneity = (float *) my_malloc(sizeof(float) * 4);

	int i, pin, side, height, width, counted_pins_per_side[4];
	int counted_pins = 0;
	int track = 0;
	for (side = 0; side < 4; side++){
		counted_pins_per_side[side] = 0;
		for (i = 0; i < nodes_per_chan; i++){
			wire_conns[side][i] = 0;
		}
	}	

	/*  Count the number of times each wire is used on each side */ 
	for (side = 0; side < 4; side++){
		/* over each height unit */
		for (height = 0; height < block_type->height; height++){
			/*  over each width unit */
			for (width = 0; width < block_type->width; width++){
				/* over each pin */
				for (pin = 0; pin < block_type->num_pins; pin++){
					/* only doing pin_type pins */
					if (block_type->class_inf[block_type->pin_class[pin]].type != pin_type){
						continue;
					}

					if (counted_pins == num_pin_type_pins){
						/* Some blocks like io appear to have four sides, but only one	*
						*  of those sides is actually used in practice. So here we try	*
						*  not to count the unused pins.				*/
						break;
					}
					
					/* check that pin has connections at this height/width/side */
					track = tracks_connected_to_pin[pin][width][height][side][0];
					if (OPEN == track){
						continue;
					}
					
					wire_conns[side][track]++;
					
					for(i = 1; i < Fc; i++){
						/* get next track */
						track = tracks_connected_to_pin[pin][width][height][side][i];
						/* find the number of times a connection is made with each wire/side */
						wire_conns[side][track]++; 
					}
					counted_pins++;
					counted_pins_per_side[side]++;
				}
			}
		} 
	}

	/* determine the homogeneity of each wire.*/ 
	int unconnected_wires = 0;
	int total_conns = 0;
	float mean = 0;
	float wire_homogeneity_temp = 0;
	int mult = (both_sides) ? 2 : 1;
	//if both sides, we only do two sides.
	/* sides must be ordered as TOP, RIGHT, BOTTOM, LEFT. see the e_side enum */
	for (side = 0; side < (4 / mult); side++){
		mean = 0;
		unconnected_wires = 0;
		total_conns = 0;
		for (i = 0; i < mult; i++){
			total_conns += (float)counted_pins_per_side[side + mult*i];
		}
		total_conns *= Fc;
		unconnected_wires = (total_conns) ? std::max(0, nodes_per_chan - total_conns)  :  0 ;
		mean = (float)total_conns / (float)(nodes_per_chan - unconnected_wires);
		//printf("total_conns %d  mean: %f   unconnected: %d\n", total_conns, mean, unconnected_wires);
		wire_homogeneity[side] = 0;
		for (track = 0; track < nodes_per_chan; track++){
			wire_homogeneity_temp = 0;
			for (i = 0; i < mult; i++){
				if (counted_pins_per_side[side + i*mult] > 0){
					/* only include sides with connected pins */
					wire_homogeneity_temp += (float)wire_conns[side + i*mult][track];
				}
			}
			wire_homogeneity[side] += fabs(wire_homogeneity_temp - mean);
			//printf("side: %d   conns: %d  WH: %f\n", side, wire_conns[side][track], wire_homogeneity[side]);
		}
		wire_homogeneity[side] -= unconnected_wires * mean;
		wire_homogeneity[side] /= (float)(2*Fc);
		total_wire_homogeneity += wire_homogeneity[side];
	}
	total_wire_homogeneity /= num_pin_type_pins;

	//for (side = 0; side < 4; side++){
	//	mean = (float)(counted_pins_per_side[side] * Fc) / (float)nodes_per_chan;
	//	if (counted_pins_per_side[side] > 0){
	//		unconnected_wires = nodes_per_chan - (counted_pins_per_side[side] * Fc);
	//	} else {
	//		/* this side is not connected to any wires; skip it */
	//		unconnected_wires = 0;
	//		continue;
	//	}
	//	
	//	wire_homogeneity[side] = 0;
	//	for (track = 0; track < nodes_per_chan; track++){
	//		wire_homogeneity[side] += fabs((float)wire_conns[side][track] - mean);
	//		//printf("track: %d   WH: %f   mean: %f   wire_conns: %d\n", track, wire_homogeneity[side], mean, wire_conns[side][track]); 	
	//	}
	//	/* correct for wires that are naturally unconnected if mean < 1 */
	////	printf("unconnected: %d  sum: %f\n", unconnected_wires, wire_homogeneity[side]);
	//	wire_homogeneity[side] -= std::max(0, unconnected_wires);
	//	wire_homogeneity[side] /= (float)Fc;
	//	total_wire_homogeneity += wire_homogeneity[side];
	//}
	///* normalize block WH to [0,1] range */
	//total_wire_homogeneity /= num_pin_type_pins;

	free_matrix(wire_conns, 0, 3, 0, sizeof(int));
	free(wire_homogeneity);
	wire_conns = NULL;
	wire_homogeneity = NULL;

	return total_wire_homogeneity;
}


/* returns the pin homogeneity of a block's connection to tracks */
static float get_pin_homogeneity(INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent){

	float total_pin_homogeneity = 0;
	int **pin_averages = (int **) alloc_matrix(0, block_type->num_pins - 1, 0, num_wire_types - 1, sizeof(int));
	float *pin_homogeneity = (float *) my_malloc(sizeof(float) * block_type->num_pins);

	int i, pin, side, height, width;
	int track = 0;
	int counted_pins = 0;
	for (pin = 0; pin < block_type->num_pins; pin++){
		for (i = 0; i < num_wire_types; i++){
			pin_averages[pin][i] = 0;
		}
	}

	/* Count the number of times each pin connects to each wire class */
	for (side = 0; side < 4; side++){
		/* over each height unit */
		for (height = 0; height < block_type->height; height++){
			/*  over each width unit */
			for (width = 0; width < block_type->width; width++){
				/* over each pin */
				for (pin = 0; pin < block_type->num_pins; pin++){
					/* only doing pin_type pins */
					if (block_type->class_inf[block_type->pin_class[pin]].type != pin_type){
						continue;
					}

					if (counted_pins == num_pin_type_pins){
						/* Some blocks like io appear to have four sides, but only one	*
						*  of those sides is actually used in practice. So here we try	*
						*  not to count the unused pins.				*/
						break;
					}
					
					/* check that pin has connections at this height/width/side */
					track = tracks_connected_to_pin[pin][width][height][side][0];
					if (OPEN == track){
						continue;
					}
					
					pin_averages[pin][track % num_wire_types]++;
					
					for(i = 1; i < Fc; i++){
						/* get next track */
						track = tracks_connected_to_pin[pin][width][height][side][i];
						/* find the number of times the pin connects to each wire class */
						pin_averages[pin][track % num_wire_types]++;
					}
					counted_pins++;
				}
			}
		} 
	}

	//printf("counted: %d  drivers: %d  receivers: %d  num_pins: %d  num_class: %d\n", counted_pins, block_type->num_drivers, block_type->num_receivers, block_type->num_pins, block_type->num_class);
	/* determine the homogeneity of each pin. */
	int unconnected_types = Fc % num_wire_types;
	float mean = (float)Fc / (float)(num_wire_types);
	for (pin = 0; pin < block_type->num_pins; pin++){
		/* only doing pin_type pins */
		if (block_type->class_inf[block_type->pin_class[pin]].type != pin_type){
			continue;
		}
		
		pin_homogeneity[pin] = 0;
		for (i = 0; i < num_wire_types; i++){
			pin_homogeneity[pin] += abs((float)pin_averages[pin][i] - mean);
		}
		//TODO pin_homogeneity[pin -= unconnected_types
		pin_homogeneity[pin] /= (float)(2 * Fc);
		total_pin_homogeneity += pin_homogeneity[pin];
	}
	/* normalize block PH to [0,1] range */
	total_pin_homogeneity /= num_pin_type_pins;
	
	free_matrix(pin_averages, 0, block_type->num_pins - 1, 0, sizeof(int));
	free(pin_homogeneity);
	pin_averages = NULL;
	pin_homogeneity = NULL;

	return total_pin_homogeneity;
}


//float get_hamming_distance


/* Uses homogeneity computed for each individual block to compute the wire/pin homogeneity over	*
*  the entire FPGA										*/
t_conn_block_homogeneity get_conn_block_homogeneity_fpga(INP t_conn_block_homogeneity *block_homogeneity, 
		INP int num_block_types, INP struct s_grid_tile **fpga_grid, INP int size_x, INP int size_y,
		INP t_type_ptr block_types, INP e_pin_type pin_type){
	
	int *num_fpga_blocks;
	t_conn_block_homogeneity fpga_homogeneity;
	fpga_homogeneity.pin_homogeneity = 0;
	fpga_homogeneity.wire_homogeneity = 0;

	/* iterate over FPGA grid and determine number of different blocks */
	num_fpga_blocks = (int *) my_malloc(num_block_types * sizeof(int));
	get_num_fpga_blocks(OUTP num_fpga_blocks, num_block_types, fpga_grid, size_x, size_y);

	/* can not compute homogeneity for open pins */
	assert( OPEN != pin_type );
	
	int iblock;
	int num_pins, num_blocks_used, tot_block_pins, total_fpga_pins;
	num_pins = 0;
	total_fpga_pins = 0;
	for (iblock = 0; iblock < num_block_types; iblock++){
		/* block index should be same as position in block_types array */
		assert( iblock == block_types[iblock].index );
		
		if ( DRIVER == pin_type ){
			num_pins = block_types[iblock].num_drivers;
		} else if (RECEIVER == pin_type) {
			num_pins = block_types[iblock].num_receivers;
		} /* OPEN == pin_type taken care of by assert before */
		
		num_blocks_used = num_fpga_blocks[iblock];
		tot_block_pins = num_blocks_used * num_pins;

		/* weigh each block's homogeneity by total amount of pins it has in the FPGA */
		fpga_homogeneity.pin_homogeneity += block_homogeneity[iblock].pin_homogeneity * tot_block_pins;//num_blocks_used;
		fpga_homogeneity.wire_homogeneity += block_homogeneity[iblock].wire_homogeneity * tot_block_pins;//num_blocks_used;
		total_fpga_pins += tot_block_pins;
		//printf("block %s  ph %f  wh %f  num_blocks_used %d  num_pins %d\n", block_types[iblock].name,
		//	block_homogeneity[iblock].pin_homogeneity, block_homogeneity[iblock].wire_homogeneity, 
		//	num_blocks_used, num_pins);
	}
	/* normalize metrics to once again be <= 1 */
	fpga_homogeneity.pin_homogeneity /= total_fpga_pins;
	fpga_homogeneity.wire_homogeneity /= total_fpga_pins;

	free(num_fpga_blocks);
	num_fpga_blocks = NULL;

	return fpga_homogeneity;
}

/* returns the number of times each each block occurs in the FPGA */
static void get_num_fpga_blocks(OUTP int *num_fpga_blocks, INP int num_block_types, 
		INP struct s_grid_tile **fpga_grid, INP int size_x, INP int size_y){

	/* initialize array */
	for (int i = 0; i < num_block_types; i++){
		num_fpga_blocks[i] = 0;
	} 	
	
	/* iterate over the entire FPGA grid and count up the number of each block */
	int ix, iy, block_index, block_usage;
	for (ix = 0; ix < size_x+1; ix++){
		for (iy = 0; iy < size_y+1; iy++){
			if (fpga_grid[ix][iy].width_offset > 0 &&
			    fpga_grid[ix][iy].height_offset > 0){
				continue;	/* not the parent block */
			} else {
				block_index = fpga_grid[ix][iy].type->index;
				block_usage = std::max(fpga_grid[ix][iy].usage, 1);	/* TODO: check if large blocks always have usage=0 */
				num_fpga_blocks[block_index] += block_usage;	
			}
		}	
	} 

	return;
}






