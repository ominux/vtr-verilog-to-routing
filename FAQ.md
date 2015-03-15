# Frequently Asked Questions #

## Installing VTR ##
Nothing here yet.

## Running VTR ##

### Q: How do I run the VTR flow for a single benchmark? ###
[Running a Single Benchmark](RunningVTR#Running_a_Single_Benchmark.md) explains how to run the VTR flow for a single benchmark.  However, the task framework provides the additional benefits of file organization and statistics extraction. For this reason, it is suggested to use the task framework even when running a single benchmark circuit (See [Tasks](RunningVTR#Tasks.md)).

### Q: How do I run the VTR flow for a collection of benchmarks? ###
Use the task framework (See [Tasks](RunningVTR#Tasks.md)).

### Q: How do I run the VTR flow for my own circuits? ###
To run the VTR flow for your own circuits, you must create a new task and specify your circuits in the configuration file.  See [Creating and Modifying Tasks](CreatingTasks.md).  You will also need to provide the name of the clock signal for the circuit (See [Circuit Clock Files](ClocksFile.md)).

### Q: How do I run the VTR flow for my own architecture files? ###
To run the VTR flow for your own architecture files, you must create a new task and specify your architecture files in the configuration file.  See [Creating and Modifying Tasks](CreatingTasks.md).

### Q: How do I run only a portion of the VTR flow (i.e. Only ODIN)? ###
The [run\_vtr\_flow.pl](Run_VTR_Flow.md) script provides command line options to specify the starting and ending stage of the VTR flow.  If you are using the task framework, the command line options can be passed to the script using the configuration file (See [Creating and Modifying Tasks: Optional Fields](CreatingTasks#Optional_Fields.md)).

### Q: How do I run the VTR flow for circuits with more than one verilog file? ###
Although ODIN II does support synthesis of circuits comprised of multiple verilog files, the provided scripts do not yet support this feature.  This will be added in the future.