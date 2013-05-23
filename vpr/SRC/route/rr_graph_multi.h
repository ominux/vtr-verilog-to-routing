#ifndef RR_GRAPH_MULTI_H
#define RR_GRAPH_MULTI_H

/* 
 * Function that does num_cuts horizontal cuts to the chip,
 * percent_wires_cut% of the wires crossing these cuts are removed
 * (have their capacity reduced to zero actually) and the remaining
 * wires on this section have their delay increased by delay increase
 * TODO
 */
void cut_rr_graph_edges(INP int nodes_per_chan, INP t_seg_details * seg_details,
			INOUTP t_rr_node * L_rr_node, INP t_ivec *** L_rr_node_indices,
			INP enum e_directionality directionality, INP int percent_wires_cut,
			INP int num_cuts, INP int delay_increase);

void cut_rr_graph_capacity(INP int nodes_per_chan, INP t_seg_details * seg_details,
			INOUTP t_rr_node * L_rr_node, INP t_ivec *** L_rr_node_indices,
			INP enum e_directionality directionality, INP int percent_wires_cut,
			INP int num_cuts, INP int delay_increase);

#endif
