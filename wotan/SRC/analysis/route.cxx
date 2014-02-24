
#include <map>
#include <vector>
#include <queue>
#include "route.h"
#include "wotan_types.h"
#include "wotan_util.h"
#include "io.h"
#include "exception.h"
#include "wotan_init.h"
#include "build.h"

using namespace std;

/**** Function Declarations ****/
/* performs a flood fill of the graph nodes - basically a maze router that has no specific destination */
static void flood_fill(INP Track_Coord start_node_key, INP t_node_map *node_map);
/* analyze. TODO: comment */
static void analyze_flood_fill( INP Track_Coord start_node_key, INP User_Options user_opts, 
		INP t_node_map *node_map, INOUTP t_flood_results *flood_results );
/* resets the marks of all graph nodes to -1 */
static void clear_node_marks(INP t_node_map *node_map);
//TODO: comment
static bool node_mark_too_high(INP Node *node, INP Track_Coord node_track_coord, INP Track_Coord compare_track_coord);



/**** Function Definitions ****/

/* starts at some node and uses a maze router to 'flood' the routing area */
void perform_and_analyze_flood_fill(INP User_Options user_opts, INP t_node_map *node_map, 
		INOUTP t_flood_results *flood_results){

	Track_Coord start_node_key(Coordinate(user_opts.x/2, user_opts.y/2), CHANX, 0);
	
	flood_results->clear();

	/* reset node marks used for routing */
	clear_node_marks( node_map );

	/* do the flood fill */
	flood_fill(start_node_key, node_map);

	/* analyze flood fill results */
	analyze_flood_fill( start_node_key, user_opts, node_map, flood_results );
}


/* performs a flood fill of the graph nodes - basically a maze router that has no specific destination */
static void flood_fill(INP Track_Coord start_node_key, INP t_node_map *node_map){
	queue< Node* > flood_queue;

	/* push the starting node onto the queue */
	Node *start_node = node_map->at(start_node_key);
	start_node->mark = 0;
	start_node->track_usage = start_node->track_length;
	flood_queue.push(start_node);
	

	/* main loop */
	while ( !flood_queue.empty() ){

		/* get front node */
		Node *f_node = flood_queue.front();
		int f_node_mark = f_node->mark;
		int track_usage = f_node->track_usage;
		if (f_node_mark < 0){
			W_THROW("Got node on flood queue with illegal mark: " << f_node_mark);
		}

		/* push the nodes the front node connects to onto the queue */
		for ( int iedge = 0; iedge < (int)f_node->outgoing.size(); iedge++ ){
			Edge *edge = f_node->outgoing.at(iedge);
			Node *to_node = edge->to;
			/* mark these nodes and push onto queue if they are not marked yet */
			if (to_node->mark < 0){
				to_node->mark = f_node_mark + 1;
				to_node->track_usage = track_usage + to_node->track_length;
				flood_queue.push( to_node );
			}
		}

		/* pop the front node off the queue */
		flood_queue.pop();
	}

}

/* analyze. TODO: comment */
static void analyze_flood_fill( INP Track_Coord start_node_key, INP User_Options user_opts, 
		INP t_node_map *node_map, INOUTP t_flood_results *flood_results ){
	
	//Node *starting_node = node_map->at(start_node_key);


	for (int x = 0; x < user_opts.x; x++){
		for (int y = 0; y < user_opts.y; y++){
			for (int ichan = 0; ichan < 2; ichan++){
				e_chan_type chan_type = (e_chan_type)ichan;
				/* key to index into the flood fill results map */
				Chan_Coord flood_map_key(Coordinate(x,y), chan_type);
				for (int track = 0; track < user_opts.W; track++){
					/* key to index into the node map */
					Track_Coord node_key(Coordinate(x,y), chan_type, track);
					
					/* make sure we aren't out of bounds here */
					if ( coords_out_of_bounds(user_opts, node_key.coord, chan_type) ){
						continue;
					}

					if ( !map_key_exists(node_key, node_map) ){
						W_THROW("No node corresponds to node key " << node_key);
					}

					Node *node = node_map->at(node_key);
					int node_mark = node->mark;

					/* skip if this node hasn't been marked */
					if (node_mark < 0){
						continue;
					}

					/* skip this node if it's mark doesn't meet certain criteria */
					if ( node_mark_too_high(node, node_key, start_node_key) ){
						node->mark = -1;
						continue;
					}

					/* check if the appropriate entry in the results map exists yet. if not, create it */
					if ( !map_key_exists(flood_map_key, flood_results) ){
						/* so far we reached one track in this channel */
						(*flood_results)[flood_map_key] = 1;
					} else {
						/* increment the number of tracks we've reached this channel at */
						flood_results->at(flood_map_key)++;
					}
				}
			}
		}
	}
}

//TODO: comment
static bool node_mark_too_high(INP Node *node, INP Track_Coord node_track_coord, INP Track_Coord compare_track_coord){
	bool too_high = true;

	int dx_plus_dy = node_track_coord.coord.get_dx_plus_dy( compare_track_coord.coord );
	
	if (node->track_usage <= dx_plus_dy*1.3 + 8){
		too_high = false;
	} else {
		too_high = true;
	}

	return too_high;
}

/* resets the marks of all graph nodes to -1 */
static void clear_node_marks(INP t_node_map *node_map){
	t_node_map::const_iterator it;
	for (it = node_map->begin(); it != node_map->end(); it++){
		Node *node = it->second;
		node->mark = -1;
	}
}


/*
What do we want to do with route here?
- We are analyzing the performance of the switchblock
- How many tracks do we hit in each channel?

- so we start on some node, then essentially flood fill until queue is empty

- then we go through and look at the 'number' on each node we managed to hit and either count it or disqualify it based on that.

- do we need other variables to keep track of shit?
	- a final map of how many tracks in each channel were hit
	- a 'mark' field in each node

*/
