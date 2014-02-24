#ifndef BUILD_SWITCHBLOCKS_H
#define BUILD_SWITCHBLOCKS_H

#include <map>
#include <vector>
#include "util.h"
#include "vpr_types.h"
#include "parse_switchblocks.h"


/**** Function Declarations ****/
t_sb_connection_map * alloc_and_load_switchblock_permutations( INP t_chan_details * chan_details_x, 
				INP t_chan_details * chan_details_y, INP int nx, INP int ny, 
				INP std::vector<t_switchblock_inf> switchblocks, 
				INP int nodes_per_chan, INP enum e_directionality directionality);

void free_switchblock_permutations(INOUTP t_sb_connection_map *sb_conns);

#endif
