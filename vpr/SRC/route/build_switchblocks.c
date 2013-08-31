/*
DESCRIPTION
The functions in this file are used to help build the switchblocks in rr_graph.c:build_rr_graph
(partially) based on the information in physical_types.h:t_switchblock_inf parsed from the switchblock section
of the XML architecture file.
*/


#include <cstring>
#include <cassert>
#include "build_switchblocks.h"
#include "physical_types.h"
#include "parse_switchblocks.h"

using namespace std;


/**** Function Declarations ****/
//TODO: comment
static void get_permutation_conns(INP int x_coord, INP int y_coord, INP enum e_side from_side,
			INP enum e_side to_side, INP int from_track, INP t_chan_details * chan_details_x,
			INP t_chan_details * chan_details_y, INP t_switchblock_inf *sb,
			INP int nx, INP int ny, INOUTP t_sb_connection_map *sb_conns);

static void index_into_correct_chan(INP int tile_x, INP int tile_y, INP enum e_side side, 
			INP t_chan_details *chan_details_x, INP t_chan_details *chan_details_y,
			OUTP int *chan_x, OUTP int *chan_y, 
			OUTP t_chan_details **chan_details);

/* checks whether the specified coordinates are out of bounds */
static bool coords_out_of_bounds(INP int nx, INP int ny, INP int x_coord, INP int y_coord);

/* checks whether the entry specified by coord exists in the switchblock map sb_conns */
static bool sb_connection_exists( INP Switchblock_Lookup coord, INP t_sb_connection_map *sb_conns);



/**** Function Definitions ****/
t_sb_connection_map * alloc_and_load_switchblock_permutations( INP t_chan_details * chan_details_x, 
				INP t_chan_details * chan_details_y, INP int nx, INP int ny, 
				INP vector<t_switchblock_inf> switchblocks, 
				INP int nodes_per_chan, INP enum e_directionality directionality){

	Switchblock_Lookup coord;

	/* allocate the switchblock permutations pointer */
	t_sb_connection_map *sb_conns = new t_sb_connection_map;

	/* iterate over all the switchblocks specified in the architecture */
	for (int i_sb = 0; i_sb < (int)switchblocks.size(); i_sb++){
		t_switchblock_inf sb = switchblocks[i_sb];

		/* verify that switchblock type matches specifies directionality -- currently we have to stay consistent */
		if (directionality != sb.directionality){
			vpr_printf(TIO_MESSAGE_ERROR, "alloc_and_load_switchblock_connections: Switchblock %s does not match directionality of architecture\n", sb.name.c_str());
			exit(1);
		}

		/* Iterate over the x,y coordinates spanning the FPGA. Currently, the FPGA size is set as
		   (nx+1) by (ny+1), so we iterate over 0..nx and 0..ny. */
		for (int x_coord = 0; x_coord <= nx; x_coord++){
			for (int y_coord = 0; y_coord <= ny; y_coord++){
				/* Iterate over each track in channel */
				for (int itrack = 0; itrack < nodes_per_chan; itrack++){

					/* now we iterate over all the potential side1->side2 connections */
					for ( e_side from_side = (e_side) 0; from_side < 4; from_side = (e_side)(from_side + 1) ){
						for ( e_side to_side = (e_side) 0; to_side < 4; to_side = (e_side)(to_side + 1) ){
							
							/* Fill appropriate entry of the sb_conns map with vector specifying the tracks 
							   the current track will connect to */
							get_permutation_conns(x_coord, y_coord, from_side, to_side, itrack,
									chan_details_x, chan_details_y, &sb, nx, ny,
									sb_conns);
							
						}
					}
				}
			}
		}
	}

	return sb_conns;
}

/* calls delete on the switchblock permutations pointer and sets it to 0 */
void free_switchblock_permutations(INOUTP t_sb_connection_map *sb_conns){
	delete sb_conns;
	sb_conns = NULL;
	return;
}

//comment
static void get_permutation_conns(INP int x_coord, INP int y_coord, INP enum e_side from_side,
			INP enum e_side to_side, INP int from_track, INP t_chan_details * chan_details_x,
			INP t_chan_details * chan_details_y, INP t_switchblock_inf *sb,
			INP int nx, INP int ny, INOUTP t_sb_connection_map *sb_conns){
	
	t_chan_details *from_chan_details = NULL;	/* details for source channel */
	t_chan_details *to_chan_details = NULL ;	/* details for destination channel */
	int from_x, from_y;				/* index into source channel */
	int to_x, to_y;					/* index into destination channel */

	Connect_SB_Sides side_conn(from_side, to_side);		/* for indexing into this switchblock's permutation funcs */
	Switchblock_Lookup coord(x_coord, y_coord, from_side, to_side, from_track);	/* for indexing into FPGA's switchblock map */

	/* can't connect a switchblock side to itself */
	if (from_side == to_side){
		return;
	}
	/* check that the permutation map has an entry for this side combination */
	t_permutation_map::const_iterator it = (*sb).permutation_map.find(side_conn);
	if ((*sb).permutation_map.end() == it){
		/* the specified switchblock does not have any permutation funcs for this side1->side2 connection */
		return;
	}

	/* find the correct channel, and the coordinates to index into it for both the source and
	   destination channels */
	index_into_correct_chan(x_coord, y_coord, from_side, chan_details_x, chan_details_y, 
				&from_x, &from_y, &from_chan_details);
	index_into_correct_chan(x_coord, y_coord, to_side, chan_details_x, chan_details_y, 
				&to_x, &to_y, &to_chan_details);

	/* make sure from_x/y and to_x/y aren't out of bounds */
	if (coords_out_of_bounds(nx, ny, to_x, to_y) ||
	    coords_out_of_bounds(nx, ny, from_x, from_y)){
		return;
	}

	
	/* iterate over all the wire connections specified by this switchblock */
	t_wireconn_inf *wireconn_ptr = NULL;
	const char *track_name = from_chan_details[from_x][from_y][from_track].type_name_ptr;
	for (int iconn = 0; iconn < (int)(*sb).wireconns.size(); iconn++){
		wireconn_ptr = &(*sb).wireconns[iconn];
		
		/* check that the current track is a source of this wire connection */
		if ( strcmp(track_name, wireconn_ptr->from_type.c_str()) != 0 ){
			continue;
		}

	}
	track_name = NULL;
	wireconn_ptr = NULL;
	
	//iterate over all wireconns
		//confirm wires with specified names exist
		//find specified source and target tracks
		//connect them according to the specific permutation function
	
	from_chan_details = NULL;
	to_chan_details = NULL;
	return;
}


/* Here we find the correct channel (x or y), and the coordinates to index into it based on the 
   specified tile coordinates and the switchblock side */
static void index_into_correct_chan(INP int tile_x, INP int tile_y, INP enum e_side side, 
			INP t_chan_details *chan_details_x, INP t_chan_details *chan_details_y,
			OUTP int *set_x, OUTP int *set_y, 
			OUTP t_chan_details **set_chan_details){

	/* here we use the VPR convention that a tile 'owns' the channels directly to the right
	   and above it */
	switch (side){
		case TOP:
			*set_chan_details = chan_details_y;
			*set_x = tile_x;
			*set_y = tile_y;
			break;
		case RIGHT:
			*set_chan_details = chan_details_x;
			*set_x = tile_x;
			*set_y = tile_y;
			break;
		case BOTTOM:
			*set_chan_details = chan_details_y;
			*set_x = tile_x;
			*set_y = tile_y-1;
			break;
		case LEFT:
			*set_chan_details = chan_details_x;
			*set_x = tile_x-1;
			*set_y = tile_y;
			break;
		default:
			vpr_printf(TIO_MESSAGE_ERROR, "index_into_correct_chan: unknown side specified: %d\n", side);
			exit(1);
			break;
	}

	return;
}

/* checks whether the specified coordinates are out of bounds */
static bool coords_out_of_bounds(INP int nx, INP int ny, INP int x_coord, INP int y_coord){
	bool result;

	if (x_coord < 0 || x_coord > nx+1 ||
	    y_coord < 0 || y_coord > ny+1){
		result = true;
	} else {
		result = false;
	}
	return result;
}

/* checks whether the entry specified by coord exists in the switchblock map sb_conns */
static bool sb_connection_exists( INP Switchblock_Lookup coord, INP t_sb_connection_map *sb_conns){
	bool result;

	t_sb_connection_map::const_iterator it = (*sb_conns).find(coord);
	if ((*sb_conns).end() != it){
		result = true;
	} else {
		result = false;
	}

	return result;
}
