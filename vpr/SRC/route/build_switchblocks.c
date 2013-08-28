/*
DESCRIPTION
The functions in this file are used to help build the switchblocks in rr_graph.c:build_rr_graph
(partially) based on the information in physical_types.h:t_switchblock_inf parsed from the switchblock section
of the XML architecture file.
*/


#include "build_switchblocks.h"
#include "physical_types.h"
#include "parse_switchblocks.h"

using namespace std;


/**** Function Declarations ****/



/**** Function Definitions ****/
t_sb_permutation_map * alloc_and_load_switchblock_permutations( INP t_chan_details * chan_details_x, 
				INP t_chan_details * chan_details_y, INP int nx, INP int ny, 
				INP int num_switchblocks, INP t_switchblock_inf *switchblocks, 
				INP int nodes_per_chan, INP enum e_directionality directionality){

	Switchblock_Lookup coord;

	/* allocate the switchblock permutations pointer */
	t_sb_permutation_map *sb_conns = new t_sb_permutation_map;

	/* iterate over all the switchblocks specified in the architecture */
	for (int i_sb; i_sb < num_switchblocks; i_sb++){
		t_switchblock_inf sb = switchblocks[i_sb];

		/* verify that switchblock type matches specifies directionality -- currently we have to stay consistent */
		if (directionality != sb.directionality){
			vpr_printf(TIO_MESSAGE_ERROR, "alloc_and_load_switchblock_permutations: Switchblock %s does not match directionality of architecture\n", sb.name);
			exit(1);
		}
		
		//iterate over the x coordinate
			//iterate over the y coordinate
				//for each of the 12 permutation functions
					//check if it has entries
					//iterate over all wireconns
						//confirm wires with specified names exist
						//find specified source and target tracks
						//connect them according to the specific permutation function

		/* Iterate over the x,y coordinates spanning the FPGA. Currently, the FPGA size is set as
		   (nx+1) by (ny+1), so we iterate over 0..nx and 0..ny. */
		for (int x_coord = 0; x_coord <= nx; x_coord++){
			for (int y_coord = 0; y_coord <= ny; y_coord++){
				/* Iterate over each track in channel */
				for (int itrack = 0; itrack < nodes_per_chan; itrack++){

					/* now we manually interate over the switchblock permutation functions */
					/* permutation functions from the top */
					//TODO: convert to using map for permutation funcs, then update this
					//if ( !sb.tl.empty() ){
					//	/* Fill appropriate entry of the sb_conns map with vector specifying the tracks 
					//	   the current track will connect to */
					//	//get_permutation_conns(x_coord, y_coord, TOP, LEFT, itrack,
					//	//		chan_details_x, chan_details_y, sb.tl, sb_conns);
					//} if ( !sb.tb.empty() ){

					//} if ( !sb.tr.empty() ){

					//}

					///* permutation functions from the right */
					//if ( !sb.rt.empty() ){

					//} if ( !sb.rl.empty() ){

					//} if ( !sb.rb.empty() ){

					//}

					///* permutation functions from the bottom */
					//if ( !sb.bl.empty() ){

					//} if ( !sb.bt.empty() ){

					//} if ( !sb.br.empty() ){

					//}

					///* permutation functions from the left */
					//if ( !sb.lt.empty() ){

					//} if ( !sb.lr.empty() ){

					//} if ( !sb.lb.empty() ){

					//}
				}
			}
		}
	
		//can simplify iteration over the 12 permutation functions by working with segment increments w.r.t.
		//the current coordinates. then for each of the 12 cases we simply have to specify the channel (x/y) and 
		//segment increment, and pass that on, along with a pointer to the permutation function vector, to a 
		//single generic function
	}

	return sb_conns;
}

/* calls delete on the switchblock permutations pointer and sets it to 0 */
void free_switchblock_permutations(INOUTP t_sb_permutation_map *sb_conns){
	delete sb_conns;
	sb_conns = NULL;
	return;
}
