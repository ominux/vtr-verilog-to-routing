# Parse Configuration Files #

## Overview ##
A parse configuration file defines a set of values that will be searched for within user-specified files.

## Format ##
The configuration file contains one line for each value to be searched for.  Each line contains a semicolon delimited triple in the following format:

```

<field_name>;<file_to_search_within>;<regex_expression>
```

**field\_name**: The name of the value to be searched for.  This name is used when generating the output files of [parse\_vtr\_task.pl](Parse_VTR_Task.md) and [parse\_vtr\_flow.pl](Parse_VTR_Flow.md).

**file\_to\_search\_within**: The name of the file that will be searched (example: vpr.out, odin.out, etc.)

**regex\_expression**: A perl regular expression used to find the desired value.  The regex must contain a single grouping '()' which will contain the desired value.

## Example File ##
The following is an example file, vpr\_standard.txt:
```

vpr_status;output.txt;vpr_status=(.*)
vpr_seconds;output.txt;vpr_seconds=(\d+)
width;vpr.out;Best routing used a channel width factor of (\d+)
pack_time;vpr.out;Packing took (.*) seconds
place_time;vpr.out;Placement took (.*) seconds
route_time;vpr.out;Routing took (.*) seconds
num_pre_packed_nets;vpr.out;Total Nets: (\d+)
num_pre_packed_blocks;vpr.out;Total Blocks: (\d+)
num_post_packed_nets;vpr.out;Netlist num_nets:\s*(\d+)
num_clb;vpr.out;Netlist clb blocks:\s*(\d+)
num_io;vpr.out;Netlist inputs pins:\s*(\d+)
num_outputs;vpr.out;Netlist output pins:\s*(\d+)
num_lut0;vpr.out;(\d+) LUTs of size 0
num_lut1;vpr.out;(\d+) LUTs of size 1
num_lut2;vpr.out;(\d+) LUTs of size 2
num_lut3;vpr.out;(\d+) LUTs of size 3
num_lut4;vpr.out;(\d+) LUTs of size 4
num_lut5;vpr.out;(\d+) LUTs of size 5
num_lut6;vpr.out;(\d+) LUTs of size 6
unabsorb_ff;vpr.out;(\d+) FFs in input netlist not absorbable
num_memories;vpr.out;Netlist memory blocks:\s*(\d+)
num_mult;vpr.out;Netlist mult_36 blocks:\s*(\d+)
equiv;abc.out;Networks are (equivalent)
error;output.txt;error=(.*)
```