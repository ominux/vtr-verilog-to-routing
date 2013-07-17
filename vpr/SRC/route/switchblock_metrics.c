
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <ctime>
#include "util.h"
#include "vpr_types.h"
#include "vpr_utils.h"
#include "switchblock_metrics.h"


static void get_num_fpga_blocks(OUTP int *num_fpga_blocks, INP int num_block_types, 
		INP struct s_grid_tile **fpga_grid, INP int size_x, INP int size_y);

static float get_pin_homogeneity(INP int **pin_averages, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent);

static float get_wire_homogeneity(INP int **wire_conns, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP int *counted_pins_per_side, 
		INP boolean both_sides);

static float get_hamming_distance(INP int ***pin_array, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP boolean both_sides);

static float get_hamming_proximity(INP int ***pin_array, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP boolean both_sides);

static float get_pin_diversity(INP int **pin_averages, INP t_type_ptr block_type, 
		INP e_pin_type pin_type, INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins);



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
	
	int  i, Fc, iclass, pin, num_pin_type_pins;
	i = Fc = iclass = pin = num_pin_type_pins = 0;

	/* Find max Fc */
	for (pin = 0; pin < block_type->num_pins; ++pin) {
		iclass = block_type->pin_class[pin];
		if (Fc_array[pin] > Fc && block_type->class_inf[iclass].type == pin_type) {
			Fc = Fc_array[pin];
		}
	}
	/* get number of pin_type_pins */
	if (DRIVER == pin_type){
		num_pin_type_pins = block_type->num_drivers;
	} else if (RECEIVER == pin_type){
		num_pin_type_pins = block_type->num_receivers;
	} else {
		assert(FALSE);
	}

	int num_wire_types = get_num_wire_types(num_segments, segment_inf);
	int **wh_wire_conns = (int **) alloc_matrix(0, 3, 0, nodes_per_chan - 1, sizeof(int));
	int **ph_pin_averages = (int **) alloc_matrix(0, block_type->num_pins - 1, 0, num_wire_types - 1, sizeof(int));
	int ***hd_pin_array = (int ***) alloc_matrix3(0, 3, 0, block_type->num_pins - 1, 0, Fc - 1, sizeof(int));

	t_conn_block_homogeneity conn_block_homogeneity;	
	conn_block_homogeneity.pin_homogeneity = 0;
	conn_block_homogeneity.wire_homogeneity = 0;

	int side, height, width, counted_pins_per_side[4];
	int counted_pins = 0;
	int track = 0;
	for (side = 0; side < 4; side++){
		counted_pins_per_side[side] = 0;
		for (i = 0; i < nodes_per_chan; i++){
			wh_wire_conns[side][i] = 0;
		}
		for (pin = 0; pin < block_type->num_pins; pin++){
			for (i = 0; i < Fc; i++){
				hd_pin_array[side][pin][i] = -1;
			}
		}
	}	
	for (pin = 0; pin < block_type->num_pins; pin++){
		for (i = 0; i < num_wire_types; i++){
			ph_pin_averages[pin][i] = 0;
		}
	}


	/* Here we make the arrays necessary for PH, HD, and WH */
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
					hd_pin_array[side][pin][0] = track;					
					wh_wire_conns[side][track]++;
					ph_pin_averages[pin][track % num_wire_types]++;
					
					//printf("side: %d  pin: %d  count: %d  track: %d\n", side, pin, 0, track);
					for(i = 1; i < Fc; i++){
						/* get next track */
						track = tracks_connected_to_pin[pin][width][height][side][i];
						
						hd_pin_array[side][pin][i] = track;
						//printf("side: %d  pin: %d  count: %d  track: %d\n", side, pin, i, track);
						/* find the number of times the pin connects to each wire class */
						ph_pin_averages[pin][track % num_wire_types]++;
						/* find the number of times a connection is made with each wire/side */
						wh_wire_conns[side][track]++; 
					}
					counted_pins++;
					counted_pins_per_side[side]++;
				}
			}
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

		conn_block_homogeneity.pin_homogeneity = get_pin_homogeneity(ph_pin_averages, block_type, tracks_connected_to_pin, 
								pin_type, Fc, nodes_per_chan, num_wire_types,
								num_pin_type_pins, 2);

		conn_block_homogeneity.wire_homogeneity = get_wire_homogeneity(wh_wire_conns, block_type, tracks_connected_to_pin,
								pin_type, Fc, nodes_per_chan, num_wire_types,
								num_pin_type_pins, 2, counted_pins_per_side, both_sides); 
		
		conn_block_homogeneity.hamming_distance = get_hamming_distance(hd_pin_array, block_type, tracks_connected_to_pin,
								pin_type, Fc, nodes_per_chan, num_wire_types,
								num_pin_type_pins, 2, both_sides); 
		
		conn_block_homogeneity.hamming_proximity = get_hamming_proximity(hd_pin_array, block_type, tracks_connected_to_pin,
								pin_type, Fc, nodes_per_chan, num_wire_types,
								num_pin_type_pins, 2, both_sides); 

		conn_block_homogeneity.pin_diversity = get_pin_diversity(ph_pin_averages, block_type,
								pin_type, Fc, nodes_per_chan, num_wire_types,
								num_pin_type_pins); 
	}	

	free_matrix(ph_pin_averages, 0, block_type->num_pins - 1, 0, sizeof(int));
	free_matrix(wh_wire_conns, 0, 3, 0, sizeof(int));
	free_matrix3(hd_pin_array, 0, 3, 0,block_type->num_pins - 1, 0, sizeof(int));
	ph_pin_averages = NULL;
	wh_wire_conns = NULL;
	return conn_block_homogeneity;
}


void adjust_hamming(INP float target, INP float target_tolerance, INP float pin_tolerance,
		INP t_type_ptr block_type, INOUTP int *****tracks_connected_to_pin, 
		INP e_pin_type pin_type, INP int *Fc_array, INP int nodes_per_chan, 
		INP int num_segments, INP t_segment_inf *segment_inf){
//TODO: move this up top so all functions han see what's being optimized.
//TODO: also, rename to OPTIMIZE_HAMMING_PROXIMITY or something.
#define HAMMING_DISTANCE
//#define PRESERVE_TRACKS
#ifdef HAMMING_DISTANCE
	#define USE_ANNEALER
#endif

	boolean success = FALSE;	
	int Fc = 0;
	int num_pins_per_side = 5;
	int pin_map[20] = {	40, 44, 48, 52, 56, 
				41, 45, 49, 53, 57, 
				42, 46, 50, 54, 58,
				43, 47, 51, 55, 59};

	/* get initial values of metrics */
	t_conn_block_homogeneity initialMetrics;
	initialMetrics = get_conn_block_homogeneity(block_type, 
		tracks_connected_to_pin, pin_type, 
		Fc_array, nodes_per_chan, num_segments, 
		segment_inf);

	float init_hamming, init_pin;
#ifdef HAMMING_DISTANCE
	init_hamming = initialMetrics.hamming_distance;
#elif defined HAMMING_PROXIMITY
	init_hamming = initialMetrics.hamming_proximity;
#elif defined WIRE_HOMOGENEITY
	init_hamming = initialMetrics.wire_homogeneity;
#else
	//crash and burn
#endif
	init_pin = initialMetrics.pin_diversity;
	
	if (fabs(init_hamming - target) <= target_tolerance){
		/* already in correct range */
		return;
	}

	/* Find max Fc */
	for (int pin = 0; pin < block_type->num_pins; ++pin) {
		int iclass = block_type->pin_class[pin];
		if (Fc_array[pin] > Fc && block_type->class_inf[iclass].type == pin_type) {
			Fc = Fc_array[pin];
		}
	}

	srand(time(0));

	int num_pin_type_pins = 0;
	/* get number of pin_type_pins */
	if (DRIVER == pin_type){
		num_pin_type_pins = block_type->num_drivers;
	} else if (RECEIVER == pin_type){
		num_pin_type_pins = block_type->num_receivers;
	} else {
		assert(FALSE);
	}

	int counted_pins = 0;
	int **wire_conns = (int **) alloc_matrix(0, 3, 0, nodes_per_chan - 1, sizeof(int));
	for (int side = 0; side < 4; side++){
		for (int i = 0; i < nodes_per_chan; i++){
			wire_conns[side][i] = 0;
		}
	}
	/* Here we make the arrays necessary for PH, HD, and WH */
	for (int side = 0; side < 4; side++){
		/* over each height unit */
		for (int height = 0; height < block_type->height; height++){
			/*  over each width unit */
			for (int width = 0; width < block_type->width; width++){
				/* over each pin */
				for (int pin = 0; pin < block_type->num_pins; pin++){
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
					int track = tracks_connected_to_pin[pin][width][height][side][0];
					if (OPEN == track){
						continue;
					}
					wire_conns[side % 2][track]++;
					
					//printf("side: %d  pin: %d  count: %d  track: %d\n", side, pin, 0, track);
					for(int i = 1; i < Fc; i++){
						/* get next track */
						track = tracks_connected_to_pin[pin][width][height][side][i];
						
						/* find the number of times a connection is made with each wire/side */
						wire_conns[side % 2][track]++; 
					}
					counted_pins++;
				}
			}
		} 
	}


	float old_diff, new_diff;
	old_diff = fabs(init_hamming - target);
	/* start iterative process */
#ifdef USE_ANNEALER
	int no_luck = 0;
#endif
	int maxTries = 100000;
	for (int tries = 0; tries < maxTries; tries++){
		t_conn_block_homogeneity newMetrics;
		int rand_side, rand_pin, rand_con;
		int *track_ptr, old_track, new_track;
 		boolean invalid_new_track = TRUE;
		boolean invalid_old_track = TRUE;
		
		/* find a random side, random pin, random switch */
		rand_side = rand() % 4;
		rand_pin = rand() % num_pins_per_side;
		rand_pin += (num_pins_per_side * rand_side);
		rand_pin = pin_map[rand_pin];		
		
		track_ptr = &tracks_connected_to_pin[rand_pin][0][0][rand_side][0];
		
		/* get an old track connection */
		do {
			invalid_old_track = FALSE;
			rand_con = rand() % Fc;
			old_track = track_ptr[rand_con];
			//printf("conns: %d\n", wire_conns[rand_side][old_track]);
#ifdef PRESERVE_TRACKS
			if (wire_conns[rand_side % 2][old_track] <= 1){
				invalid_old_track = TRUE;
			}
#else
			invalid_old_track = FALSE;
#endif		
		} while (invalid_old_track);

		/* get a new track connection */
		do {
			new_track = rand() % nodes_per_chan;
			invalid_new_track = FALSE;
			/* check if new_track is equal to an existing track */
			for( int i = 0; i < Fc; i++ ){
				if( new_track == track_ptr[i] ){
					invalid_new_track = TRUE;
					break;
				}
			}
		} while (invalid_new_track);

		/* assign new track, and test metrics */
		track_ptr[rand_con] = new_track;
		newMetrics = get_conn_block_homogeneity(block_type, tracks_connected_to_pin, 
					pin_type, Fc_array, nodes_per_chan, num_segments, 
					segment_inf);
		
		float new_hamming, new_pin;
		new_pin = newMetrics.pin_diversity;
#ifdef HAMMING_DISTANCE
		new_hamming = newMetrics.hamming_distance;
#elif defined HAMMING_PROXIMITY
		new_hamming = newMetrics.hamming_proximity;
#elif defined WIRE_HOMOGENEITY
		new_hamming = newMetrics.wire_homogeneity;
#endif 
		new_diff = fabs(new_hamming - target);
		/* check if we're closer to hitting our target */
		float test_condition = 0;
#ifdef USE_ANNEALER
		float temp = (1 - (float)tries / (float)maxTries);
		test_condition = temp;
#else
		test_condition = old_diff;
#endif
		if(new_diff < test_condition &&
		    fabs(new_pin - init_pin) <= pin_tolerance){
			printf("new_hamming %f   test_condition %f   new_diff %f   new_pin %f\n", new_hamming, test_condition, new_diff, new_pin);
			/* check if we satisfy all constraints */
			if ( new_diff <= target_tolerance ){
				/* we're done */
				success = TRUE;
				break;
			} else {
				/* keep going */
				old_diff = new_diff;
				wire_conns[rand_side % 2][old_track]--;
				wire_conns[rand_side % 2][new_track]++;
#ifdef USE_ANNEALER				
				if ( fabs(test_condition - new_diff) >= 0.05 ){
					tries = (int)((1-new_diff)*(float)maxTries*0.99);
				}
				no_luck = 0;
#endif
			}
		} else {
			/* revert changes */
#ifdef USE_ANNEALER
			no_luck++;
			if (no_luck==10000){
				printf("test_condition %f   new_diff %f\n", test_condition, new_diff);
				break;
			}
			if (tries%3 == 0){
				tries--;
			}
#endif
			track_ptr[rand_con] = old_track;
		}
		 
		track_ptr = NULL;
	}
	if (FALSE == success){
		printf("failed to adjust hamming!\n");
	}
	//int total = 0;
	//for (int side = 0; side < 4; side++){
	//	for (int track = 0; track < nodes_per_chan; track++){
	//		printf("Fc %d   side: %d   track %d   conns %d\n", Fc, side, track, wire_conns[side % 2][track]);
	//		total += wire_conns[side % 2][track];
	//	}
	//}
	//printf("total: %d\n", total);

	free_matrix(wire_conns, 0, 3, 0, sizeof(int));
	return;
}

void adjust_pin_metric(INP float pin_target, INP float pin_tolerance, INP float hamming_tolerance,
		INP t_type_ptr block_type, 
		INOUTP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int *Fc_array, INP int nodes_per_chan, INP int num_segments, 
		INP t_segment_inf *segment_inf){

#define PIN_DIVERSITY

	boolean success = FALSE;
	int Fc = 0;
	float old_diff, new_diff;
	float init_hamming, hamming_metric, pin_metric;
	int num_pins_per_side = 5;
	int pin_map[20] = {	40, 44, 48, 52, 56, 
				41, 45, 49, 53, 57, 
				42, 46, 50, 54, 58,
				43, 47, 51, 55, 59};

	/* get initial values of metrics */
	t_conn_block_homogeneity initialMetrics;
	initialMetrics = get_conn_block_homogeneity(block_type, 
		tracks_connected_to_pin, pin_type, 
		Fc_array, nodes_per_chan, num_segments, 
		segment_inf);
#ifdef PIN_DIVERSITY
	pin_metric = initialMetrics.pin_diversity;
#elif defined PIN_HOMOGENEITY
	pin_metric = initialMetrics.pin_homogeneity;
#endif
	init_hamming = initialMetrics.hamming_distance;
	old_diff = fabs(pin_metric - pin_target);

	if (old_diff <= pin_tolerance){
		/* already meet target */
		return;
	}

	/* Find max Fc */
	for (int pin = 0; pin < block_type->num_pins; ++pin) {
		int iclass = block_type->pin_class[pin];
		if (Fc_array[pin] > Fc && block_type->class_inf[iclass].type == pin_type) {
			Fc = Fc_array[pin];
		}
	}

	srand(time(0));
	
	for (int tries = 0; tries < 100000; tries++){
		t_conn_block_homogeneity newMetrics;
		int rand_side, rand_pin, rand_con;
		int *track_ptr, old_track, new_track;
 		boolean invalid_conn = TRUE;
		
		/* find a random side, random pin, random switch */
		rand_side = rand() % 4;
		rand_pin = rand() % num_pins_per_side;
		rand_pin += (num_pins_per_side * rand_side);
		rand_pin = pin_map[rand_pin];		
		rand_con = rand() % Fc;	
		
		/* now we get a new track connection */
		track_ptr = &tracks_connected_to_pin[rand_pin][0][0][rand_side][0];
		old_track = track_ptr[rand_con];
		do {
			new_track = rand() % nodes_per_chan;
			invalid_conn = FALSE;
			/* check if new_track is equal to an existing track */
			for( int i = 0; i < Fc; i++ ){
				if( new_track == track_ptr[i] ){
					invalid_conn = TRUE;
					break;
				}
			}
		} while (invalid_conn);

		/* assign new track, and test metrics */
		track_ptr[rand_con] = new_track;
		newMetrics = get_conn_block_homogeneity(block_type, tracks_connected_to_pin, 
					pin_type, Fc_array, nodes_per_chan, num_segments, 
					segment_inf);
		/* test if new metrics meet demand */
		hamming_metric = newMetrics.hamming_distance;
#ifdef PIN_DIVERSITY
		pin_metric = newMetrics.pin_diversity;
#elif defined PIN_HOMOGENEITY
		pin_metric = newMetrics.pin_homogeneity;
#endif
		new_diff = fabs(pin_metric - pin_target);

		/* check if we are closer to target */
		if( new_diff <= old_diff &&
		    fabs(hamming_metric - init_hamming) <= hamming_tolerance){
			printf("Pin Metric: %f   new_diff: %f   old_diff: %f\n", pin_metric, new_diff, old_diff);
			/* check if we satisfy desired pin constraint */
			if( new_diff <= pin_tolerance){
				/* we are done */
				success = TRUE;
				break;
			} else {
				/* we keep going */
				old_diff = new_diff;
			}
		} else {
			/* revert changes */
			track_ptr[rand_con] = old_track;
		}
		track_ptr = NULL;
	}
	if (FALSE == success)
		printf("failed to adjust metric!\n");

	/* this commented code swaps connections instead of moving a single connection to a new location */
	//for (int tries = 0; tries < 100000; tries++){
	//	t_conn_block_homogeneity newMetrics;
	//	boolean valid_conns;
	//	int *track1, *track2;
	//	int rand_pin1, rand_pin2, rand_con1, rand_con2, rand_side;	
	//
	//	do{
	//		/* random side */
	//		rand_side = rand() % 4;
	//		/* random pins */
	//		rand_pin1 = rand() % num_pins_per_side;
	//		rand_pin2 = 0;
	//		do{
	//			/* pin2 must be different from pin 1 */
	//			rand_pin2 = rand() % num_pins_per_side;
	//		} while (rand_pin1 == rand_pin2);
	//		/* make pins 0..20 */
	//		rand_pin1 += (num_pins_per_side * rand_side);
	//		rand_pin2 += (num_pins_per_side * rand_side);
	//		rand_pin1 = pin_map[rand_pin1];
	//		rand_pin2 = pin_map[rand_pin2];
	//		/* get random connections*/
	//		rand_con1 = rand() % Fc;
	//		rand_con2 = rand() % Fc;

	//		/* swap these two conns and test */
	//		track1 = &tracks_connected_to_pin[rand_pin1][0][0][rand_side][rand_con1];
	//		track2 = &tracks_connected_to_pin[rand_pin2][0][0][rand_side][rand_con2];
	//		/* need to check that one pin does not already make the same connection as the other */
	//		valid_conns = TRUE;
	//		for (int i = 0; i < Fc; i++){
	//			int *temp_track1 = &tracks_connected_to_pin[rand_pin1][0][0][rand_side][i];
	//			int *temp_track2 = &tracks_connected_to_pin[rand_pin2][0][0][rand_side][i];
	//			//printf("i %d   tmpt1 %d   tmpt2 %d\n", i, *temp_track1, *temp_track2);
	//			if (*track2 == *temp_track1 ||
	//			    *track1 == *temp_track2){
	//				valid_conns = FALSE;
	//				break;
	//			}
	//		}
	//	} while (FALSE == valid_conns);

	//	/* can check if pin metric won't change for sure */
	//	if (*track1 % 4 == *track2 % 4){
	//		continue;
	//	}

	//	int temp_track = *track1;
	//	*track1 = *track2;
	//	*track2 = temp_track;
	//	newMetrics = get_conn_block_homogeneity(block_type, tracks_connected_to_pin, 
	//				pin_type, Fc_array, nodes_per_chan, num_segments, 
	//				segment_inf);


	//	/* test if new metrics meet demand */
	//	hamming_metric = newMetrics.hamming_distance;
//#ifdef P//IN_DIVERSITY
	//	pin_metric = newMetrics.pin_diversity;
//#elif de//fined PIN_HOMOGENEITY
	//	pin_metric = newMetrics.pin_homogeneity;
//#endif
	//	new_diff = fabs(pin_metric - pin_target);

	//	/* check if we are closer to target */
	//	if( new_diff <= old_diff &&
	//	    fabs(hamming_metric - init_hamming) <= hamming_tolerance){
	//		printf("Pin Metric: %f   new_diff: %f   old_diff: %f\n", pin_metric, new_diff, old_diff);
	//		/* check if we satisfy desired pin constraint */
	//		if( new_diff <= pin_tolerance){
	//			/* we are done */
	//			success = TRUE;
	//			break;
	//		} else {
	//			/* we keep going */
	//			old_diff = new_diff;
	//		}
	//	} else {
	//		/* undo the connection swap */
	//		temp_track = *track1;
	//		*track1 = *track2;
	//		*track2 = temp_track;
	//	}

	//	track1 = NULL;
	//	track2 = NULL;
	//}
	//if (FALSE == success){
	//	printf("Failed to adjust PD\n");
	//}

	return;
}

static float get_pin_diversity(INP int **pin_averages, INP t_type_ptr block_type, 
		INP e_pin_type pin_type, INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins){

	float total_pin_diversity = 0;
	float exp_factor = 3.3;
	float *pin_diversity = (float *) my_malloc(sizeof(float) * block_type->num_pins);

	/* Determine the diversity of each pin. The concept of this function is that	*
	*  a pin connecting to a wire class more than once returns diminishing gains.	*
	*  This is modelled as an exponential function s.t. at a given ratio of 	*
	*  connections/expected_connections we will always get the same contribution to	*
	*  pin diversity.								*/
	float mean = (float)Fc / (float)(num_wire_types);
	for (int pin = 0; pin < block_type->num_pins; pin++){
		/* only doing pin_type pins */
		if (block_type->class_inf[block_type->pin_class[pin]].type != pin_type){
			continue;
		}
		
		pin_diversity[pin] = 0;
		for (int i = 0; i < num_wire_types; i++){
			pin_diversity[pin] += (1 / (float)num_wire_types) * 
					      (1 - exp(-exp_factor * (float)pin_averages[pin][i] / mean));
		}
		total_pin_diversity += pin_diversity[pin];
	}
	total_pin_diversity /= num_pin_type_pins;

	free(pin_diversity);
	pin_diversity = NULL;

	return total_pin_diversity;
}



/* Returns the hamming average hamming distance of the block's pins */
static float get_hamming_proximity(INP int ***pin_array, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP boolean both_sides){
	
	float total_hamming_proximity = 0;

	/* get the overall hamming distance for this block */
	int side, pin, comp_pin, itrack, jtrack, track1, track2;
	int checked_pins, pin_class;
	float hd_pins = 0;
	int new_pin, new_comp_pin, new_side, new_comp_side;
	int mult = (both_sides) ? 2 : 1;	/* may want to calculate HD for two sides at once to	*
						*  simulate presence of neighboring blocks		*/
	int num_pins = block_type->num_pins;

	for (side = 0; side < (4 / mult); side++){
		checked_pins = 0;
		hd_pins = 0;
		for (pin = 0; pin < (mult * num_pins); pin++){
			if (pin >= num_pins){
				new_pin = pin % num_pins;
				new_side = side + 2;
			} else {
				new_pin = pin;
				new_side = side;
			}

			/* only doing pin_type pins */
			pin_class = block_type->pin_class[new_pin];
			if (block_type->class_inf[pin_class].type != pin_type){
				continue;
			} else if (-1 == pin_array[new_side][new_pin][0]){
				continue;
			}
			//printf("side %d  pin %d  track %d\n", new_side, new_pin, pin_array[new_side][new_pin][0]);
			int pin_hd = 0;
			int pin_comparisons = 0;
			checked_pins++;
			for (comp_pin = pin + 1; comp_pin < (mult * num_pins); comp_pin++){
				if (comp_pin >= num_pins){
					new_comp_pin = comp_pin % num_pins;
					new_comp_side = side + 2;
				} else {
					new_comp_pin = comp_pin;
					new_comp_side = side;
				}
				int pin_to_pin_hd = 0;

				pin_class = block_type->pin_class[new_comp_pin];
				if (block_type->class_inf[pin_class].type != pin_type){
					continue;
				} else if (-1 == pin_array[new_comp_side][new_comp_pin][0]){
					continue;
				}

				pin_comparisons++;
				//printf("Fc %d\n", Fc);
				for (itrack = 0; itrack < Fc; itrack++){
				//	printf("iterating track\n");
					track1 = pin_array[new_side][new_pin][itrack];
					track2 = pin_array[new_comp_side][new_comp_pin][itrack];
					for (jtrack = 0; jtrack < Fc; jtrack++){
						track2 = pin_array[new_comp_side][new_comp_pin][jtrack];
						//printf("track1 %d   track2 %d\n", track1, track2);
						if (track1 == track2){
							pin_to_pin_hd++;	/* not hamming distance yet */
							//printf("pin_hd: %d\n", pin_hd);
						}
					}
				}
				/* If two bit vectors of some length have Fc 1's, and x 1's share the	*
				*  same bit position, then the hamming distance between them is 2(Fc-x)	*/
				pin_to_pin_hd = pow( pin_to_pin_hd, exponent);
				pin_hd += pin_to_pin_hd;
			}
			/* add to running total */
			hd_pins += (float)pin_hd;
			//printf("hd_pins: %f\n", hd_pins);	
		}
		total_hamming_proximity += hd_pins * 2.0 / (float)((checked_pins - 1) * pow(Fc,exponent));
		
		//printf("Fc: %d   side: %d  hd: %f  checked_pins: %d\n", Fc, side, hd_pins * 2.0 / (float)((checked_pins - 1) * pow(2*Fc,exponent)), checked_pins);
	}
	/* average over the sides */
	total_hamming_proximity /= num_pin_type_pins;


	return total_hamming_proximity;
}


/* Returns the hamming average hamming distance of the block's pins */
static float get_hamming_distance(INP int ***pin_array, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP boolean both_sides){
	
	float total_hamming_distance = 0;

	/* get the overall hamming distance for this block */
	int side, pin, comp_pin, itrack, jtrack, track1, track2;
	int checked_pins, pin_class;
	float hd_pins = 0;
	int new_pin, new_comp_pin, new_side, new_comp_side;
	int mult = (both_sides) ? 2 : 1;	/* may want to calculate HD for two sides at once to	*
						*  simulate presence of neighboring blocks		*/
	int num_pins = block_type->num_pins;

	for (side = 0; side < (4 / mult); side++){
		checked_pins = 0;
		hd_pins = 0;
		for (pin = 0; pin < (mult * num_pins); pin++){
			if (pin >= num_pins){
				new_pin = pin % num_pins;
				new_side = side + 2;
			} else {
				new_pin = pin;
				new_side = side;
			}

			/* only doing pin_type pins */
			pin_class = block_type->pin_class[new_pin];
			if (block_type->class_inf[pin_class].type != pin_type){
				continue;
			} else if (-1 == pin_array[new_side][new_pin][0]){
				continue;
			}
			//printf("side %d  pin %d  track %d\n", new_side, new_pin, pin_array[new_side][new_pin][0]);
			float pin_hd = 0;
			int pin_comparisons = 0;
			checked_pins++;
			for (comp_pin = pin + 1; comp_pin < (mult * num_pins); comp_pin++){
				if (comp_pin >= num_pins){
					new_comp_pin = comp_pin % num_pins;
					new_comp_side = side + 2;
				} else {
					new_comp_pin = comp_pin;
					new_comp_side = side;
				}
				float pin_to_pin_hd = 0;

				pin_class = block_type->pin_class[new_comp_pin];
				if (block_type->class_inf[pin_class].type != pin_type){
					continue;
				} else if (-1 == pin_array[new_comp_side][new_comp_pin][0]){
					continue;
				}

				pin_comparisons++;
				//printf("Fc %d\n", Fc);
				for (itrack = 0; itrack < Fc; itrack++){
				//	printf("iterating track\n");
					track1 = pin_array[new_side][new_pin][itrack];
					track2 = pin_array[new_comp_side][new_comp_pin][itrack];
					for (jtrack = 0; jtrack < Fc; jtrack++){
						track2 = pin_array[new_comp_side][new_comp_pin][jtrack];
						//printf("track1 %d   track2 %d\n", track1, track2);
						if (track1 == track2){
							pin_to_pin_hd++;	/* not hamming distance yet */
						}
					}
				}
				/* If two bit vectors of some length have Fc 1's, and x 1's share the	*
				*  same bit position, then the hamming distance between them is 2(Fc-x)	*/
				//pin_to_pin_hd = pow( (2 * (Fc - pin_to_pin_hd)), exponent);
				pin_to_pin_hd = 2 * ((float)Fc - pin_to_pin_hd);	//convert hamming proximity to hamming distance
				if (pin_to_pin_hd==0)
					pin_to_pin_hd = 1;
				pin_to_pin_hd = pow( 1 / pin_to_pin_hd, exponent);	//implement Lemieux's cost function
				pin_hd += pin_to_pin_hd;				//add to running total for current pin
				//printf("pin_to_pin_hd: %f\n", pin_to_pin_hd);
			}
			/* add to running total */
			hd_pins += (float)pin_hd;
			//printf("pin_hd: %f\n", pin_hd);	
		}
		total_hamming_distance += hd_pins / (0.5*checked_pins*(checked_pins - 1)); // norm factor for old HD: * 2.0 / (float)((checked_pins - 1) * pow(2*Fc,exponent));
		//printf("checked_pins: %d  hd_pins: %f  total_lemieux: %f\n", checked_pins, hd_pins, total_hamming_distance);	
	}
	/* average over the sides */
	total_hamming_distance /= (4 / (both_sides ? 2 : 1));//(0.5*num_pin_type_pins*(num_pin_type_pins - 1));//num_pin_type_pins;


	return total_hamming_distance;
}


/* Returns the wire homogeneity of a block's connection to tracks */
static float get_wire_homogeneity(INP int **wire_conns, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent, INP int *counted_pins_per_side, 
		INP boolean both_sides){
	
	float total_wire_homogeneity = 0;
	float *wire_homogeneity = (float *) my_malloc(sizeof(float) * 4);


	/* determine the homogeneity of each wire.*/ 
	int i = 0;
	int unconnected_wires = 0;
	int total_conns = 0;
	int total_pins_on_side = 0;
	float mean = 0;
	float wire_homogeneity_temp = 0;
	int mult = (both_sides) ? 2 : 1;
	//if both sides, we only do two sides.
	/* sides must be ordered as TOP, RIGHT, BOTTOM, LEFT. see the e_side enum */
	for (int side = 0; side < (4 / mult); side++){
		mean = 0;
		unconnected_wires = 0;
		total_conns = total_pins_on_side = 0;
		for (i = 0; i < mult; i++){
			//printf("pins_per_side: %d\n", counted_pins_per_side[side+mult*i]);
			total_pins_on_side += counted_pins_per_side[side + mult*i];
		}
		total_conns = total_pins_on_side * Fc;
		unconnected_wires = (total_conns) ? std::max(0, nodes_per_chan - total_conns)  :  0 ;
		mean = (float)total_conns / (float)(nodes_per_chan - unconnected_wires);
		//printf("total_conns %d  mean: %f   unconnected: %d\n", total_conns, mean, unconnected_wires);
		wire_homogeneity[side] = 0;
		for (int track = 0; track < nodes_per_chan; track++){
			wire_homogeneity_temp = 0;
			for (i = 0; i < mult; i++){
				if (counted_pins_per_side[side + i*mult] > 0){
					/* only include sides with connected pins */
					wire_homogeneity_temp += (float)wire_conns[side + i*mult][track];
				}
			}
			wire_homogeneity[side] += pow(fabs(wire_homogeneity_temp - mean), exponent);
		//	printf("side: %d   conns: %d  WH: %f\n", side, wire_conns[side][track], wire_homogeneity[side]);
		}
		float normalization = ((float)Fc*pow(((float)total_pins_on_side - mean),exponent) + (float)(nodes_per_chan - Fc)*pow(mean,exponent)) / (float)total_pins_on_side;
		//printf("total_pins %d  normalization %f\n",total_pins_on_side, normalization);
		wire_homogeneity[side] -= unconnected_wires * mean;
		wire_homogeneity[side] /= normalization;//(float)(2*Fc);
		total_wire_homogeneity += wire_homogeneity[side];
	}
	total_wire_homogeneity /= num_pin_type_pins;

	free(wire_homogeneity);
	wire_homogeneity = NULL;

	return total_wire_homogeneity;
}


/* returns the pin homogeneity of a block's connection to tracks */
static float get_pin_homogeneity(INP int **pin_averages, INP t_type_ptr block_type, 
		INP int *****tracks_connected_to_pin, INP e_pin_type pin_type, 
		INP int Fc, INP int nodes_per_chan, INP int num_wire_types,
		INP int num_pin_type_pins, INP int exponent){

	float total_pin_homogeneity = 0;
	float *pin_homogeneity = (float *) my_malloc(sizeof(float) * block_type->num_pins);

	//printf("counted: %d  drivers: %d  receivers: %d  num_pins: %d  num_class: %d\n", counted_pins, block_type->num_drivers, block_type->num_receivers, block_type->num_pins, block_type->num_class);
	/* determine the homogeneity of each pin. */
	//int unconnected_types = Fc % num_wire_types;
	float mean = (float)Fc / (float)(num_wire_types);
	for (int pin = 0; pin < block_type->num_pins; pin++){
		/* only doing pin_type pins */
		if (block_type->class_inf[block_type->pin_class[pin]].type != pin_type){
			continue;
		}
		
		pin_homogeneity[pin] = 0;
		for (int i = 0; i < num_wire_types; i++){
			pin_homogeneity[pin] += pow(fabs((float)pin_averages[pin][i] - mean), exponent);
		}
		//TODO pin_homogeneity[pin -= unconnected_types
		pin_homogeneity[pin] /= 2 * pow( ((float)Fc * (1 - 1 / (float)num_wire_types)), exponent);
		total_pin_homogeneity += pin_homogeneity[pin];
	}
	/* normalize block PH to [0,1] range */
	total_pin_homogeneity /= num_pin_type_pins;
	
	free(pin_homogeneity);
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






