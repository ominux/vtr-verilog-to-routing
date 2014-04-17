#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "vpack.h"
#include "globals.h"
#include "output_clustering.h"

#define LINELENGTH 80


/****************** Subroutines local to this module ************************/


/**************** Subroutine definitions ************************************/

static void print_inpad(FILE *fpout, int iblock, int icluster) {
	fprintf(fpout,
		"\t<block name=\"%s\" instance=\"io[%d]\" mode=\"inpad\">\n"
		"\t\t<inputs>\n"
		"\t\t\t<port name=\"outpad\">open </port>\n"
		"\t\t</inputs>\n"
		"\t\t<outputs>\n"
		"\t\t\t<port name=\"inpad\">inpad[0].inpad[0]->inpad  </port>\n"
		"\t\t</outputs>\n"
		"\t\t<clocks>\n"
		"\t\t\t<port name=\"clock\">open </port>\n"
		"\t\t</clocks>\n"
		"\t\t<block name=\"%s\" instance=\"inpad[0]\">\n"
		"\t\t\t<inputs>\n"
		"\t\t\t</inputs>\n"
		"\t\t\t<outputs>\n"
		"\t\t\t\t<port name=\"inpad\">%s </port>\n"
		"\t\t\t</outputs>\n"
		"\t\t\t<clocks>\n"
		"\t\t\t</clocks>\n"
		"\t\t</block>\n"
		"\t</block>\n\n", block[iblock].name, icluster, block[iblock].name, block[iblock].name);
}

static void print_outpad(FILE *fpout, int iblock, int icluster) {
	
	fprintf(fpout,
		"\t<block name=\"%s\" instance=\"io[%d]\" mode=\"outpad\">\n"
		"\t\t<inputs>\n"
		"\t\t<port name=\"outpad\">%s </port>\n"
		"\t\t</inputs>\n"
		"\t\t<outputs>\n"
		"\t\t<port name=\"inpad\">open </port>\n"
		"\t\t</outputs>\n"
		"\t\t<clocks>\n"
		"\t\t<port name=\"clock\">open </port>\n"
		"\t\t</clocks>\n"
		"\t\t<block name=\"%s\" instance=\"outpad[0]\">\n"
		"\t\t\t<inputs>\n"
		"\t\t\t<port name=\"outpad\">io.outpad[0]->outpad  </port>\n"
		"\t\t\t</inputs>\n"
		"\t\t\t<outputs>\n"
		"\t\t\t</outputs>\n"
		"\t\t\t<clocks>\n"
		"\t\t\t</clocks>\n"
		"\t\t</block>\n"
		"\t</block>\n\n", block[iblock].name, icluster, net[block[iblock].nets[0]].name, block[iblock].name);
}


static void print_string (char *str_ptr, int *column, FILE *fpout) {

/* Prints string without making any lines longer than LINELENGTH.  Column  *
 * points to the column in which the next character will go (both used and *
 * updated), and fpout points to the output file.                          */

 int len;
 fprintf(fpout,"%s ",str_ptr);
 *column += len + 1;
}


static void print_net_name (int inet, int *column, FILE *fpout) {

/* This routine prints out the net name (or open) and limits the    *
 * length of a line to LINELENGTH characters by using \ to continue *
 * lines.  net_num is the index of the net to be printed, while     *
 * column points to the current printing column (column is both     *
 * used and updated by this routine).  fpout is the output file     *
 * pointer.                                                         */
 
 char *str_ptr;
 
 if (inet == OPEN)
    str_ptr = "open";
 else
    str_ptr = net[inet].name;
 
 print_string (str_ptr, column, fpout);
}


static void print_net_number (int inet, int *column, FILE *fpout) {

/* Prints out either a number or "open".                     */

 char str[BUFSIZE];

 if (inet == OPEN) 
    sprintf (str, "open");
 else
    sprintf (str, "%d", inet);

 print_string (str, column, fpout);
}


static void print_pins (FILE *fpout, int bnum, int lut_size) {

/* This routine is used only for netlists that have NOT been clustered.   *
 * It prints out the pinlist for a clb that contains either a LUT, a      *
 * LATCH or both, assuming that each LUT input needs its own pin, and     *
 * the output cannot be fed back to inputs via local routing.  It prints  *
 * the LUT inputs and output in the order below so that the net ordering  *
 * when parsed will be the same as that of the old combinational blif     *
 * parsing in vpr.  This allows the routing serial numbers to be compared.*/
 
 int ipin, column, netnum;
 
 fprintf(fpout,"pinlist: ");
 
 column = 10;  /* Next column I will write to. */
 
/* LUT inputs first. */
 
 for (ipin=1;ipin<=lut_size;ipin++) {
    netnum = block[bnum].nets[ipin];
    print_net_name (netnum, &column, fpout);
 }
 
/* Output next.  */
 
 netnum = block[bnum].nets[0];
 print_net_name (netnum, &column, fpout);
 
/* Clock last. */
 
 netnum = block[bnum].nets[lut_size+1];
 print_net_name (netnum, &column, fpout);
 
 fprintf(fpout,"\n");
}


static void print_subblock(FILE *fpout, int bnum, int *io_net_mapping, int index) {
	fprintf(fpout, "\t\t<block name=\"%s\" instance=\"ble4[%d]\" mode=\"ble4\">\n", block[bnum].name, index);

	int ipin, inet, io_loc, column;

	fprintf(fpout, "\t\t\t<inputs> ");
	fprintf(fpout, "<port name=\"in\">");
	for (ipin = 1; ipin < 5; ipin++) {
		inet = block[bnum].nets[ipin];
		if (inet == OPEN) {
			fprintf(fpout, "open ");
		}
		else {
			io_loc = io_net_mapping[inet];
			if (io_loc < 18) {
				fprintf(fpout, "clb.I[%d]->crossbar ", io_loc);
			}
			else {
				fprintf(fpout, "ble4[%d].out[0]->crossbar ", io_loc - 18);
			}
		}
	}
	fprintf(fpout, "</port>");
	fprintf(fpout, "</inputs>\n");

	fprintf(fpout, "\t\t\t<outputs> ");
	fprintf(fpout, "<port name=\"out\">");
	inet = block[bnum].nets[0];
	if (inet == OPEN) {
		fprintf(fpout, "open ");
	}
	else {
		if (block[bnum].type == LUT) {
			fprintf(fpout, "lut4[0].out[0]->mux1 ");
		}
		else {
			fprintf(fpout, "ff[0].Q[0]->mux1 ");
		}
	}
	fprintf(fpout, "</port>");
	fprintf(fpout, "</outputs>\n");

	fprintf(fpout, "\t\t\t<clocks>");
	fprintf(fpout, "<port name=\"clk\">");
	inet = block[bnum].nets[5];
	if (inet == OPEN) {
		fprintf(fpout, "open ");
	}
	else {
		fprintf(fpout, "clb.clk[0]->clks ");
		
	}
	fprintf(fpout, "</port>");
	fprintf(fpout, "</clocks>\n");

	/////////////////////////////////////////////////////////
	// LUT
	/////////////////////////////////////////////////////////
	if (block[bnum].type == LUT || block[bnum].type == LUT_AND_LATCH) {
		int ipin, inet, io_loc;

		fprintf(fpout, "\t\t\t<block name=\"%s\" instance=\"lut4[0]\" mode=\"lut4\">\n", block[bnum].name);
		fprintf(fpout, "\t\t\t\t<inputs> ");
		fprintf(fpout, "<port name=\"in\">");
		for (ipin = 1; ipin < 5; ipin++) {
			inet = block[bnum].nets[ipin];
			if (inet == OPEN) {
				fprintf(fpout, "open ");
			}
			else {
				fprintf(fpout, "ble4.in[%d]->direct1 ", ipin - 1);
			}
		}
		fprintf(fpout, "</port>");
		fprintf(fpout, "</inputs>\n");
		fprintf(fpout, "\t\t\t\t<outputs> ");
		fprintf(fpout, "<port name=\"out\">lut[0].out[0]->direct:lut4 ");
		fprintf(fpout, "</port>");
		fprintf(fpout, "</outputs>\n");
		fprintf(fpout, "\t\t\t\t<clocks>");
		fprintf(fpout, "</clocks>\n");

		if (block[bnum].type == LUT) {
			fprintf(fpout, "\t\t\t\t<block name=\"%s\" instance=\"lut[0]\">\n", block[bnum].name);
		}
		else {
			fprintf(fpout, "\t\t\t\t<block name=\"%s\" instance=\"lut[0]\">\n", block[bnum].lut_name);
		}
		fprintf(fpout, "\t\t\t\t\t<inputs> ");
		fprintf(fpout, "<port name=\"in\">");
		for (ipin = 1; ipin < 5; ipin++) {
			inet = block[bnum].nets[ipin];
			if (inet == OPEN) {
				fprintf(fpout, "open ");
			}
			else {
				fprintf(fpout, "lut4.in[%d]->direct:lut4 ", ipin - 1);
			}
		}
		fprintf(fpout, "</port>");
		fprintf(fpout, "</inputs>\n");
		fprintf(fpout, "\t\t\t\t\t<outputs> ");
		column = 0;
		fprintf(fpout, "<port name=\"out\">");
		if (block[bnum].type == LUT) {
			column = 0;
			print_net_name(block[bnum].nets[0], &column, fpout);
		}
		else {
			fprintf(fpout, "%s", block[bnum].lut_name);
		}

		fprintf(fpout, "</port>");
		fprintf(fpout, "</outputs>\n");
		fprintf(fpout, "\t\t\t\t\t<clocks>");
		fprintf(fpout, "</clocks>\n");
		fprintf(fpout, "\t\t\t\t</block>\n");
		fprintf(fpout, "\t\t\t</block>\n");
		if (block[bnum].type == LUT) {
			fprintf(fpout, "\t\t\t<block name=\"open\" instance=\"ff[0]\" />\n");
		}
	}

	if (block[bnum].type == LATCH || block[bnum].type == LUT_AND_LATCH) {
		int ipin, inet, io_loc;
		if (block[bnum].type == LATCH) {
			fprintf(fpout,	"\t\t\t<block name=\"open\" instance=\"lut4[0]\" mode=\"wire\">\n"
							"\t\t\t\t<inputs>\n"
							"\t\t\t\t\t<port name=\"in\">ble4.in[0]->direct1 open open open </port>\n"
							"\t\t\t\t</inputs>\n"
							"\t\t\t\t<outputs>\n"
							"\t\t\t\t\t<port name=\"out\">lut4[0].in[0]->complete:lut4  </port>\n"
							"\t\t\t\t</outputs>\n"
							"\t\t\t\t<clocks>\n"
							"\t\t\t\t</clocks>\n"
							"\t\t\t</block>\n"				
				);
		}
		fprintf(fpout, "\t\t\t<block name=\"%s\" instance=\"ff[0]\">\n", block[bnum].name);
		fprintf(fpout, "\t\t\t\t<inputs> ");
		fprintf(fpout, "<port name=\"D\">lut4[0].out[0]->direct2 </port>");
		fprintf(fpout, "</inputs>\n");
		fprintf(fpout, "\t\t\t\t<outputs> ");
		fprintf(fpout, "<port name=\"Q\">%s ", block[bnum].name);
		fprintf(fpout, "</port>");
		fprintf(fpout, "</outputs>\n");
		fprintf(fpout, "\t\t\t\t<clocks><port name=\"clk\">ble4.clk[0]->direct3  </port>");
		fprintf(fpout, "</clocks>\n");
		fprintf(fpout, "\t\t\t</block>\n");
	}


	fprintf(fpout, "\t\t</block>\n");
}


static void print_clbs (FILE *fpout, int *cluster_occupancy, 
          int **cluster_contents, int lut_size, int cluster_size, 
          int inputs_per_cluster, int clocks_per_cluster, 
		  int num_clusters, boolean muxes_to_cluster_output_pins, int cluster_offset) {

/* Prints out one cluster (clb).  Both the external pins and the *
 * internal connections are printed out.                         */

 int ipin, icluster, sub_blk, num_clb_pins, inet, free_pin_index;
 int column, iblk;
 int redundant_outputs, used_outputs, total_used_ble_inputs;
 int total_locally_generated_ble_inputs, total_bles;

/* [0..num_nets-1].  Which clb pin connects to this net?   Need  *
 * separate data structures for input or output pins and clocks, *
 * because a clock net could connect to both a clb output and a  *
 * clock input.                                                  */

 int *io_net_mapping, *clock_net_mapping, *num_pins_in_cluster;

/* [0..num_cluster_pins].  Net connected to this clb pin. */
 int *net_of_clb_pin;
 

 io_net_mapping = (int *) my_malloc (num_nets * sizeof (int));
 clock_net_mapping = (int *) my_malloc (num_nets * sizeof (int));
 num_pins_in_cluster = (int *) my_malloc (num_nets * sizeof (int));
 
 for (inet=0;inet<num_nets;inet++) {
    io_net_mapping[inet] = OPEN;
    clock_net_mapping[inet] = OPEN;
    num_pins_in_cluster[inet] = 0;
 }

/* Counters used only for statistics purposes. */

 redundant_outputs = 0;
 total_used_ble_inputs = 0;
 total_locally_generated_ble_inputs = 0;

 num_clb_pins = inputs_per_cluster + cluster_size + clocks_per_cluster;
 net_of_clb_pin = (int*)my_malloc(num_clb_pins * sizeof(int));
 for (ipin=0;ipin<num_clb_pins;ipin++)
    net_of_clb_pin[ipin] = OPEN;

 for (icluster=0;icluster<num_clusters;icluster++) {

/* First I have to get a list of the pins that have to be routed to *
 * this cluster's inputs.  Also need to know which pin will supply  *
 * each net needed by the LUTs of this cluster.  Start with the     *
 * cluster output nets, as nets generated in the cluster don't need *
 * to be routed here as inputs.                                     */

    for (iblk=0;iblk<cluster_occupancy[icluster];iblk++) {
       sub_blk = cluster_contents[icluster][iblk];
       inet = block[sub_blk].nets[0];   /* Output */
       net_of_clb_pin[inputs_per_cluster + iblk] = inet;
      
       if (io_net_mapping[inet] != OPEN) {
          printf("Error in print_clbs.  Net #%d (%s) in cluster %d\n",
                   inet, net[inet].name, icluster);
          printf("appears to be driven by two outputs.\n");
          exit (1);
       }

       io_net_mapping[inet] = inputs_per_cluster + iblk;
       num_pins_in_cluster[inet]++;
    }   

/* Now do the input pins. */

    free_pin_index = 0;
    for (iblk=0;iblk<cluster_occupancy[icluster];iblk++) {
       sub_blk = cluster_contents[icluster][iblk];
       for (ipin=1;ipin<=lut_size;ipin++) {
          inet = block[sub_blk].nets[ipin];
          if (inet != OPEN) {
             num_pins_in_cluster[inet]++;
             total_used_ble_inputs++; /* Used LUT inputs;doesn't include clk */

             if (io_net_mapping[inet] == OPEN) {  /* Must get from CLB IPIN */
                io_net_mapping[inet] = free_pin_index;
                net_of_clb_pin[free_pin_index] = inet;
                free_pin_index++;
             }

             else if (io_net_mapping[inet] >= inputs_per_cluster) { 
               /* Input is locally generated (output of a BLE). */
                total_locally_generated_ble_inputs++;
             }
          }
       }
    }

    if (free_pin_index > inputs_per_cluster) {
       printf("Error in print_clbs:  cluster %d has %d input pins.\n",
             icluster, free_pin_index);
       exit (1);
    }

/* Now do the clocks. */ 

    free_pin_index = inputs_per_cluster + cluster_size;
    for (iblk=0;iblk<cluster_occupancy[icluster];iblk++) {
       sub_blk = cluster_contents[icluster][iblk];
       inet = block[sub_blk].nets[lut_size + 1];
       if (inet != OPEN) {

/* No internal output - clock connection, so don't increment num_pins_in *
 * _cluster so that I can guarantee that all clock inputs and outputs    *
 * that generate clocks do come out to the cluster pins.                 */

          if (clock_net_mapping[inet] == OPEN) {
             clock_net_mapping[inet] = free_pin_index;
             net_of_clb_pin[free_pin_index] = inet;
             free_pin_index++;
          }
       }
    }

    if (free_pin_index > num_clb_pins) {
       printf("Error in print_clbs:  cluster %d has %d clock pins.\n",
           icluster, free_pin_index - inputs_per_cluster - cluster_size);
       exit (1);
    }

/* Remove all nets that are now routed completely within the cluster from  *
 * the cluster input/output pins.                                          */

    for (ipin=0;ipin<num_clb_pins;ipin++) {
       inet = net_of_clb_pin[ipin];
       if (inet != OPEN) {
          if (num_pins_in_cluster[inet] == net[inet].num_pins) {
             net_of_clb_pin[ipin] = OPEN;   /* Remove. */
             redundant_outputs++;
             if (ipin < inputs_per_cluster || ipin >= inputs_per_cluster
                    + cluster_size) {
                printf("Error in print_clbs:  cluster %d pin %d is not an\n",
                     icluster, ipin);
                printf("output, but it has been marked as a net that doesn't\n");
                printf("fanout, and will be removed.  Net #%d (%s)\n", inet,
                     net[inet].name);
                exit (1);
             }
          }
       } 
    }

/* Do the actual printout. */

/* Name clb (arbitrarily) after the first block it contains. */

	fprintf(fpout, "\t<block name=\"%s\" instance=\"clb[%d]\" mode=\"clb\">\n", block[cluster_contents[icluster][0]].name, cluster_offset + icluster);
	fprintf(fpout, "\t\t<inputs>\n");
	fprintf(fpout, "\t\t<port name=\"I\">");
	column = 16;
	for (ipin = 0; ipin<18; ipin++)
		print_net_name(net_of_clb_pin[ipin], &column, fpout);
	fprintf(fpout, "</port>\n");
	fprintf(fpout, "\t\t</inputs>\n");

	fprintf(fpout, "\t\t<outputs>\n");
	fprintf(fpout, "\t\t<port name=\"O\">");
	column = 16;
	for (ipin = 18; ipin < 26; ipin++) {
		if (net_of_clb_pin[ipin] == OPEN) {
			fprintf(fpout, "open ");
		}
		else {
			fprintf(fpout, "ble4[%d].out[0]->clbouts1 ", io_net_mapping[net_of_clb_pin[ipin]] - 18);
		}
	}
	fprintf(fpout, "</port>\n");
	fprintf(fpout, "\t\t</outputs>\n");

	fprintf(fpout, "\t\t<clocks>\n");
	fprintf(fpout, "\t\t<port name=\"clk\">");
	column = 16;
	print_net_name(net_of_clb_pin[26], &column, fpout);
	fprintf(fpout, "</port>\n");
	fprintf(fpout, "\t\t</clocks>\n");

	for (iblk=0;iblk<cluster_occupancy[icluster];iblk++) {
       sub_blk = cluster_contents[icluster][iblk];
	   print_subblock(fpout, sub_blk, io_net_mapping, iblk);
    }

	for (iblk = cluster_occupancy[icluster]; iblk<8; iblk++) {
		fprintf(fpout, "\t\t<block name=\"open\" instance=\"ble4[%d]\"/>\n", iblk);
	}

	fprintf(fpout, "\t</block>\n");
 
    fprintf (fpout, "\n");
    
/* Reset the data structures in order to do the next cluster. */

    for (iblk=0;iblk<cluster_occupancy[icluster];iblk++) {
       sub_blk = cluster_contents[icluster][iblk];
       for (ipin=0;ipin<=lut_size;ipin++) {
          inet = block[sub_blk].nets[ipin];
          if (inet != OPEN) {
             io_net_mapping[inet] = OPEN;
             num_pins_in_cluster[inet] = 0;
          }
       }
       inet = block[sub_blk].nets[lut_size+1];   /* Clock */
       if (inet != OPEN)
          clock_net_mapping[inet] = OPEN;
    }

    for (ipin=0;ipin<num_clb_pins;ipin++) 
       net_of_clb_pin[ipin] = OPEN;
 }

 used_outputs = num_blocks - num_p_inputs - num_p_outputs - redundant_outputs;
 total_bles = num_blocks - num_p_inputs - num_p_outputs;

 printf("\nAverage number of cluster outputs used outside cluster: %f\n", 
           (float) used_outputs / (float) num_clusters);

 printf("Fraction outputs made local: %f\n", (float) redundant_outputs /
       (float) (total_bles));

 printf ("Average number of used LUT inputs per BLE (excluding clocks): %f\n", 
         (float) total_used_ble_inputs / (float) total_bles);

 printf ("Average fraction of BLE (LUT) inputs generated locally: %f\n",
         (float) total_locally_generated_ble_inputs / 
         (float) total_used_ble_inputs);

 free (io_net_mapping);
 free (clock_net_mapping);
 free (num_pins_in_cluster);
 free (net_of_clb_pin);
}


void output_clustering (int **cluster_contents, int *cluster_occupancy,
     int cluster_size, int inputs_per_cluster, int clocks_per_cluster,
     int num_clusters, int lut_size, boolean global_clocks, boolean
     muxes_to_cluster_output_pins, boolean *is_clock, char *out_fname) {

/* This routine dumps out the output netlist in a format suitable for  *
 * input to vpr.  The clb pins are in the following order:             *
 * inputs_per_cluster inputs, cluster_size outputs, and                *
 * clocks_per_cluster clocks.  For a single LUT cluster, this reduces  *
 * to N LUT inputs, 1 output, followed by the clock input.  Pins that  *
 * are unconnected are marked as open.  If global_clocks is TRUE, all  *
 * the clock nets (nets marked with TRUE in is_clock) will be set to   *
 * be global.  This routine also dumps out the internal structure of   *
 * the cluster, in essentially a graph based format.  For each LUT +   * 
 * FF subblock, I write out which cluster pin connects to each of the  *
 * LUT pins.  The LUT + FF pins are ordered as N LUT inputs, LUT       *
 * output, then clock.  If a pin is not connected, it is marked as     *
 * OPEN.  Otherwise, it is given the number of the cluster pin to      *
 * which it connects -- the cluster pins begin with inputs (starting   *
 * from 0), then list outputs, then clocks.  Hence all cluster pins    *
 * have an index from 0 to inputs_per_cluster + cluster_size +         *
 * clocks_per_cluster - 1.                                             *
 * If cluster_contents is NULL, this routine prints out an unclustered *
 * netlist -- i.e. it just prints out the LUT+FF logic blocks with all *
 * pins distint.                                                       */

 FILE *fpout;
 int bnum, netnum, column;
 boolean skip_clustering;
 int cluster_offset;

 fpout = my_fopen (out_fname, "w", 0);

 fprintf(fpout, "<block name=\"%s\" instance=\"FPGA_packed_netlist[0]\">\n\n", out_fname);
 fprintf(fpout, "\t<inputs>\n");
 for (bnum = 0; bnum<num_blocks; bnum++) {
	 if (block[bnum].type == INPAD) {
		 netnum = block[bnum].nets[0];
		 if (!is_clock[netnum])
			 fprintf(fpout, "\t\t\t%s\n", net[netnum].name);
	 }
 }
 fprintf(fpout, "\t</inputs>\n");
 fprintf(fpout, "\t<outputs>\n");
 for (bnum = 0; bnum<num_blocks; bnum++) {
	 if (block[bnum].type == OUTPAD) {
		 fprintf(fpout, "\t\t%s\n", block[bnum].name);
	 }
 }
 fprintf(fpout, "\t</outputs>\n");

 fprintf(fpout, "\t<clocks>\n");
	for (bnum = 0; bnum<num_blocks; bnum++) {
		if (block[bnum].type == INPAD) {
			netnum = block[bnum].nets[0];
			if (is_clock[netnum])
				fprintf(fpout, "\t\t\t%s\n", net[netnum].name);
		}
	 }
 fprintf(fpout, "\t</clocks>\n");

 cluster_offset = 0;
 
 for (bnum = 0; bnum < num_blocks; bnum++) {
	 if (block[bnum].type == INPAD) {
		 print_inpad(fpout, bnum, cluster_offset);
		 cluster_offset++;
	 }
	 else if (block[bnum].type == OUTPAD) {
		 print_outpad(fpout, bnum, cluster_offset);
		 cluster_offset++;
	 }
 }


 print_clbs (fpout, cluster_occupancy, cluster_contents, lut_size,
         cluster_size, inputs_per_cluster, clocks_per_cluster, 
         num_clusters, muxes_to_cluster_output_pins, cluster_offset); 

 fprintf(fpout, "</block>\n", out_fname);

 fclose (fpout);
}
