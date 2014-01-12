/*
DESCRIPTION
The functions in this file are used to help build the switchblocks in rr_graph.c:build_rr_graph
(partially) based on the information in physical_types.h:t_switchblock_inf parsed from the switchblock section
of the XML architecture file.
*/

/* TODO TODO TODO!! in t_seg_details he referes to 'group' as a collection of L tracks that are consecutive and all 
			all with different start points. I'm referring to 'group' as the collection of tracks 
			in a channel going in the same direction and having the same start point (going by literature,
			this seems to be the more appropriate definition of group) */

#include <cstring>
#include <cassert>
#include <algorithm>
#include <iterator>
#include "build_switchblocks.h"
#include "physical_types.h"
#include "parse_switchblocks.h"

using namespace std;

/**** Typedefs ****/
/* Used for translating the name of a track type to the number of tracks belonging to this type */
typedef std::map< std::string, int > t_track_type_sizes;


/**** Function Declarations ****/
/* Counts the number of tracks in each track type in the specified channel */
static void count_track_type_sizes(INP t_seg_details *channel, INP int nodes_per_chan, 
			INOUTP t_track_type_sizes *track_type_sizes);

/* Compute the track that the track at (x, y, from_side, to_side, from_track) should connect to.
   sb_conns is updated with the result */
static void compute_track_connections(INP int x_coord, INP int y_coord, INP enum e_side from_side,
			INP enum e_side to_side, INP int from_track, INP t_chan_details * chan_details_x,
			INP t_chan_details * chan_details_y, INP t_switchblock_inf *sb,
			INP int nx, INP int ny, INP int nodes_per_chan, INP t_track_type_sizes &track_type_sizes,
			INP e_directionality directionality, INOUTP t_sb_connection_map *sb_conns);

static t_rr_type index_into_correct_chan(INP int tile_x, INP int tile_y, INP enum e_side side, 
			INP t_chan_details *chan_details_x, INP t_chan_details *chan_details_y,
			OUTP int *chan_x, OUTP int *chan_y, 
			OUTP t_chan_details **chan_details);

/* checks whether the specified coordinates are out of bounds */
static bool coords_out_of_bounds(INP int nx, INP int ny, INP int x_coord, INP int y_coord, 
			e_rr_type chan_type);

/* returns seg coordinate based on a channel's x/y coordinates */
static int get_seg_coordinate(INP t_rr_type chan_type, INP int x_coord, INP int y_coord);

/* returns the direction in which a destination track at the specified
   switchblock side should be headed */
static e_direction get_dest_direction(e_side to_side, e_directionality directionality);

/* returns the direction in which a source track at the specified
   switchblock side should be headed. */
static e_direction get_src_direction(e_side from_side, e_directionality directionality);

/* returns the group number of the specified track */
static int get_track_group(INP int nx, INP int ny, INP e_rr_type chan_type,
			 INP t_seg_details &track_details, int seg_coord);

/* Finds the index of the first track in the channel specified by 'track_details'
   to have the type name specified by 'type' */
static int find_start_of_track_type(INP t_seg_details *track_details, INP const char *type, 
			INP int nodes_per_chan, INP t_track_type_sizes track_type_sizes);

/* returns the track index in the channel specified by 'track_details' that corresponds to the 
   first occurence of the specified track group (belonging to type referred to by track_type_start, 
   track_group, and track_direction) */
static int find_start_of_track_group(INP int nx, INP int ny, INP t_rr_type chan_type, 
			INP t_seg_details *track_details, INP const char *type, 
			INP int track_group, INP int track_seg_coord, INP e_direction track_direction, 
			INP int track_type_start, INP int nodes_per_chan, INP t_track_type_sizes track_type_sizes);

/* fills the 'tracks_in_group' vector with the indices of the tracks in the 'track_details' channel
   which have the specified 'track_type', 'track_group', and are going in 'track_direction' */
static void get_group_tracks(INP int nx, INP int ny, INP t_seg_details *track_details, INP t_rr_type chan_type,
			INP const char *track_type, INP int track_group, INP int track_seg_coord, 
			INP e_direction track_direction, INP int nodes_per_chan, INP int type_start, 
			INP bool all_tracks_of_type, INP t_track_type_sizes &track_type_sizes, 
			INOUTP vector<int> *tracks);

/* Returns the switchpoint of the track specified by track_details at a segment coordinate
   of seg_coord, and connection to the sb_side of the switchblock */
static int get_switchpoint_of_track(INP int nx, INP int ny, INP e_rr_type chan_type,
		INP t_seg_details &track_details, INP int seg_coord, INP e_side sb_side);

static void get_switchpoint_tracks(INP int nx, INP int ny, INP t_seg_details *track_details, INP t_rr_type chan_type,
			INP const char *track_type, INP int track_switchpoint, INP int track_seg_coord, 
			INP e_direction track_direction, INP int nodes_per_chan, INP t_track_type_sizes &track_type_sizes, 
			INP e_side sb_side, INP bool all_tracks_of_type, INOUTP vector<int> *tracks);

/**** Function Definitions ****/
t_sb_connection_map * alloc_and_load_switchblock_permutations( INP t_chan_details * chan_details_x, 
				INP t_chan_details * chan_details_y, INP int nx, INP int ny, 
				INP vector<t_switchblock_inf> switchblocks, 
				INP int nodes_per_chan, INP e_directionality directionality){

	/* the switchblock permutations pointer */
	t_sb_connection_map *sb_conns = new t_sb_connection_map;

	/* We assume that x & y channels have the same ratios of track types. i.e., looking at a single 
	   channel is representative of all channels in the FPGA -- as of 3/9/2013 this is true in VPR */
	t_track_type_sizes track_type_sizes;
	count_track_type_sizes(chan_details_x[0][0], nodes_per_chan, &track_type_sizes);
	
	/* iterate over all the switchblocks specified in the architecture */
	for (int i_sb = 0; i_sb < (int)switchblocks.size(); i_sb++){
		t_switchblock_inf sb = switchblocks[i_sb];

		/* verify that switchblock type matches specified directionality -- currently we have to stay consistent */
		if (directionality != sb.directionality){
			vpr_printf(TIO_MESSAGE_ERROR, "alloc_and_load_switchblock_connections: Switchblock %s does not match directionality of architecture\n", sb.name.c_str());
			exit(1);
		}

		//TODO: it's better to build each switchblock once then stamp it out across the FPGA
		/* Iterate over the x,y coordinates spanning the FPGA. Currently, the FPGA size is set as
		   (0..nx+1) by (0..ny+1), so we iterate over 0..nx and 0..ny. */
		for (int x_coord = 0; x_coord <= nx+1; x_coord++){
			for (int y_coord = 0; y_coord <= ny+1; y_coord++){
				/* Iterate over each track in channel */
				for (int from_track = 0; from_track < nodes_per_chan; from_track++){

					/* now we iterate over all the potential side1->side2 connections */
					for ( e_side from_side = (e_side) 0; from_side < 4; from_side = (e_side)(from_side + 1) ){
						for ( e_side to_side = (e_side) 0; to_side < 4; to_side = (e_side)(to_side + 1) ){
							
							/* Fill appropriate entry of the sb_conns map with vector specifying the tracks 
							   the current track will connect to */
							compute_track_connections(x_coord, y_coord, from_side, to_side, from_track,
									chan_details_x, chan_details_y, &sb, nx, ny, nodes_per_chan,
									track_type_sizes, directionality, sb_conns);
							
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

/* Counts the number of tracks in each track type in the specified channel */
static void count_track_type_sizes(INP t_seg_details *channel, INP int nodes_per_chan, 
			INOUTP t_track_type_sizes *track_type_sizes){
	string track_type;
	const char * new_type;
	int num_tracks = 0;	

	track_type = channel[0].type_name_ptr;
	for (int itrack = 0; itrack < nodes_per_chan; itrack++){
		new_type = channel[itrack].type_name_ptr;
		if (strcmp(new_type, track_type.c_str()) != 0){
			(*track_type_sizes)[track_type] = num_tracks;
			track_type = new_type;
			num_tracks = 0;
		}
		num_tracks++;
	}
	(*track_type_sizes)[track_type] = num_tracks;

	return;
} 

/* Compute the track that the track at (x, y, from_side, to_side, from_track) should connect to.
   sb_conns is updated with the result */
static void compute_track_connections(INP int x_coord, INP int y_coord, INP enum e_side from_side,
			INP enum e_side to_side, INP int from_track, INP t_chan_details * chan_details_x,
			INP t_chan_details * chan_details_y, INP t_switchblock_inf *sb,
			INP int nx, INP int ny, INP int nodes_per_chan, INP t_track_type_sizes &track_type_sizes,
			INP e_directionality directionality, INOUTP t_sb_connection_map *sb_conns){
	
	t_chan_details *from_chan_details = NULL;	/* details for source channel */
	t_chan_details *to_chan_details = NULL ;	/* details for destination channel */
	int from_x, from_y;				/* index into source channel */
	int to_x, to_y;					/* index into destination channel */
	t_rr_type from_chan_type, to_chan_type;		/* the type of channel - i.e. CHANX or CHANY */
	e_direction src_direction, dest_direction;	/* the direction in which the source and destination tracks head */

	Connect_SB_Sides side_conn(from_side, to_side);		/* for indexing into this switchblock's permutation funcs */
	Switchblock_Lookup sb_conn(x_coord, y_coord, from_side, to_side, from_track);	/* for indexing into FPGA's switchblock map */

	/* can't connect a switchblock side to itself */
	if (from_side == to_side){
		return;
	}
	/* check that the permutation map has an entry for this side combination */
	t_permutation_map::const_iterator map_it = sb->permutation_map.find(side_conn);
	if (sb->permutation_map.end() == map_it){
		/* the specified switchblock does not have any permutation funcs for this side1->side2 connection */
		return;
	}

	/* find the correct channel, and the coordinates to index into it for both the source and
	   destination channels. also return the channel type (ie chanx/chany) into which we are 
	   indexing */
	from_chan_type = index_into_correct_chan(x_coord, y_coord, from_side, chan_details_x, chan_details_y, 
				&from_x, &from_y, &from_chan_details);
	to_chan_type = index_into_correct_chan(x_coord, y_coord, to_side, chan_details_x, chan_details_y, 
				&to_x, &to_y, &to_chan_details);

	/* make sure from_x/y and to_x/y aren't out of bounds */
	if (coords_out_of_bounds(nx, ny, to_x, to_y, to_chan_type) ||
	    coords_out_of_bounds(nx, ny, from_x, from_y, from_chan_type)){
		return;
	}

	/* get the direction in which the source/destination tracks are headed */
	src_direction = get_src_direction(from_side, directionality);
	dest_direction = get_dest_direction(to_side, directionality);	

	/* name of the source track */
	const char *track_name = from_chan_details[from_x][from_y][from_track].type_name_ptr;
	/* iterate over all the wire connections specified by the passed-in switchblock */
	for (int iconn = 0; iconn < (int)sb->wireconns.size(); iconn++){
		/* pointer to a connection specification between two wire types/groups */
		t_wireconn_inf *wireconn_ptr = &(sb->wireconns.at(iconn));

		/* name of track type we're connecting from/to */
		const char *from_track_type = wireconn_ptr->from_type.c_str();
		const char *to_track_type = wireconn_ptr->to_type.c_str();
		/* the 'seg' coordinates of the from/to channels */
		int from_seg = get_seg_coordinate( from_chan_type, from_x, from_y );
		int to_seg = get_seg_coordinate( to_chan_type, to_x, to_y );
		/* get the from/to switchpoints at which the connection is made */
		int from_switchpoint = get_switchpoint_of_track(nx, ny, from_chan_type,
				from_chan_details[from_x][from_y][from_track], from_seg, from_side);
		int to_switchpoint = wireconn_ptr->to_point;
		/* vectors that will contain indices of the source/dest tracks belonging to the source/dest switchpoints */
		vector<int> tracks_in_src_switchpoint;
		vector<int> tracks_in_dest_switchpoint;
		/* the index of the source/destination track within their own switchpoint group */
		int src_track_in_sp, dest_track_in_sp;
		/* the effective destination channel width is the size of the destination track group */
		int dest_W;
		/* indicates whether all source/destination tracks belonging to the given track type are to be considered */
		bool all_source_type_tracks = false, all_dest_type_tracks = false;
		if (-1 == wireconn_ptr->from_point){
			all_source_type_tracks = true;
		}
		if (-1 == wireconn_ptr->to_point){
			all_dest_type_tracks = true;
		}

		/* check that the current track has the type specified by the wire connection */
		if ( strcmp(track_name, from_track_type) != 0 ){	
			continue;
		}
		/* check that the current track has the switch point specified by the wire connection */
		if ( from_switchpoint != wireconn_ptr->from_point && !all_source_type_tracks ){
			continue;
		}

		/* now we need to find all tracks in the destination channel that correspond to the 
		   type and group specified by the current wireconn_ptr. Must also go in appropriate 
		   direction (from e_direction) */
		
		/* get the indices of tracks in the destination group, as well as the effective destination 
		   channel width (which is the number of tracks in destination group) */
		get_switchpoint_tracks(nx, ny, to_chan_details[to_x][to_y], to_chan_type, to_track_type, to_switchpoint, to_seg, 
						dest_direction, nodes_per_chan, track_type_sizes, to_side, all_dest_type_tracks, &tracks_in_dest_switchpoint);
		dest_W = tracks_in_dest_switchpoint.size();

		/* get the index of the source track relative to all the tracks with the same switchpoint */
		get_switchpoint_tracks(nx, ny, from_chan_details[from_x][from_y], from_chan_type, from_track_type, from_switchpoint, 
						from_seg, src_direction, nodes_per_chan, track_type_sizes, from_side, all_source_type_tracks,
						&tracks_in_src_switchpoint);
		vector<int>::iterator it = find(tracks_in_src_switchpoint.begin(), tracks_in_src_switchpoint.end(), from_track);
		src_track_in_sp = it - tracks_in_src_switchpoint.begin();
		
		/* get a reference to the string vector containing desired side1->side2 permutations */
		vector<string> &permutations_ref = sb->permutation_map.at(side_conn);

		/* iterate over the permutation functions specified in the string vector */
		int to_track;
		s_formula_data formula_data;
		for (int iperm = 0; iperm < (int)permutations_ref.size(); iperm++){
			/* Convert the symbolic permutation formula to a number */
			formula_data.dest_W = dest_W;
			if (0 == dest_W){
				continue;
			}
			formula_data.track = src_track_in_sp;
			dest_track_in_sp = get_sb_formula_result(permutations_ref.at(iperm).c_str(), formula_data);
			dest_track_in_sp = (dest_track_in_sp + dest_W) % (dest_W);
			/* the resulting track number is the *index* of the destination track in it's own
			   group, so we need to convert that back to the absolute index of the track in the channel */
			//printf("formula: %s  track: %d  W: %d  dest_track: %d\n", permutations_ref.at(iperm).c_str(), src_track_in_sp, dest_W, dest_track_in_sp); 
			if(dest_track_in_sp < 0){
				vpr_printf(TIO_MESSAGE_ERROR, "compute_track_connections: got a negative track, %d\n", dest_track_in_sp);
				exit(1);
			}
			to_track = tracks_in_dest_switchpoint.at(dest_track_in_sp);
			
			//debugging
			//if (x_coord == 3 && y_coord == 1)
			//	printf("W: %d  from_s: %d  to_s: %d  tile_x: %d  tile_y: %d  from_x: %d  from_y: %d  to_x: %d  to_y: %d  from_trk: %d  src: %d  dest: %d  dest_W: %d  to_trk: %d\n", nodes_per_chan, from_side, to_side, x_coord, y_coord, from_x, from_y, to_x, to_y, from_track, src_track_in_sp, dest_track_in_sp, dest_W, to_track);

			/* create the struct containing the to_track and switch name, which will be added to the 
			   sb connections map */	
			t_to_track_inf to_track_inf;
			to_track_inf.to_track = to_track;
			to_track_inf.switch_name = sb->switch_name;
			/* and now, finally, add this switchblock connection to the switchblock connections map */
			(*sb_conns)[sb_conn].push_back(to_track_inf);
			/* If bidir architecture, implement the reverse connection as well */
			if (BI_DIRECTIONAL == directionality){
				to_track_inf.to_track = from_track;
				Switchblock_Lookup sb_conn_reverse(x_coord, y_coord, to_side, from_side, to_track);
				(*sb_conns)[sb_conn_reverse].push_back(to_track_inf);
			}
		}
	}

	track_name = NULL;
	
	from_chan_details = NULL;
	to_chan_details = NULL;
	return;
}


/* Here we find the correct channel (x or y), and the coordinates to index into it based on the 
   specified tile coordinates and the switchblock side. Also returns the type of channel
   that we are indexing into (ie, CHANX or CHANY */
static t_rr_type index_into_correct_chan(INP int tile_x, INP int tile_y, INP enum e_side side, 
			INP t_chan_details *chan_details_x, INP t_chan_details *chan_details_y,
			OUTP int *set_x, OUTP int *set_y, 
			OUTP t_chan_details **set_chan_details){
	
	t_rr_type chan_type;

	/* here we use the VPR convention that a tile 'owns' the channels directly to the right
	   and above it */
	switch (side){
		case TOP:
			/* this is y-channel belonging to tile above */
			*set_chan_details = chan_details_y;
			*set_x = tile_x;
			*set_y = tile_y+1;
			chan_type = CHANY;
			break;
		case RIGHT:
			/* this is x-channel belonging to tile to the right */
			*set_chan_details = chan_details_x;
			*set_x = tile_x+1;
			*set_y = tile_y;
			chan_type = CHANX;
			break;
		case BOTTOM:
			/* this is y-channel on the right of the tile */
			*set_chan_details = chan_details_y;
			*set_x = tile_x;
			*set_y = tile_y;
			chan_type = CHANY;
			break;
		case LEFT:
			/* this is x-channel on top of the tile */
			*set_chan_details = chan_details_x;
			*set_x = tile_x;
			*set_y = tile_y;
			chan_type = CHANX;
			break;
		default:
			vpr_printf(TIO_MESSAGE_ERROR, "index_into_correct_chan: unknown side specified: %d\n", side);
			exit(1);
			break;
	}

	return chan_type;
}

/* checks whether the specified coordinates are out of bounds */
static bool coords_out_of_bounds(INP int nx, INP int ny, INP int x_coord, INP int y_coord, 
			e_rr_type chan_type){
	bool result = true;

	if (CHANX == chan_type){
		if (x_coord <=0 || x_coord >= nx+1 ||	/* there is no x-channel at x=0 */
		    y_coord < 0 || y_coord >= ny+1){
			result = true;
		} else {
			result = false;
		}
	} else if (CHANY == chan_type){
		if (x_coord < 0 || x_coord >= nx+1 ||
		    y_coord <= 0 || y_coord >= ny+1){	/* there is no y-channel at y=0 */
			result = true;
		} else {
			result = false;
		}

	} else {
		vpr_printf(TIO_MESSAGE_ERROR, "coords_out_of_bounds(): illegal channel type %d\n", chan_type);
		exit(1);
	}
	return result;
}

/* We can look at a channel as either having an x and y coordinate, or as having a 
   'seg' and 'chan' coordinate. Here we return the seg coordinate based on the 
   channel type and channel x/y coordinates */
static int get_seg_coordinate(INP t_rr_type chan_type, INP int x_coord, INP int y_coord){
	int seg = -1;
	
	switch (chan_type){
		case CHANX:
			seg = x_coord;
			break;
		case CHANY:
			seg = y_coord;
			break;
		default:
			vpr_printf(TIO_MESSAGE_ERROR, "get_seg_coordinate: 'chan_type' %d is not a channel\n", chan_type);
			exit(1);
			break;
	}

	return seg;
}

/* returns the direction in which a source track at the specified
   switchblock side should be headed. i.e. if we connect from a track on the top
   side of a switchblock in a unidir architecture, this track should be
   going in the -y direction (aka in the direction of 'decreasing' coordinate) . */
static e_direction get_src_direction(e_side from_side, e_directionality directionality){
	e_direction src_dir;

	if (BI_DIRECTIONAL == directionality){
		src_dir = BI_DIRECTION;
	} else if (UNI_DIRECTIONAL == directionality){
		if (TOP == from_side || RIGHT == from_side){
			/* in direction of decreasing coordinate */
			src_dir = DEC_DIRECTION;
		} else if (LEFT == from_side || BOTTOM == from_side){
			/* in direction of increasing coordinate */
			src_dir = INC_DIRECTION;
		} else {
			vpr_printf(TIO_MESSAGE_ERROR, "get_src_direction(): unkown switchblock side %d\n", from_side);
			exit(1);
		}
	} else {
		vpr_printf(TIO_MESSAGE_ERROR, "get_src_direction(): unknown directionality %d\n", directionality);
		exit(1);
	}

	return src_dir;
}

/* returns the direction in which a destination track at the specified
   switchblock side should be headed. i.e. if we connect to a track on the top
   side of a switchblock in a unidir architecture, this track should be
   going in the +y direction (aka in the direction of 'increasing' coordinate) . */
static e_direction get_dest_direction(e_side to_side, e_directionality directionality){
	e_direction dest_dir;

	if (BI_DIRECTIONAL == directionality){
		dest_dir = BI_DIRECTION;
	} else if (UNI_DIRECTIONAL == directionality){
		if (TOP == to_side || RIGHT == to_side){
			/* in direction of increasing coordinate */
			dest_dir = INC_DIRECTION;
		} else if (LEFT == to_side || BOTTOM == to_side){
			/* in direction of decreasing coordinate */
			dest_dir = DEC_DIRECTION;
		} else {
			vpr_printf(TIO_MESSAGE_ERROR, "get_dest_direction(): unkown switchblock side %d\n", to_side);
			exit(1);
		}
	} else {
		vpr_printf(TIO_MESSAGE_ERROR, "get_dest_direction(): unknown directionality %d\n", directionality);
		exit(1);
	}

	return dest_dir;
}

/* returns the group number of the specified track */
static int get_track_group(INP int nx, INP int ny, INP e_rr_type chan_type,
			 INP t_seg_details &track_details, int seg_coord){
	
	/* We get track group by comparing the track's seg_coord to the seg_start of the track.
	   The offset between seg_start and seg_coord is the group number */

	/* ISSUE: seg start is the actual start of the segment. i.e. at seg=1, all tracks have a 
	start point at seg=1. 
	One solution is to look at both seg_start and seg_end. Is that enough info?
	Cases:
	seg starts at bottom but does not extend all the way to the top -- look at seg_end
	seg starts > bottom and does not extend all the way to top -- look at seg_start or seg_end
	seg starts > bottom but terminates all the way at the top -- look at seg_start
	seg starts at bottom and extends all the way to the top -- must be group 0
	*/


	int group;
	int seg_start = track_details.seg_start;
	int seg_end = track_details.seg_end;
	int track_length = track_details.length;
	int max_seg, min_seg;

	/* determine the minimum and maximum values that the 'seg' coordinate 
	   of a track can take */
	min_seg = 1;	
	if (CHANX == chan_type){
		max_seg = nx;
	} else {
		assert(CHANY == chan_type);
		max_seg = ny;
	}

	if (seg_start != min_seg){
		group = seg_coord - seg_start;
	} else if (seg_end != max_seg){
		group = (track_length-1) - (seg_end - seg_coord);
	} else {
		/* track must span the entire FPGA */
		//assert( track_details.longline ); TODO: confirm what longline is
		group = 0;
	}

	/* the start of this track cannot be *after* the current seg coordinate */
	if (group < 0){
		vpr_printf(TIO_MESSAGE_ERROR, "get_track_group: invalid seg_start, seg_coord combination\n");
		exit(1); 
	}

	return group;
}

/* Finds the index of the first track in the channel specified by 'track_details'
   to have the type name specified by 'type' */
static int find_start_of_track_type(INP t_seg_details *track_details, INP const char *type, 
			INP int nodes_per_chan, INP t_track_type_sizes track_type_sizes){

	string map_accessor;
	int track = 0;
	while( track < nodes_per_chan ){
		/* check type of track */
		if ( strcmp(type, track_details[track].type_name_ptr) == 0 ){
			/* track type found */
			break;
		} else {
			/* move on to next track type */
			map_accessor = track_details[track].type_name_ptr;
			track += track_type_sizes[map_accessor];
		}
	}

	if (track >= nodes_per_chan){
		vpr_printf(TIO_MESSAGE_ERROR, "find_start_of_track_type: track type %s could not be found in the channel\n", type);
		exit(1);
	}

	return track;
} 

/* returns the track index in the channel specified by 'track_details' that corresponds to the 
   first occurence of the specified track group (belonging to type referred to by track_type_start, 
   track_group, and track_direction) */
static int find_start_of_track_group(INP int nx, INP int ny, INP t_rr_type chan_type, 
			INP t_seg_details *track_details, INP const char *type, 
			INP int track_group, INP int track_seg_coord, INP e_direction track_direction, 
			INP int track_type_start, INP int nodes_per_chan, INP t_track_type_sizes track_type_sizes){
	string map_accessor = type;
	int num_tracks = track_type_sizes.at(map_accessor);
	int track_type_end = track_type_start + num_tracks - 1;	

	int track;
	for (track = track_type_start; track <= track_type_end; track++){
		e_direction direction = track_details[track].direction;
		int group = get_track_group(nx, ny, chan_type, track_details[track], track_seg_coord);
		if (group == track_group && direction == track_direction){
			/* found the requested track group */
			break;
		}

		/* if we hit the last track but still haven't found the group... */
		if (track == track_type_end){
			vpr_printf(TIO_MESSAGE_ERROR, "track group %d could not be found in the channel\n", track_group);
			exit(1);
		}
	}

	return track;
}

/* fills the 'tracks_in_group' vector with the indices of the tracks in the 'track_details' channel
   which have the specified 'track_type', 'track_group', and are going in 'track_direction' */
static void get_group_tracks(INP int nx, INP int ny, INP t_seg_details *track_details, INP t_rr_type chan_type,
			INP const char *track_type, INP int track_group, INP int track_seg_coord, 
			INP e_direction track_direction, INP int nodes_per_chan, INP int type_start, 
			INP bool all_tracks_of_type, INP t_track_type_sizes &track_type_sizes, 
			INOUTP vector<int> *tracks){
	int group_size;			/* the number of tracks in the channel belonging to specified type->group */
	int group_start;		/* track index at which the given 'group' starts */
	int track_type_end;		/* track index where the track type ends */
	int track_length;		/* length of track */
	int dir_mult;			/* multiplier to account for unidir/bidir differences */
	int step_size;			/* spacing between two consecutive tracks of the same group/type */
	int num_tracks;			/* the total number of tracks which will be put int 'tracks' */
	int track_start;		/* the index of the first track */
	
	/* multiplier to account for unidir vs bidir differences */
	if (BI_DIRECTION == track_direction){
		dir_mult = 1;
	} else if (INC_DIRECTION == track_direction || DEC_DIRECTION == track_direction){
		dir_mult = 2;
	} else {
		vpr_printf(TIO_MESSAGE_ERROR, "get_group_size(): unknown track direction %d\n", track_direction);
		exit(1);
	}
	
	string map_accessor = track_type;
	if (all_tracks_of_type){
		/* considering all tracks of the given type */
		num_tracks = track_type_sizes.at(map_accessor);
		step_size = 1;
		track_start = type_start;
	} else {
		/* considering tracks which are currently at a certain switchpoint */
		/* get index of track in channel at which the specified type->group starts */
		group_start = find_start_of_track_group(nx, ny, chan_type, track_details, track_type, track_group, 
						track_seg_coord, track_direction, type_start, nodes_per_chan, 
						track_type_sizes);
		
		track_type_end = type_start + track_type_sizes.at(map_accessor) - 1;
		track_length = track_details[group_start].length;

		/* get the group size. Consecutive tracks of the same group
		   are separated by 'track_length' tracks for the bidirectional case,
		   and by 2*track_length tracks for the unidirectional case, because for 
		   unidir tracks are grouped in pairs (the dir_mult factor accounts for this). */
		step_size = track_length * dir_mult;
		group_size = track_type_end - group_start;
		group_size = floor(group_size / step_size) + 1;
		num_tracks = group_size;
		track_start = group_start;
	}

	/* and now we set which track indices in the channel correspond to the 
	   desired type, group, and directionality */
	tracks->reserve(num_tracks);
	for (int i = 0; i < num_tracks; i++){
		tracks->push_back( track_start + i*step_size );
	}

	return;
}

/* Returns the switchpoint of the track specified by track_details at a segment coordinate
   of seg_coord, and connection to the sb_side of the switchblock */
static int get_switchpoint_of_track(INP int nx, INP int ny, INP e_rr_type chan_type,
		 INP t_seg_details &track_details, INP int seg_coord, INP e_side sb_side){

	/* this function calculates the switchpoint of a given track by first calculating
	   the group of the specified track. For instance, for a track with L=4:

		switchpoint:	0-------1-------2-------3-------0
		      group:	    0       1       2       3

	   So knowing the track's group and which switchblock side it connects to is 
	   enough to calculate the switchpoint

	*/

	int switchpoint;
	int track_length = track_details.length;
	int group = get_track_group(nx, ny, chan_type, track_details, seg_coord);

	if (LEFT == sb_side || BOTTOM == sb_side){
		switchpoint = (group + 1) % track_length;
	} else {
		assert(RIGHT == sb_side || TOP == sb_side);
		switchpoint = group;
	}

	return switchpoint;
}

/* in this function we'd like to find all the tracks belonging to the specified switchpoint.
   since the switchpoint is related to the track group as:

	switchpoint:	0-------1-------2-------3-------0
	      group:	    0       1       2       3

   we can find the tracks belonging to the switchpoint by finding the tracks belonging to
   the appropriate group.
   The result is stored in the 'tracks' vector */
static void get_switchpoint_tracks(INP int nx, INP int ny, INP t_seg_details *track_details, INP t_rr_type chan_type,
			INP const char *track_type, INP int track_switchpoint, INP int track_seg_coord, 
			INP e_direction track_direction, INP int nodes_per_chan, INP t_track_type_sizes &track_type_sizes, 
			INP e_side sb_side, INP bool all_tracks_of_type, INOUTP vector<int> *tracks){

	int track_group;
	int type_start = find_start_of_track_type(track_details, track_type, nodes_per_chan, track_type_sizes);
	int track_length = track_details[type_start].length;

	/* get the track group based on the desired switchpoint */
	if (LEFT == sb_side || BOTTOM == sb_side){
		track_group = (track_switchpoint - 1 + track_length) % track_length ;
	} else {
		assert(RIGHT == sb_side || TOP == sb_side);
		track_group = track_switchpoint;
	}

	/* fill 'tracks' with the indices of the track corresponding to the desired group/switchpoint */
	get_group_tracks(nx, ny, track_details, chan_type, track_type, track_group, track_seg_coord, track_direction, 
		nodes_per_chan, type_start, all_tracks_of_type, track_type_sizes, tracks);

	return;
}


/* checks whether the entry specified by coord exists in the switchblock map sb_conns */
bool sb_connection_exists( INP Switchblock_Lookup coord, INP t_sb_connection_map *sb_conns ){
	bool result;
	if (NULL == sb_conns){
		vpr_printf(TIO_MESSAGE_ERROR, "sb_connection_exists: sb_conns pointer is NULL\n");
		exit(1);
	}

	t_sb_connection_map::const_iterator it = (*sb_conns).find(coord);
	if ((*sb_conns).end() != it){
		result = true;
	} else {
		result = false;
	}

	return result;
}
