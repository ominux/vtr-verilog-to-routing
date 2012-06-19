#ifndef PATH_DELAY
#define PATH_DELAY

#define DO_NOT_ANALYSE -1

/*********************** Defines for timing options *******************************/

#define SLACK_DEFINITION 4
/* Choose whether, and how, to normalize negative slacks load_net_slack_and_slack_ratio for optimization 
  (not for final analysis, since real slacks are always given here).
  Possible values:
   1: Slacks are not normalized at all.  Negative slacks are possible.
   2: All negative slacks are "clipped" to 0.  
   3: If negative slacks exist, increase the value of all slacks by the largest negative slack.  Only the critical path will have 0 slack.
   4: Set the required time to the max of the "real" required time (constraint + tnode[inode].clock_skew) and the arrival time.  Only the critical path will have 0 slack.
*/

#define SLACK_RATIO_DEFINITION 1
/* Which definition of slack ratio (related to criticality) should VPR use?  In general, slack ratio is slack/maximum delay.  Possible values:
   1: slack ratio = minimum over all traversals of (slack of edge for this traversal)/(maximum required time T_req_max for this traversal)
   2: slack ratio = (slack of this edge)/(maximum required time T_req_max in design)
   Note that if SLACK_DEFINITION = 4 above, T_req_max will be taken from normalized required times, not real required times.
*/

/* WARNING: SLACK_DEFINITION 3 and SLACK_RATIO_DEFINITION 1 are not compatible. */

/*************************** Function declarations ********************************/

void alloc_and_load_timing_graph(t_timing_inf timing_inf);

void alloc_and_load_pre_packing_timing_graph(float block_delay,
		float inter_cluster_net_delay, t_model *models, t_timing_inf timing_inf);

t_linked_int *allocate_and_load_critical_path(void);

void load_timing_graph_net_delays(float **net_delay);

void load_net_slack_and_slack_ratio(boolean do_lut_input_balancing, boolean is_final_analysis);

void free_timing_graph(void);

void print_timing_graph(char *fname);

void print_net_slack(char *fname);

void print_net_slack_ratio(char *fname);

void print_lut_remapping(char *fname);

void print_critical_path(char *fname);

void get_tnode_block_and_output_net(int inode, int *iblk_ptr, int *inet_ptr);

void do_constant_net_delay_timing_analysis(t_timing_inf timing_inf,
		float constant_net_delay_value);

void print_timing_graph_as_blif(char *fname, t_model *models);

/*************************** Variable declarations ********************************/

extern int num_netlist_clocks; /* [0..num_netlist_clocks - 1] number of clocks in netlist */

extern t_clock * clock_list; /* [0..num_netlist_clocks - 1] array of clocks in netlist */

extern float ** timing_constraints; /* [0..num_netlist_clocks - 1 (source)][0..num_netlist_clocks - 1 (destination)] */

extern float ** net_slack, **net_slack_ratio; /* [0..num_nets-1][1..num_pins] */

#endif