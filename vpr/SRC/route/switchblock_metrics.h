#ifndef SWITCHBLOCK_METRICS_H
#define SWITCHBLOCK_METRICS_H

/**** Classes ****/
/* constructor and destructor for a 2d array */
class int_array_2d{
private:	
	bool already_set;
	int sizex;
	int sizey;

public:
	int **ptr;

	int_array_2d(){
		sizex = 0;
		sizey = 0;
		already_set = false;
	}
	int_array_2d(int x, int y){
		sizex = x;
		sizey = y;
		ptr = (int **) alloc_matrix(0, x-1, 0, y-1, sizeof(int));
		already_set = true;
	}
	
	/* destructor takes care of memory clean up */
	~int_array_2d(){
		if (already_set){
			free_matrix(ptr, 0, sizex-1, 0, sizeof(int));
		}
	}

	/* can allocate memory for array if that hasn't been done already */
	void alloc_array(int x, int y){
		free_array();
	
		if (!already_set){
			sizex = x;
			sizey = y;
			ptr = (int **) alloc_matrix(0, x-1, 0, y-1, sizeof(int));
			already_set = true;

			//init_to_zero();
		}
	}
	void free_array(){
		if (already_set){
			free_matrix(ptr, 0, sizex-1, 0, sizeof(int));
			ptr = NULL;
			already_set = false;
		}
		sizex = sizey = 0;
	}
};

/* constructor and destructor for a 3d array */
class int_array_3d{
private:	
	bool already_set;
	int sizex;
	int sizey;
	int sizez;

public:
	int ***ptr;

	int_array_3d(){
		sizex = 0;
		sizey = 0;
		sizez = 0;
		already_set = false;
		ptr = NULL;
	}
	int_array_3d(int x, int y, int z){
		sizex = x;
		sizey = y;
		sizez = z;
		ptr = (int ***) alloc_matrix3(0, x-1, 0, y-1, 0, z-1, sizeof(int));
		already_set = true;
	}
	
	/* destructor takes care of memory clean up */
	~int_array_3d(){
		if (already_set){
			assert(NULL != ptr);
			free_matrix3(ptr, 0, sizex-1, 0, sizey-1, 0, sizeof(int));
		}
	}
	/* can allocate memory for array if that hasn't been done already */
	void alloc_array(int x, int y, int z){
		free_array();

		if (!already_set){
			sizex = x;
			sizey = y;
			sizez = z;
			ptr = (int ***) alloc_matrix3(0, x-1, 0, y-1, 0, z-1, sizeof(int));
			already_set = true;

			//init_to_zero();
		}
	}
	void free_array(){
		if (already_set){
			free_matrix3(ptr, 0, sizex-1, 0, sizey-1, 0, sizeof(int));
			ptr = NULL;
			already_set = false;
		}
		sizex = sizey = sizez = 0;
	}
};


/**** Structures ****/
/* Contains statistics on a connection block's diversity */
typedef struct s_conn_block_homogeneity {
	/* actual metrics */
	float pin_homogeneity;
	float pin_diversity;
	float wire_homogeneity;
	float hamming_distance;
	float hamming_proximity;

	/*other important data -- very useful for computing metrics within an optimizer */
	int num_wire_types;
	int_array_2d wh_wire_conns;
	int_array_2d ph_pin_averages;
	int_array_3d hd_pin_array;
	int counted_pins_per_side[4];

	/* initialize to zeros */
	s_conn_block_homogeneity() : 
	wh_wire_conns(), ph_pin_averages(), hd_pin_array()
	{
		pin_homogeneity = 0;
		pin_diversity = 0;
		wire_homogeneity = 0;
		hamming_distance = 0;
		hamming_proximity = 0;

		num_wire_types = 0;
		for (int i = 0; i<4; i++)
			counted_pins_per_side[i] = 0;
	}
} t_conn_block_homogeneity;


int get_num_wire_types(INP int num_segments, INP t_segment_inf *segment_inf);

void get_conn_block_homogeneity(OUTP t_conn_block_homogeneity &cbm, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int *Fc_array, INP int nodes_per_chan, INP int num_segments, 
		INP t_segment_inf *segment_inf);

/* TODO: this function seems to not be playing well with the pointers used by the metric structure
   investigate */
t_conn_block_homogeneity get_conn_block_homogeneity_fpga(INP t_conn_block_homogeneity *block_homogeneity, 
		INP int num_block_types, INP struct s_grid_tile **fpga_grid, INP int size_x, INP int size_y,
		INP t_type_ptr block_types, INP e_pin_type pin_type);

void adjust_pin_metric(INP float PD_target, INP float PD_tolerance, INP float HD_tolerance,
		INP t_type_ptr block_type, 
		INOUTP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int *Fc_array, INP int nodes_per_chan, INP int num_segments, 
		INP t_segment_inf *segment_inf);

void adjust_hamming(INP float target, INP float target_tolerance, INP float pin_tolerance,
		INP t_type_ptr block_type, INOUTP int *****tracks_connected_to_pin, 
		INP e_pin_type pin_type, INP int *Fc_array, INP int nodes_per_chan, 
		INP int num_segments, INP t_segment_inf *segment_inf);

void generate_random_trackmap(INOUTP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP t_type_ptr block_type);

void write_trackmap_to_file(INP char *filename, INP int *****tracks_connected_to_pin, INP e_pin_type pin_type,
		INP t_type_ptr block_type, INP int Fc);

void read_trackmap_from_file(INP char *filename, OUTP int *****tracks_connected_to_pin, INP e_pin_type pin_type,
		INP t_type_ptr block_type, INP int Fc);

int get_max_Fc(INP int *Fc_array, INP t_type_ptr block_type, INP e_pin_type pin_type);

//need to find a way to write track connections to a file
// first check if something like this already exists in our libraries

#endif /*SWITCHBLOCK_METRICS_H*/
