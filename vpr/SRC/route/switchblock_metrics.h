#ifndef SWITCHBLOCK_METRICS_H
#define SWITCHBLOCK_METRICS_H


/* Contains statistics on a connection block's diversity 
*  pin_homogeniety: measure of how non-diverse a pin's connection to wires is
*  wire_homogeniety: measure of how non-diverse the connections across the wires are */ 
typedef struct s_conn_block_homogeneity {
	float pin_homogeneity;
	float wire_homogeneity;
	//float hamming_distance;
} t_conn_block_homogeneity;


int get_num_wire_types(INP int num_segments, INP t_segment_inf *segment_inf);

t_conn_block_homogeneity get_conn_block_homogeneity(INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int *Fc_array, INP int nodes_per_chan, INP int num_segments, 
		INP t_segment_inf *segment_inf);

t_conn_block_homogeneity get_conn_block_homogeneity_fpga(INP t_conn_block_homogeneity *block_homogeneity, 
		INP int num_block_types, INP struct s_grid_tile **fpga_grid, INP int size_x, INP int size_y,
		INP t_type_ptr block_types, INP e_pin_type pin_type);


#endif /*SWITCHBLOCK_METRICS_H*/
