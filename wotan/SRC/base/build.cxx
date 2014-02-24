
#include <cmath>
#include <cstring>
#include <algorithm>
#include "build.h"
#include "io.h"
#include "exception.h"
#include "parse_switchblocks.h"

using namespace std;

/**** Definitions ****/
#define REF_X 0
#define REF_Y 0


/**** Function Declarations ****/

/* Compute the track that the track at (coord, from_side, to_side, from_track) should connect to.
   sb_conns is updated with the result */
static void compute_track_connections(INP Coordinate coord, INP enum e_side from_side,
		INP enum e_side to_side, INP int from_track, INP t_chan_map *chan_map,
		INP t_switchblock_inf *sb, INP User_Options user_opts, 
		INOUTP t_sb_connection_map *sb_conns);
/* computes a horizontal line of switchblocks of size sb_line_size (or of nx-2, whichever is smaller), starting 
   at coordinate (1,1) */
static void compute_switchblock_line(INP int sb_line_size, INP t_chan_map *chan_map,
		INP vector<t_switchblock_inf> *switchblocks, INP User_Options user_opts, 
		INOUTP t_sb_connection_map *sb_line);
/* stamp out a line of horizontal switchblocks throughout the entire test area */
static void stampout_switchblock_line( INP int sb_line_size, INP User_Options user_opts,
		INP t_sb_connection_map *sb_line, INOUTP t_sb_connection_map *sb_conns );
/* want to have the same number of tracks per wire type as VPR. This function
   basically copies what VPR does to get the count of track per wire type*/
static void get_type_track_count(INP int num_sets, INP int num_seg_types, INP t_segment_inf *segment_inf,
		INOUTP vector<int> *type_track_count);
/* Here we find the correct channel type (CHANX or CHANY), and the coordinates to index into it based on the 
   specified tile coordinates and the switchblock side.  */
static e_chan_type index_into_correct_chan(INP Coordinate tile_coord, INP enum e_side side, 
			INOUTP Coordinate *coord_out);
/* Returns the wirepoint of the track based on its group and the switchblock side */
static int get_wirepoint_of_track(INP Track_Info *track_info, INP e_side sb_side);
/* returns the track indices belonging to the types in 'track_types' and wirepoints in 'points' at the given 
   coordinates/side */ 
static void get_wirepoint_tracks( INP t_chan_map *chan_map, INP User_Options *user_opts, INP Coordinate coord,
			INP e_chan_type chan_type, INP e_side side, INP vector<string> *track_types, INP vector<int> *points,
			INP bool is_dest, INOUTP vector<int> *tracks);
/* adjusts a negative destination track calculated from a permutation formula */
static int adjust_formula_result(INP int dest_track, INP int dest_W);
/* get the length for each wire type */
static void get_wire_type_lengths(INP User_Options user_opts, INP t_chan_map *chan_map, 
			INOUTP map<string, int> *track_type_lengths);
/* over all connected wire types (i,j), compute the maximum least common multiple of their track lengths, 
   ie max(LCM(L_i, L_J)) */
static int get_max_lcm( INP vector<t_switchblock_inf> *switchblocks, INP map<string, int> *track_type_lengths);

/* conditionally adds a node based on passed-in parameters to the node map. node is only added
   if the track segment to which it corresponds is in group 0. i.e., one node represents
   an entire length of track */
static void add_node(INP User_Options user_opts, INP Coordinate coord, e_chan_type chan_type, INP int track, INP t_chan_map *chan_map, 
		INOUTP t_node_map *node_map);
/* creates edges for, and connects nodes that correspond to the coord/side/track/sb_conns parameters */
static void add_edge(INP User_Options user_opts, INP Coordinate coord, INP e_side from_side, INP e_side to_side,
		INP int from_track, INP t_sb_connection_map *sb_conns, INP t_chan_map *chan_map, INOUTP t_node_map *node_map);
static int get_conn_point(INP e_side side, INP int track_group);


/**** Function Definitions ****/

/* build the switchblock connections map based on switchblock permutations and the channel map */
void build_sb_permutation_map( INP t_chan_map *chan_map, INP User_Options user_opts, 
		INP vector<t_switchblock_inf> *switchblocks, INOUTP t_sb_connection_map *sb_conns ){

	W_PRINT("Building Switchblocks...\n");

	/* get a quick lookup for the lengths of each wire type */
	map<string, int> track_type_lengths;
	get_wire_type_lengths(user_opts, chan_map, &track_type_lengths);

	/* figure out max(lcm(L_i, L_j)) for all wire lengths belonging to wire types i & j */
	int max_lcm = get_max_lcm(switchblocks, &track_type_lengths);
	
	/* compute a representative line of switchblocks which will then be stamped out through the entire test area */
	t_sb_connection_map sb_line;
	compute_switchblock_line( max_lcm, chan_map, switchblocks, user_opts, &sb_line );

	/* stampout the precomputed switchblock line throughout the entire FPGA test area */
	stampout_switchblock_line(max_lcm, user_opts, &sb_line, sb_conns);
}

/* stamp out a line of horizontal switchblocks throughout the entire test area */
static void stampout_switchblock_line( INP int sb_line_size, INP User_Options user_opts,
		INP t_sb_connection_map *sb_line, INOUTP t_sb_connection_map *sb_conns ){

	/* over all x coordinates that may need stamping out*/
	for (int x = 0; x < user_opts.x; x++){
		/* over all y coordinates that may need stamping out */
		for (int y = 0; y < user_opts.y; y++){
			/* over each source side */
			for (int from_side = 0; from_side < 4; from_side ++){
				/* over each destination side */
				for (int to_side = 0; to_side < 4; to_side ++){
					/* can't connect a side to itself */
					if (from_side == to_side){
						continue;
					}
					/* over each source track */
					for (int itrack = 0; itrack < user_opts.W; itrack++){
						/* The switchblock like was stamped computed starting at (1,1).
						   get the total x+y distance between this and the (1,1) coordinate */
						int distance = (x-REF_X) + (y-REF_Y);
						if (distance < 0){
							distance = sb_line_size - ((-1*distance)%sb_line_size);
						}
						/* figure out the coordinates of the switchblock we want to copy */
						int copy_y = REF_Y;
						int copy_x = REF_X + (distance % sb_line_size);

						/* create the indices to key into the switchblock permutation map */
						Switchblock_Lookup my_key(x, y, from_side, to_side, itrack);
						Switchblock_Lookup copy_key(copy_x, copy_y, from_side, to_side, itrack);


						//TODO: sometimes crashes if we don't do this. figure out why.
						if ( !map_key_exists(copy_key, sb_line) ){
							continue;
						}

						//cout << "my key: " << endl;
						//cout << "  x: " << x << "  y: " << y << "  fs: " << from_side << "  ts: " << to_side << "  track: " << itrack << endl;
						//cout << "copy key: " << endl;
						//cout << "  x: " << copy_x << "  y: " << copy_y << "  fs: " << from_side << "  ts: " << to_side << "  track: " << itrack << endl;

						//TODO: dies on small test sizes. not all of the switchblock line is built, 
						//	and not enough channels are available for the entire sample switchblock
						//	line to be built. solution: do not build in a line, but rather build
						//	in-place s.t. the sample switchblocks never go out of bounds anyway.
						(*sb_conns)[my_key] = sb_line->at(copy_key);
					}
				}
			}
		}
	}
}

/* computes a horizontal line of switchblocks of size sb_line_size (or of nx-2, whichever is smaller), starting 
   at coordinate (1,1) */
static void compute_switchblock_line(INP int sb_line_size, INP t_chan_map *chan_map,
		INP vector<t_switchblock_inf> *switchblocks, INP User_Options user_opts, 
		INOUTP t_sb_connection_map *sb_line){

	int y = REF_Y;
	for (int isb = 0; isb < (int)switchblocks->size(); isb++){
		t_switchblock_inf *sb = &(switchblocks->at(isb));
		for (int x = REF_X; x < REF_X + sb_line_size; x++){
			/* Currently we do not care if a switchblock in the architecture has been specified
			   as being limited to a certain region of the FPGA. They all get treated as 'EVERYWHERE' */
			/* Iterate over each track in channel */
			for (int from_track = 0; from_track < user_opts.W; from_track++){
				/* now we iterate over all the potential side1->side2 connections */
				for ( e_side from_side = (e_side) 0; from_side < 4; from_side = (e_side)(from_side + 1) ){
					for ( e_side to_side = (e_side) 0; to_side < 4; to_side = (e_side)(to_side + 1) ){
						Coordinate coord(x, y);

						/* Fill appropriate entry of the sb_conns map with vector specifying the tracks 
						   the current track will connect to */
						compute_track_connections(coord, from_side, to_side, from_track,
								chan_map, sb, user_opts, sb_line);
					}
				}
			}
		}
	}
}


/* Compute the track that the track at (coord, from_side, to_side, from_track) should connect to.
   sb_conns is updated with the result */
static void compute_track_connections(INP Coordinate coord, INP enum e_side from_side,
		INP enum e_side to_side, INP int from_track, INP t_chan_map *chan_map,
		INP t_switchblock_inf *sb, INP User_Options user_opts, 
		INOUTP t_sb_connection_map *sb_conns){

	Coordinate from_coord, to_coord;	/* coordinates of source/destination channels */
	e_chan_type from_chan, to_chan;		/* types of the source/destination channels */

	Connect_SB_Sides side_conn(from_side, to_side);		/* for indexing into this switchblock's permutation funcs */
	Switchblock_Lookup sb_conn(coord.x, coord.y, from_side, to_side, from_track);	/* for indexing into FPGA's switchblock map */

	/* can't connect a switchblock side to itself */
	if (from_side == to_side){
		return;
	}

	/* check that the permutation map has an entry for this side combination */
	if( !map_key_exists(side_conn, &(sb->permutation_map)) ){
		/* the specified switchblock does not have any permutation funcs for this side1->side2 connection */
		return;
	}

	/* the passed-in coord gives the coordinate of the switchblock which is not necessarily the same
	   as that of 'from_track'. so we need to index into the correct from/to channels and get correct coords */
	from_chan = index_into_correct_chan(coord, from_side, &from_coord);
	to_chan = index_into_correct_chan(coord, to_side, &to_coord);

	/* make sure that the source/destination coordinates aren't out of bounds */
	if (coords_out_of_bounds(user_opts, from_coord, from_chan) ||
	    coords_out_of_bounds(user_opts, to_coord, to_chan)){
		return;
	}

	/* key to index into the source channel map */
	Track_Coord track_coord_from(from_coord, from_chan, from_track);

	/* name of the source track */
	const char *track_name = chan_map->at(track_coord_from).type_name_ptr;
	/* iterate over all the wire connections specified by the passed-in switchblock */
	for (int iconn = 0; iconn < (int)sb->wireconns.size(); iconn++){
		/* pointer to a connection specification between two wire types/groups */
		t_wireconn_inf *wireconn_ptr = &(sb->wireconns.at(iconn));
		
		/* names of track type we may be connecting from/to */
		vector<string> *from_track_type = &(wireconn_ptr->from_type);
		vector<string> *to_track_type = &(wireconn_ptr->to_type);
		/* the 'from' wirepoint and 'to' wirepoints at which the connection is made */
		vector<int> from_wirepoint, to_wirepoint;
		
		/* the current track corresponds to a single wirepoint -- vector will have only one entry */
		from_wirepoint.push_back( get_wirepoint_of_track(&chan_map->at(track_coord_from), from_side) );
		/* there may be multiple destination wirepoints */
		to_wirepoint = wireconn_ptr->to_point;
		
		/* vectors that will contain indices of the source/dest tracks belonging to the source/dest wirepoints */
		vector<int> potential_src_tracks;
		vector<int> potential_dest_tracks;
		/* the index of the source/destination track within their own wirepoint group */
		int src_track_in_sp, dest_track_in_sp;
		/* the effective destination channel width is the size of the destination track group */
		int dest_W;

		/* check that the current track has one of the types specified by the wire connection */
		bool skip = true;
		for (int itype = 0; itype < (int)from_track_type->size(); itype++){
			if ( strcmp(from_track_type->at(itype).c_str(), track_name) == 0 ){
				skip = false;
				break;
			}
		}
		if (skip){
			continue;
		}

		/* check that the current track has one of the source wire points specified by the wire connection */
		skip = true;
		for (int ipoint = 0; ipoint < (int)wireconn_ptr->from_point.size(); ipoint++){
			if (from_wirepoint.at(0) == wireconn_ptr->from_point.at(ipoint)){
				skip = false;
				break;
			}
		}
		if (skip){
			continue;
		}

		
		/* now we need to find all tracks in the destination channel that correspond to the 
		   type and group specified by the current wireconn_ptr. Must also go in appropriate 
		   direction (i.e. INC/DEC if unidirectional) */

		/* get the indices of the potential destination tracks, as well as the effective destination 
		   channel width (which is the number of tracks in destination group) */
		get_wirepoint_tracks( chan_map, &user_opts, to_coord, to_chan, to_side, to_track_type, &to_wirepoint,
				true, &potential_dest_tracks);
		if (0 == potential_dest_tracks.size()){
			continue;
		}
		dest_W = potential_dest_tracks.size();

		/* get the index of the source track relative to all the tracks specified by the wireconn */
		get_wirepoint_tracks( chan_map, &user_opts, from_coord, from_chan, from_side, from_track_type,
				&wireconn_ptr->from_point, false, &potential_src_tracks);
		if (0 == potential_src_tracks.size()){
			continue;
		}
		vector<int>::iterator it = find(potential_src_tracks.begin(), potential_src_tracks.end(), from_track);
		src_track_in_sp = it - potential_src_tracks.begin();

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
			dest_track_in_sp = adjust_formula_result(dest_track_in_sp, dest_W);
			/* the resulting track number is the *index* of the destination track in it's own
			   group, so we need to convert that back to the absolute index of the track in the channel */
			if(dest_track_in_sp < 0){
				W_THROW("compute_track_connections: got a negative track, " << dest_track_in_sp << " based on source track " << src_track_in_sp
					<< ". From side " << from_side << " to side " << to_side << ". W=" << user_opts.W << ". Permutation function: " 
					<< permutations_ref.at(iperm) << endl);
			}
			to_track = potential_dest_tracks.at(dest_track_in_sp);
			
			/* create the struct containing the to_track and switch name, which will be added to the 
			   sb connections map */	
			t_to_track_inf to_track_inf;
			to_track_inf.to_track = to_track;
			to_track_inf.switch_name = sb->switch_name;
			/* and now, finally, add this switchblock connection to the switchblock connections map */
			(*sb_conns)[sb_conn].push_back(to_track_inf);
			/* If bidir architecture, implement the reverse connection as well */
			if (BI_DIRECTIONAL == user_opts.directionality){
				to_track_inf.to_track = from_track;
				Switchblock_Lookup sb_conn_reverse(coord.x, coord.y, to_side, from_side, to_track);
				(*sb_conns)[sb_conn_reverse].push_back(to_track_inf);
			}
		}
	}
}

/* adjusts a negative destination track calculated from a permutation formula */
static int adjust_formula_result(INP int dest_track, INP int dest_W){
	int result = dest_track;
	if (dest_track < 0){
		int mult = (-1*dest_track) / dest_W + 1;
		result = dest_track + mult*dest_W;
	}
	result = (result + dest_W) % dest_W;
	return result;
}

/* returns the track indices belonging to the types in 'track_types' and wirepoints in 'points' at the given 
   coordinates/side */ 
static void get_wirepoint_tracks( INP t_chan_map *chan_map, INP User_Options *user_opts, INP Coordinate coord,
			INP e_chan_type chan_type, INP e_side side, INP vector<string> *track_types, INP vector<int> *points,
			INP bool is_dest, INOUTP vector<int> *tracks){

	/* factor for adjusting for unidirectional/bidirectional architectures */
	int dir_fac;
	if (BI_DIRECTIONAL == user_opts->directionality){
		dir_fac = 1;
	} else if (UNI_DIRECTIONAL == user_opts->directionality){
		dir_fac = 2;
	} else {
		W_THROW("Illegal directionality: " << user_opts->directionality);
	}


	/* want to walk over channel, skipping those tracks that do not belong to one of the given types */
	int itrack = 0;
	Track_Coord track_coord(coord, chan_type, itrack);
	while (itrack < user_opts->W){
		string type( chan_map->at(track_coord).type_name_ptr );
		int type_end = chan_map->at(track_coord).type_end; 
		/* check that this is one of the track types specified */
		vector<string>::const_iterator itt = find(track_types->begin(), track_types->end(), type);
		if ( track_types->end() == itt ){
			/* move on to the next track type */
			itrack = type_end + 1;
			track_coord.track_index = itrack;
			continue;
		}

		/* for unidirectional arch, the first track of a given type always goes in the INC direction.
		   if the tracks we are looking for are meant to be connected *TO*, then we have to make sure 
		   that if we're on left or bottom we start on the track that goes in the DEC direction */
		if (UNI_DIRECTIONAL == user_opts->directionality){
			if (is_dest && (side == LEFT || side == BOTTOM)){
				itrack++;
			}
		}

		/* walk through this type's tracks and check the wirepoint of each */
		for ( ; itrack <= type_end; itrack+=dir_fac){
			track_coord.track_index = itrack;
			/* get wirepoint of this track */
			int wirepoint = get_wirepoint_of_track(&chan_map->at(track_coord), side);
			/* check that this wirepoint exists in the 'points' map */
			vector<int>::const_iterator itp = find(points->begin(), points->end(), wirepoint);
			if (points->end() == itp){
				/* move on to next track */
				continue;
			}
			
			tracks->push_back(itrack);
		}
		track_coord.track_index = itrack;
	}
}

/* Returns the wirepoint of the track based on its group and the switchblock side */
static int get_wirepoint_of_track(INP Track_Info *track_info, INP e_side sb_side){

	/* this function calculates the wirepoint of a given track based on
	   the group of the specified track. For instance, for a track with L=4:

		wirepoint:	0-------1-------2-------3-------0
		      group:	    0       1       2       3

	   So knowing the track's group and which switchblock side it connects to is 
	   enough to calculate the wirepoint
	*/

	int wirepoint;

	if (LEFT == sb_side || BOTTOM == sb_side){
		wirepoint = (track_info->group + 1) % track_info->length;
	} else if (RIGHT == sb_side || TOP == sb_side){
		wirepoint = track_info->group;
	} else {
		W_THROW("Illegal switchblock side: " << sb_side);
	}

	return wirepoint;
}

/* checks whether the specified coordinates are out of bounds */
bool coords_out_of_bounds(INP User_Options user_opts, INP Coordinate check_coord, 
			e_chan_type chan_type){
	bool result = true;

	if (CHANX == chan_type){
		if (check_coord.x < 0 || check_coord.x >= user_opts.x ||	/* there is no x-channel at x-1*/
		    check_coord.y < 0 || check_coord.y >= user_opts.y-1){	/* no channel at y-1 */
			result = true;
		} else {
			result = false;
		}
	} else if (CHANY == chan_type){
		if (check_coord.x < 0 || check_coord.x >= user_opts.x-1 ||
		    check_coord.y < 0 || check_coord.y >= user_opts.y){		/* there is no y-channel at y-1 */
			result = true;
		} else {
			result = false;
		}
	} else {
		W_THROW("Illegal channel type: " << chan_type);
	}
	

	return result;
}

/* Here we find the correct channel type (CHANX or CHANY), and the coordinates to index into it based on the 
   specified tile coordinates and the switchblock side.  */
static e_chan_type index_into_correct_chan(INP Coordinate tile_coord, INP enum e_side side, 
			INOUTP Coordinate *coord_out){
	
	e_chan_type chan_type;

	/* here we use the VPR convention that a tile 'owns' the channels directly to the right
	   and above it */
	switch (side){
		case TOP:
			/* this is y-channel belonging to tile above */
			coord_out->set(tile_coord.x, tile_coord.y+1);
			chan_type = CHANY;
			break;
		case RIGHT:
			/* this is x-channel belonging to tile to the right */
			coord_out->set(tile_coord.x+1, tile_coord.y);
			chan_type = CHANX;
			break;
		case BOTTOM:
			/* this is y-channel on the right of the tile */
			coord_out->set(tile_coord.x, tile_coord.y);
			chan_type = CHANY;
			break;
		case LEFT:
			/* this is x-channel on top of the tile */
			coord_out->set(tile_coord.x, tile_coord.y);
			chan_type = CHANX;
			break;
		default:
			W_THROW("Unknown side specified: " << side);
			break;
	}

	return chan_type;
}

/* get the length for each wire type */
static void get_wire_type_lengths(INP User_Options user_opts, INP t_chan_map *chan_map, 
			INOUTP map<string, int> *track_type_lengths){

	int itrack = 0;
	/* the specific place on the FPGA where we get wire type length info from doesn't really matter */
	Track_Coord track_coord(Coordinate(0,0), CHANX, itrack);
	while(itrack < user_opts.W){
		//cout << "crashing at: " << track_coord << "  type_end: " << chan_map->at(track_coord).type_end << endl;
		string type( chan_map->at(track_coord).type_name_ptr );
		int length = chan_map->at(track_coord).length;
		(*track_type_lengths)[type] = length;

		/* move on to the next wire type */
		itrack = chan_map->at(track_coord).type_end + 1;
		track_coord.track_index = itrack; 
	}
}

/* over all connected wire types (i,j), compute the maximum least common multiple of their track lengths, 
   ie max(LCM(L_i, L_J)) */
static int get_max_lcm( INP vector<t_switchblock_inf> *switchblocks, INP map<string, int> *track_type_lengths){
	int max_lcm = -1;
	int num_sb = (int)switchblocks->size();

	/* over each switchblock */
	for (int isb = 0; isb < num_sb; isb++){
		t_switchblock_inf *sb = &(switchblocks->at(isb));
		int num_wireconns = (int)sb->wireconns.size();
		/* over each wireconn */
		for (int iwc = 0; iwc < num_wireconns; iwc++){
			t_wireconn_inf *wc = &(sb->wireconns.at(iwc));
			int num_from_types = (int)wc->from_type.size();
			int num_to_types = (int)wc->to_type.size();
			/* over each from type */
			for (int ifrom = 0; ifrom < num_from_types; ifrom++){
				/* over each to type */
				for (int ito = 0; ito < num_to_types; ito++){
					int length1 = track_type_lengths->at(wc->from_type.at(ifrom));
					int length2 = track_type_lengths->at(wc->to_type.at(ito));
					int current_lcm = lcm(length1, length2);
					if (current_lcm > max_lcm){
						max_lcm = current_lcm;
					}
				}
			}
		}
	}
	
	return max_lcm;
}




/* clears chan_map then fills it with channel/track information */
void build_channels(INP User_Options user_opts, INP s_arch *arch, INOUTP t_chan_map *chan_map){

	W_PRINT("Building Channels...\n");

	vector<int> type_track_count;		/* how many tracks in channel belong to each wire type */

	/* a factor to account for directionality */
	int dir_fac;

	if (BI_DIRECTIONAL == user_opts.directionality){
		dir_fac = 1;
	} else if (UNI_DIRECTIONAL == user_opts.directionality){
		dir_fac = 2;
	} else {
		W_THROW("Unknown directionality: " << user_opts.directionality);
	}

	/* get the number of tracks per wire type */
	get_type_track_count((user_opts.W /*/ dir_fac*/), arch->num_segments, arch->Segments, &type_track_count);	

	/* over each x coordinate */
	for (int x = 0; x < user_opts.x; x++){
		/* over each y coordinate */
		for (int y = 0; y < user_opts.y; y++){
			Coordinate coord(x, y);
			int itrack = 0;
			int distance = x + y;	/* horizontal+vertical distance between this coordinate and (0,0) */
			
			/* over each wire type */
			for (int itype = 0; itype < arch->num_segments; itype++){
				int track_length = arch->Segments[itype].length;
				int track_type_start = itrack;
				int track_type_end = track_type_start + type_track_count.at(itype) - 1;
	
				/* for each track in the wire type */
				for (int i = 0; i < type_track_count.at(itype); i++){
					/* map keys to index into the channel map */
					Track_Coord track_coord_x(coord, CHANX, itrack);
					Track_Coord track_coord_y(coord, CHANY, itrack);
					
					/* contains info about this specific track segment */
					Track_Info track_info;
					track_info.coord = coord;
					track_info.index = itrack;
					track_info.length = track_length;
					track_info.type_start = track_type_start;
					track_info.type_end = track_type_end;
					track_info.type_name_ptr = arch->Segments[itype].name;


					/* get the direction in which this track is headed */
					if (BI_DIRECTIONAL == user_opts.directionality){
						track_info.dir = BIDIR;
					} else {
						/* unidirectional as verified before */
						/* if even track number, then going in increasing direction */
						if (itrack % 2 == 0){
							track_info.dir = INC;	/* even tracks */
						} else {
							track_info.dir = DEC;	/* odd tracks */
						}
					}
					/* the group this track segment belongs to depends on the index of the track
					   and possibly its coordinates */
					if (user_opts.shift_chan_with_coord){
						//I think this is how VPR's channels are shifted...
						track_info.group = ((track_length - 1 - (int)floor(i/dir_fac) % track_length) + distance) % track_length; 
						//track_info.group = (int)(distance + floor(i/dir_fac)) % track_length;
					} else {
						//track_info.group = track_length - 1 - (int)floor(i/dir_fac) % track_length;
						track_info.group = (int)floor(i/dir_fac) % track_length;
					}

					//W_PRINT("coord: " << coord << "  itrack: " << itrack << "  length: " << track_length << "  track_type_start: " << track_type_start
					//		<< "  track_type_end: " << track_type_end << "  dir: " << track_info.dir << "  group: " << track_info.group << endl);

					/* put the track info into the channel map */
					if (map_key_exists(track_coord_x, chan_map)){
						W_THROW("A channel map entry with the following key already exists:" << endl << track_coord_x);
					} else {
						(*chan_map)[track_coord_x] = track_info;
					}
					
					/* the X and Y cannels are symmetrical so no need to recompute */
					if (map_key_exists(track_coord_y, chan_map)){
						W_THROW("A channel map entry with the following key already exists:" << endl << track_coord_y);
					} else {
						(*chan_map)[track_coord_y] = track_info;
					}
					
					itrack++;
				}
			}
		}
	}
}

/* want to have the same number of tracks per wire type as VPR. This function
   basically copies what VPR does to get the count of track per wire type*/
static void get_type_track_count(INP int num_sets, INP int num_seg_types, INP t_segment_inf *segment_inf,
		INOUTP vector<int> *type_track_count){

	vector<int> demand;
	int i, imax, freq_sum, assigned, size;
	double scale, max, reduce;

	type_track_count->clear();
	type_track_count->reserve(num_seg_types);
	demand.reserve(num_seg_types);

	/* Scale factor so we can divide by any length
	 * and still use integers */
	scale = 1;
	freq_sum = 0;
	for (i = 0; i < num_seg_types; ++i) {
		scale *= segment_inf[i].length;
		freq_sum += segment_inf[i].frequency;
	}
	reduce = scale * freq_sum;

	/* Init assignments to 0 and set the demand values */
	for (i = 0; i < num_seg_types; ++i) {
		type_track_count->push_back(0);
		demand.push_back( scale * num_sets * segment_inf[i].frequency );
	}

	/* Keep assigning tracks until we use them up */
	assigned = 0;
	size = 1;
	imax = 0;
	while (assigned < num_sets) {
		/* Find current maximum demand */
		max = 0;
		for (i = 0; i < num_seg_types; ++i) {
			if (demand.at(i) > max) {
				imax = i;
				max = demand.at(i);
			}
		}

		/* Assign tracks to the type and reduce the types demand */
		demand.at(imax) -= reduce;
		type_track_count->at(imax) += size;
		assigned += size;
	}

	/* Undo last assignment if we were closer to goal without it */
	if ((assigned - num_sets) > (size / 2)) {
		type_track_count->at(imax) -= size;
	}
}




/* builds the routing graph */
void build_graph(INP User_Options user_opts, INP t_sb_connection_map *sb_conns, INP t_chan_map *chan_map,
		INOUTP t_node_map *node_map){

	/* build the nodes */
	for (int x = 0; x < user_opts.x; x++){
		for (int y = 0; y < user_opts.y; y++){
			Coordinate coord(x,y);
			for (int ichan = 0; ichan < 2; ichan++){
				e_chan_type chan_type = (e_chan_type)ichan;
				for (int itrack = 0; itrack < user_opts.W; itrack++){
					add_node(user_opts, coord, chan_type, itrack, chan_map, node_map);
				}
			}
		}
	}

	/* build the edges */
	for (int x = 0; x < user_opts.x; x++){
		for (int y = 0; y < user_opts.y; y++){
			Coordinate coord(x,y);
			for (int fside = 0; fside < 4; fside++){
				e_side from_side = (e_side)fside;
				for (int tside = 0; tside < 4; tside++){
					e_side to_side = (e_side)tside;
					for (int itrack = 0; itrack < user_opts.W; itrack++){
						add_edge(user_opts, coord, from_side, to_side, itrack,
							sb_conns, chan_map, node_map);
					}
				}
			}
		}
	}
}

/* creates edges for, and connects nodes that correspond to the coord/side/track/sb_conns parameters */
static void add_edge(INP User_Options user_opts, INP Coordinate coord, INP e_side from_side, INP e_side to_side,
		INP int from_track, INP t_sb_connection_map *sb_conns, INP t_chan_map *chan_map, INOUTP t_node_map *node_map){
	
	Switchblock_Lookup sb_key(coord.x, coord.y, (int)from_side, (int)to_side, from_track);


	/* do nothing if there is no connection at this coord/side/track combination */
	if ( !map_key_exists(sb_key, sb_conns) ){
		return;
	}
	
	/* Get the tile coordinates and channel type of source/destination channels */ 
	Coordinate from_coord, to_coord;
	e_chan_type from_chan, to_chan;
	from_chan = index_into_correct_chan(coord, from_side, &from_coord);
	to_chan = index_into_correct_chan(coord, to_side, &to_coord);

	/* key to index into the node map */
	Track_Coord from_node_key(from_coord, from_chan, from_track);

	/* do nothing if no corresponding node exists */
	if ( !map_key_exists(from_node_key, node_map) ){
		return;
	}

	Node *from_node = node_map->at(from_node_key);

	/* grab the vector of connections that need to be made */
	vector< t_to_track_inf > *conns = &sb_conns->at(sb_key);
	int num_conns = (int)conns->size();

	int from_conn_point = get_conn_point(from_side, chan_map->at(from_node_key).group);

	/* iterate over all connections we need to make */
	for (int iconn = 0; iconn < num_conns; iconn++){
		int to_track = conns->at(iconn).to_track;
		Track_Coord to_node_key(to_coord, to_chan, to_track);

		/* check the destination node exists */
		if ( !map_key_exists(to_node_key, node_map) ){
			continue;
		}
		Node *to_node = node_map->at(to_node_key);

		/* shouldn't be connecting a node to itself */
		if (from_node == to_node){
			continue;
		}

		/* check that 'from_node' isn't already connected to 'to_node' */
		if (from_node->has_outgoing(to_node)){
			continue;
		}

		int to_conn_point = get_conn_point(to_side, chan_map->at(to_node_key).group);

		/* make the corresponding edge */
		Edge *edge = new Edge(from_node, to_node, from_conn_point, to_conn_point, 0);
		g_num_edges++;
	
		from_node->outgoing.push_back(edge);
		to_node->incoming.push_back(edge);
		//if (BI_DIRECTIONAL == user_opts.directionality){
		//	Edge *reverse_edge = new Edge(to_node, from_node, to_conn_point, from_conn_point, 0);
		//	to_node->outgoing.push_back(reverse_edge);
		//	from_node->incoming.push_back(reverse_edge);
		//}
	}
}


static int get_conn_point(INP e_side side, INP int track_group){
	int conn_point = -1;

	conn_point = (track_group) * 2;
	//adjust for left/bottom side
	if (LEFT == side || BOTTOM == side){
		conn_point++;
	}

	return conn_point;
}

/* conditionally adds a node based on passed-in parameters to the node map. node is only added
   if the track segment to which it corresponds is in group 0. i.e., one node represents
   an entire length of track */
static void add_node(INP User_Options user_opts, INP Coordinate coord, e_chan_type chan_type, INP int track, INP t_chan_map *chan_map, 
		INOUTP t_node_map *node_map){

	Track_Coord track_coord(coord, chan_type, track);

	/* make sure channel map has a corresponding entry */
	if ( !map_key_exists(track_coord, chan_map) ){
		W_THROW("Channel map does not have entry with key: " << track_coord);
	}

	Track_Info track_info = chan_map->at(track_coord);

	/* a node represents an entire length of a track. add node if current track segment belongs to group 0 or 
	   if the track is along the bottom or left edge of the test area where the track might not have started on 
	   group 0 */
	if (0 == track_info.group || (0 == coord.x && CHANX == chan_type) || (0 == coord.y && CHANY == chan_type)){

		/* make sure we're not out of bounds */
		if (coords_out_of_bounds(user_opts, track_coord.coord, chan_type)){
			return;
		}
		
		/* create a new node */
		Node *node = new Node;
		//node->start_segment = &chan_map->at(track_coord);
		node->track_chan_type = track_coord.chan_type;
		node->track_index = track_coord.track_index;
		node->track_length = chan_map->at(track_coord).length;
		node->track_dir = chan_map->at(track_coord).dir;
		node->track_start = coord;
		node->track_start_group = track_info.group;

		/* want all the other segments of the track to point to this one node */
		for (int iseg = 0; iseg < track_info.length - track_info.group; iseg++){

			/* make sure we're not out of bounds */
			if (coords_out_of_bounds(user_opts, track_coord.coord, chan_type)){
				break;
			}

			/* make sure a node map entry corresponding to this track segments doesn't exist yet */
			if (map_key_exists(track_coord, node_map)){
				W_THROW("Node map already has entry with key: " << track_coord);
			}

			/* add node map entry */
			(*node_map)[track_coord] = node;

			/* move on to the next segment of the track */
			if (CHANX == chan_type){
				track_coord.coord.x++;
			} else {
				track_coord.coord.y++;
			}
		}
	} else {
		/* not in group 0. all of the track's segments should already point to the correct node */
	}
}

