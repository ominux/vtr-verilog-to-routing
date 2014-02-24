#ifndef BUIDL_H
#define BUIDL_H

#include <vector>
#include "wotan_types.h"
#include "globals.h"
#include "parse_switchblocks.h"



/**** Function Declarations ****/
/* clears chan_map then fills it with channel/track information */
void build_channels(INP User_Options user_opts, INP s_arch *arch, INOUTP t_chan_map *chan_map);
/* build the switchblock connections map based on switchblock permutations and the channel map */
void build_sb_permutation_map( INP t_chan_map *chan_map, INP User_Options user_opts, 
		INP std::vector<t_switchblock_inf> *switchblocks, INOUTP t_sb_connection_map *sb_conns );
/* builds the routing graph */
void build_graph(INP User_Options user_opts, INP t_sb_connection_map *sb_conns, INP t_chan_map *chan_map,
		INOUTP t_node_map *node_map);
/* checks whether the specified coordinates are out of bounds */
bool coords_out_of_bounds(INP User_Options user_opts, INP Coordinate check_coord, 
			e_chan_type chan_type);


#endif
