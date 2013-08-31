#ifndef BUILD_SWITCHBLOCKS_H
#define BUILD_SWITCHBLOCKS_H

#include <map>
#include <vector>
#include "util.h"
#include "vpr_types.h"

/**** Classes ****/
class Switchblock_Lookup{
public:
	int x_coord;
	int y_coord;
	int from_side;
	int to_side;
	int track_num;

	/* Empty constructor initializes everything to 0 */
	Switchblock_Lookup(){
		x_coord = y_coord = from_side = to_side = track_num = 0;
	}

	/* Constructor for initializing member variables */
	Switchblock_Lookup(int set_x, int set_y, int set_from, int set_to, int set_track){
		x_coord = set_x;
		y_coord = set_y;
		from_side = set_from;
		to_side = set_to;
		track_num = set_track;
	}

	/* Function for setting the segment coordinates */
	void set_coords(int set_x, int set_y, int set_from, int set_to, int set_track){
		x_coord = set_x;
		y_coord = set_y;
		from_side = set_from;
		to_side = set_to;
		track_num = set_track;
	}

	/* Overload < operator which is used by std::map */
	bool operator < (const Switchblock_Lookup &obj) const {
		bool result;

		/* This is ugly, but it implements a hierarchy of inequality. For instance, if x1 < x2 then
		   obj1 < obj2. But if x1==x2 then we have to check y1 and y2, and so on */
		if (x_coord < obj.x_coord){
			result = true;
		} else {
			if (x_coord == obj.x_coord){
				if (y_coord < obj.y_coord){
					result = true;
				} else {
					if (y_coord == obj.y_coord){
						if (from_side < obj.from_side){
							result = true;
						} else {
							if (from_side == obj.from_side){
								if (to_side < obj.to_side){
									result = true;
								} else {
									if (to_side == obj.to_side){
										if (track_num < obj.track_num){
											result = true;
										} else {
											result = false;
										}
									} else {
										result = false;
									}
								}
							} else {
								result = false;
							}
						}
					} else {
						result = false;
					}
				}
			} else {
				result = false;
			}
		}
		
		return result;
	}
}; /* class Switchblock_Lookup */


/**** Typedefs ****/
/* Switchblock connections are made as [0..nx][0..ny][from_side][to_side][from_track (0..W-1)].
   We use the Switchblock_Lookup class to specify these five dimensions.
   Furthermore, a source_track at a given 5-d coordinate *may* connect to multiple destination tracks. We use a 
   vector to represent this possibility.
   A matrix specifying connections for all switchblocks in an FPGA would be very large, and probably sparse,
   so we use a map to take advantage of the sparsity. */
typedef std::map< Switchblock_Lookup, std::vector<int> > t_sb_connection_map;


/**** Function Declarations ****/
t_sb_connection_map * alloc_and_load_switchblock_permutations( INP t_chan_details * chan_details_x, 
				INP t_chan_details * chan_details_y, INP int nx, INP int ny, 
				INP std::vector<t_switchblock_inf> switchblocks, 
				INP int nodes_per_chan, INP enum e_directionality directionality);

void free_switchblock_permutations(INOUTP t_sb_connection_map *sb_conns);

#endif
