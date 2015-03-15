# Creating and Modifying Tasks #

## Introduction ##
Tasks provide a configuration framework for running the VTR flow.  A task specifies such things as which architectures and benchmarks to use.

## File Structure ##
All tasks are located here:
```

<vtr>/vtr_flow/tasks
```
If the user wishes to create his/her own task, it must be created in this location.

All tasks must contain a configuration file, located here:
```

<vtr>/vtr_flow/tasks/<task_name>/config/config.txt
```

## Creating a New Task ##
  1. Create the folder `<vtr>/vtr_flow/tasks/<task_name>`
  1. Create the folder `<vtr>/vtr_flow/tasks/<task_name>/config`
  1. Create and configure the file `<vtr>/vtr_flow/tasks/<task_name>/config/config.txt`

## Task Configuration File ##
The task configuration file contains key/value pairs separated by the '=' character.  Comment line are indicted using the '#' symbol. Example configuration file:
```

# Path to directory of circuits to use
circuits_dir=benchmarks/verilog

# Path to directory of architectures to use
archs_dir=arch/timing

# Add circuits to list to sweep
circuit_list_add=ch_intrinsics.v
circuit_list_add=diffeq1.v

# Add architectures to list to sweep
arch_list_add=k6_N10_memSize16384_memData64_stratix4_based_timing_sparse.xml

# Parse info and how to parse
parse_file=vpr_standard.txt
```
### Required Fields ###
**circuits\_dir**: Directory path of the benchmark circuits.  (Absolute path or relative to `<vtr>/vtr_flow/`)

**archs\_dir**: Directory path of the architecture XML files.  (Absolute path or relative to `<vtr>/vtr_flow/`)

**circuit\_list\_add**: Name of a benchmark circuit file.  Use multiple lines to add multiple circuits.

**arch\_list\_add**: Name of an architecture XML file.  Use multiple lines to add multiple architectures.

**parse\_file**: File used for parsing and extracting the statistics.  See [Parse Configuration Files](ParseFiles.md). (Absolute path or relative to `<vtr>/vtr_flow/parse/parse_config`)

### Optional Fields ###
**script\_path**: Script to run for each architecture/circuit combination.  The default is to use run\_vtr\_flow.pl, which runs the entire (or partial) VTR flow.  However, a user can use this option to provide their own script.  The user script will be used in place of run\_vtr\_flow.pl.  The circuit path will be provided as the first argument, and architecture path as the second argument to the user script. (Absolute path or relative to `<vtr>/vtr_flow/scripts/` or `<vtr>/vtr_flow/tasks/<task_name>/config/`)

**script\_params**: Parameters to be passed to the script.  This can be used to run partial VTR flows, or to preserve intermediate files. For information on parameters to the default script see [run\_vtr\_flow.pl](Run_VTR_Flow.md).

**pass\_requirements\_file**: Path to the [Pass Requirements File](PassRequirementsFiles.md).  (Absolute path or relative to `<vtr>/vtr_flow/parse/pass_requirements/` or `<vtr>/vtr_flow/tasks/<task_name>/config/`)