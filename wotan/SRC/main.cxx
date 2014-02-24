#include <iostream>
#include <string>
#include "io.h"
#include "exception.h"
#include "wotan_init.h"
#include "globals.h"
#include "../../libarchfpga/include/util.h"

#include "graphics.h"
#include "draw.h"
#include "wotan_types.h"
#include "build.h"
#include "route.h"

using namespace std;


int main(int argc, char **argv){
	try{
		wotan_init(argc, argv);

		start_easygl_graphics(g_user_opts);

		t_chan_map *chan_map = new t_chan_map;
		build_channels(g_user_opts, &g_arch, chan_map);
		cout << "chan map size: " << chan_map->size() << endl;

		t_sb_connection_map *sb_conns = new t_sb_connection_map;
		build_sb_permutation_map(chan_map, g_user_opts, &g_arch.switchblocks, sb_conns);		
		cout << "sb conn size: " << sb_conns->size() << endl;

		build_graph(g_user_opts, sb_conns, chan_map, &g_node_map);

		/* doing this doesn't necessarily release memory back to the OS... it does free it up 
		   for further use by this process though */
		chan_map->clear();
		sb_conns->clear();
		delete chan_map;
		delete sb_conns;

		cout << "Doing flood fill... ";
		t_flood_results *flood_results = new t_flood_results;
		perform_and_analyze_flood_fill(g_user_opts, &g_node_map, flood_results);
		cout << "Done." << endl;

		set_flood_results_ptr( flood_results );

		cout << "num edges: " << g_num_edges << endl;

		event_loop(act_on_button_press, NULL, NULL, drawscreen);
		delete flood_results;

		/* 
		If we specify a given track_coord, we should be able to look up the node
		Can we draw entire FPGA based on nodes and edges? What has to be done?
		
		Sounds like it's much easier to use track map sb map in conjunction with nodes 

		But we also have to factor in how the routing will be performed. Recall that maze router
		marks tracks as it routes and at the end we have to figure out which path it is that we took.
		We also want to be able to arrive at destination over multiple paths...
		
		*/

	} catch (Wotan_Exception &ex){
		W_PRINT(ex);
	}


	/*
	run init function - goes into wotan_api.c
	- print welcome message
	- run init function
		- read in architecture name and options
		- read in architecture!
		- run results through a checker to see if what we have is valid
	*/

	return 0;
}
