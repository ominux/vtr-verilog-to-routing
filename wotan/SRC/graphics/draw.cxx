
#include <cmath>
#include <sstream>
#include "io.h"
#include "exception.h"
#include "draw.h"
#include "graphics.h"
#include "globals.h"
#include "build.h"

using namespace std;




/**** File-scope Variables ****/
static float g_tile_width = 0;
static float g_channel_fraction = 0;
static float g_track_spacing = 0;

static t_flood_results *g_flood_results_ptr = NULL;


static bool g_draw_routing = false;
static bool g_draw_flood = true;

static float g_flood_scale = 1;

static int g_unreachable = 0;


/**** Function Declarations ****/

/* toggles display mode to show the flood results */
static void button_show_flood( void (*drawscreen) (void));
/* toggles display mode to show the routing elements */
static void button_show_rr( void (*drawscreen) (void));
/* toggles display mode to show the flood results */
static void button_flood_scale_plus( void (*drawscreen) (void));
/* toggles display mode to show the routing elements */
static void button_flood_scale_minus( void (*drawscreen) (void));

/* draws the results of the flooding algorithm to see which tracks are reachable */
static void draw_flood_results(INP User_Options user_opts, INP t_flood_results *flood_results);

/* draw the FPGA channels based on nodes */
static void draw_channels(INP t_node_map *node_map);
/* draw the FPGA switches based on edge connections between nodes */
static void draw_switches(INP t_node_map *node_map);
/* returns the top left point of the tile at the FPGA coordinates specified by 'tile_coord' */
static void get_bottom_left_of_tile(INP Coordinate tile_coord, INP float tile_width, INOUTP float *x, INOUTP float *y);
/* returns the size of a tile side (tile is square) */
static float get_tile_width( INP int num_tiles, INP float window_width );
/* what fraction of the tile should be reserved for the channel? */
static float get_channel_fraction(INP int num_tracks);
/* gets the draw spacing between adjacent tracks in a channel */
static float get_track_spacing(INP int num_tracks, INP float tile_width, INP float channel_fraction);
/* gets offset from top/right of tile for drawing track at specified index */
static float get_offset_of_track(INP int track_index, INP float track_spacing);

/**** Function Definitions ****/
/* initializes and starts easygl graphics */
void start_easygl_graphics(INP User_Options user_opts){
	init_graphics(WINDOW_NAME, 0);
	init_world(UPPER_LEFT_X, UPPER_LEFT_Y, LOWER_RIGHT_X, LOWER_RIGHT_Y);

	/* initialize some file-scope variables */
	g_tile_width = get_tile_width( user_opts.x, WINDOW_WIDTH );
	g_channel_fraction = get_channel_fraction( user_opts.W );
	g_track_spacing = get_track_spacing( user_opts.W, g_tile_width, g_channel_fraction );

	create_button("Window", "- Flood Scale", button_flood_scale_minus);
	create_button("Window", "+ Flood Scale", button_flood_scale_plus);
	create_button("Window", "Show Flood", button_show_flood);
	create_button("Window", "Show RR", button_show_rr);
}

/* the function which determines what happens if a user clicks a button somewhere in the drawing area */
void act_on_button_press(INP float x, INP float y, INP t_event_buttonPressed button_info){

}

/* toggles display mode to show the flood results */
static void button_show_flood( void (*drawscreen) (void)){
	g_draw_routing = false;
	g_draw_flood = true;
	drawscreen();
}
/* toggles display mode to show the routing elements */
static void button_show_rr( void (*drawscreen) (void)){
	g_draw_routing = true;
	g_draw_flood = false;
	drawscreen();
}
/* toggles display mode to show the flood results */
static void button_flood_scale_plus( void (*drawscreen) (void)){
	g_flood_scale = min(100.0, g_flood_scale*1.3);
	drawscreen();
}
/* toggles display mode to show the routing elements */
static void button_flood_scale_minus( void (*drawscreen) (void)){
	g_flood_scale = max(1.0, g_flood_scale/1.3);
	drawscreen();
}

/* the main drawscreen function which we will pass to event_loop */
void drawscreen(void){
	clearscreen();
	
	if (g_draw_routing){
		setcolor(BLACK);
		draw_channels(&g_node_map);
		setcolor(GREEN);
		draw_switches(&g_node_map);
	}

	if (g_draw_flood){
		draw_flood_results(g_user_opts, g_flood_results_ptr);
	}

	stringstream msg;
	msg << g_num_edges << " edges.   Flood Scale: x" << g_flood_scale << ".   Unreachable:" << g_unreachable;
	update_message(msg.str().c_str());
}

/* sets the filesoce flood results pointer */
void set_flood_results_ptr( INP t_flood_results *flood_results){
	g_flood_results_ptr = flood_results;
}

/* draws the results of the flooding algorithm to see which tracks are reachable */
static void draw_flood_results(INP User_Options user_opts, INP t_flood_results *flood_results){
	g_unreachable = 0;
	for (int x = 0; x < user_opts.x; x++){
		for (int y = 0; y < user_opts.y; y++){
			for (int ichan = 0; ichan < 2; ichan++){
				e_chan_type chan_type = (e_chan_type)ichan;
				/* key to index into the flood results map */
				Chan_Coord chan_coord(Coordinate(x,y), chan_type);

				if (coords_out_of_bounds(user_opts, chan_coord.coord, chan_type)){
					continue;
				} 

				float x1, y1, x2, y2;

				get_bottom_left_of_tile(chan_coord.coord, g_tile_width, &x1, &y1);
				x2 = x1 + g_tile_width;
				y2 = y1 + g_tile_width;
				if (CHANX == chan_type){
					y1 += g_tile_width * (1.0 - g_channel_fraction);
				} else {
					x1 += g_tile_width * (1.0 - g_channel_fraction);
				}

				/* set color based on number of reachable tracks at this coordinate/channel */
				int reachable_tracks;
				if ( map_key_exists(chan_coord, flood_results) ){
 					reachable_tracks = flood_results->at(chan_coord);
				} else {
					reachable_tracks = 0;
				}
				float fraction_reachable = (float)reachable_tracks / user_opts.W;
				if (fraction_reachable == 0.0){
					g_unreachable++;
				}
				if (fraction_reachable < 0.02 / g_flood_scale){
					setcolor(BLACK);
				} else if (fraction_reachable < 0.1 / g_flood_scale){
					setcolor(DARKGREY);
				} else if (fraction_reachable < 0.2 / g_flood_scale){
					setcolor(RED);
				} else if (fraction_reachable < 0.4 / g_flood_scale){
					setcolor(YELLOW);
				} else if (fraction_reachable < 0.8 / g_flood_scale){
					setcolor(DARKGREEN);
				} else if (fraction_reachable <= 1.0){
					setcolor(GREEN);
				}
				
				////TODO: temp
				//if (chan_coord.coord == Coordinate(user_opts.x/2, user_opts.y/2) && chan_coord.chan_type == CHANX){
				//	setcolor(WHITE);
				//}

				fillrect(x1, y1, x2, y2);
			}
		}
	}
}

/* draw the FPGA switches based on edge connections between nodes */
static void draw_switches(INP t_node_map *node_map){
	/* iterate over all the nodes */
	t_node_map::const_iterator it;
	for (it = node_map->begin(); it != node_map->end(); it++){
		Node *from_node = it->second;
		Track_Coord track_coord = it->first;
		
		//TODO: code != operator for Coordinate class
		if ( !(track_coord.coord == from_node->track_start) ){
			continue;
		}
		
		float from_x, from_y, to_x, to_y;

		/* walk through the outgoing edges that this node has */
		vector< Edge* >::const_iterator itv;
		for (itv = from_node->outgoing.begin(); itv != from_node->outgoing.end(); itv++){
			Edge *edge = (*itv);
			Node *to_node = edge->to;
			int from_conn_point = edge->from_conn_point;
			int to_conn_point = edge->to_conn_point;

			//TODO: stuff below should really go into a function...
			/* find location at which outgoing connection is made */
			get_bottom_left_of_tile(from_node->track_start, g_tile_width, &from_x, &from_y);
			if (CHANX == from_node->track_chan_type){
				from_y += g_tile_width * (1.0 - g_channel_fraction);
				from_y += get_offset_of_track(from_node->track_index, g_track_spacing);
				
				from_x += (float)((int)floor(from_conn_point / 2) - from_node->track_start_group) * g_tile_width;
				if ((from_conn_point & 1) == 1){
					from_x += g_tile_width * (1.0 - g_channel_fraction);
				}
			} else {
				from_x += g_tile_width * (1.0 - g_channel_fraction);
				from_x += get_offset_of_track(from_node->track_index, g_track_spacing);
				
				from_y += (float)((int)floor(from_conn_point / 2) - from_node->track_start_group) * g_tile_width;
				if ((from_conn_point & 1) == 1){
					from_y += g_tile_width * (1.0 - g_channel_fraction);
				}
			}

			/* find location at which incoming connection is made */
			get_bottom_left_of_tile(to_node->track_start, g_tile_width, &to_x, &to_y);
			if (CHANX == to_node->track_chan_type){
				to_y += g_tile_width * (1.0 - g_channel_fraction);
				to_y += get_offset_of_track(to_node->track_index, g_track_spacing);
				
				to_x += (float)((int)floor(to_conn_point / 2) - to_node->track_start_group) * g_tile_width;
				if ((to_conn_point & 1) == 1){
					to_x += g_tile_width * (1.0 - g_channel_fraction);
				}
			} else {
				to_x += g_tile_width * (1.0 - g_channel_fraction);
				to_x += get_offset_of_track(to_node->track_index, g_track_spacing);
				
				to_y += (float)((int)floor(to_conn_point / 2) - to_node->track_start_group) * g_tile_width;
				if ((to_conn_point & 1) == 1){
					to_y += g_tile_width * (1.0 - g_channel_fraction);
				}
			}

			//cout << "from coord: " << track_coord.coord << "  to_coord: " << to_node->track_start << endl;
			//cout << " from_chan: " << track_coord.chan_type << "  to_chan: " << to_node->track_chan_type << endl;
			//cout << "  from conn: " << from_conn_point << "  to conn: " << to_conn_point << endl;
			//cout << "  to_start_group: " << to_node->track_start_group << endl;
			//cout << "    from_x: " << from_x << "  from_y: " << from_y << "  to_x: " << to_x << "  to_y: " << to_y << endl;

			drawline(from_x, from_y, to_x, to_y);
		}
	}
}

/* draw the FPGA channels based on nodes */
static void draw_channels(INP t_node_map *node_map){
	/* iterate over all the nodes */
	t_node_map::const_iterator it;
	for (it = node_map->begin(); it != node_map->end(); it++){
		Node *node = it->second;
		Track_Coord track_coord = it->first;
		
		/* should not draw tracks that are outside the test area */
		if (coords_out_of_bounds(g_user_opts, track_coord.coord, track_coord.chan_type)){
			continue;
		}

		/* get the distance of this track segment from track's starting point */
		int dist_from_start = 0;
		if (CHANX == track_coord.chan_type){
			dist_from_start = track_coord.coord.x - node->track_start.x;
			if (track_coord.coord.y != node->track_start.y){
				W_THROW("horizontal track starts and ends at different y coordinates!");
			}
		} else {
			dist_from_start = track_coord.coord.y - node->track_start.y;
			if (track_coord.coord.x != node->track_start.x){
				W_THROW("vertical track starts and ends at different x coordinates!");
			}
		}

		//if (CHANY == track_coord.chan_type){
		//	cout << "At: " << track_coord.coord << "  start: " << node->track_start << "  dist: " << dist_from_start << "  channel_frac: " << g_channel_fraction << endl;
		//}
	
		//if (node->mark == -1){
		//	setcolor(RED);
		//} else {
		//	setcolor(BLACK);
		//}

		/* check if track passes through switchblock */
		bool pass_through = true;
		if (node->track_length - node->track_start_group - 1 == dist_from_start){
			/* this is the last segment of the track */	
			pass_through = false;
		}

		/* now draw the track... */
		float start_x, start_y, end_x, end_y;
		get_bottom_left_of_tile(track_coord.coord, g_tile_width, &start_x, &start_y);
		if (CHANX == track_coord.chan_type){
			start_y += g_tile_width * (1.0 - g_channel_fraction);
			start_y += get_offset_of_track(track_coord.track_index, g_track_spacing);
			end_y = start_y;
			if (pass_through){
				end_x = start_x + g_tile_width;
			} else {
				//TODO: we never get here...
				end_x = start_x + (1.0 - g_channel_fraction) * g_tile_width;
			}
		} else {
			start_x += g_tile_width * (1.0 - g_channel_fraction);
			start_x += get_offset_of_track(track_coord.track_index, g_track_spacing);
			end_x = start_x;
			if (pass_through){
				end_y = start_y + g_tile_width;
			} else {
				end_y = start_y + (1.0 - g_channel_fraction) * g_tile_width;
			}
		}
		drawline(start_x, start_y, end_x, end_y);
	}
}

/* returns the top left point of the tile at the FPGA coordinates specified by 'tile_coord' */
static void get_bottom_left_of_tile(INP Coordinate tile_coord, INP float tile_width, INOUTP float *x, INOUTP float *y){
	(*x) = (float)tile_coord.x * tile_width;
	(*y) = (float)tile_coord.y * tile_width;
}
/* returns the size of a tile side (tile is square) */
static float get_tile_width( INP int num_tiles, INP float window_width ){
	float tile_width = window_width / (float)num_tiles;
	return tile_width;
}
/* what fraction of the tile should be reserved for drawing the channel? */
static float get_channel_fraction(INP int num_tracks){
	float channel_fraction = 0;

	/* when num_tracks is 200, channel will take up 80% of the tile width.
	   any higher number of tracks will still saturate at 80% */
	channel_fraction = (float)num_tracks / 200.0;
	channel_fraction = min(channel_fraction, (float)0.8);
	channel_fraction = max(channel_fraction, (float)0.2);

	return channel_fraction;
}
/* gets the draw spacing between adjacent tracks in a channel */
static float get_track_spacing(INP int num_tracks, INP float tile_width, INP float channel_fraction){
	float track_spacing = tile_width * channel_fraction / (float)(num_tracks+1);
	return track_spacing;
}
/* gets offset from top/right of tile for drawing track at specified index */
static float get_offset_of_track(INP int track_index, INP float track_spacing){
	float offset = (float)(track_index + 1) * track_spacing;
	return offset;
}
