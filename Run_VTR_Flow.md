# run\_vtr\_flow.pl #

## Overview ##
This script runs the VTR flow for a single benchmark circuit and architecture file.  The following figure illustrates the stages of the VTR flow and associated input and output files.

![http://vtr-verilog-to-routing.googlecode.com/svn/wiki/images/flow_diagram_text_updated.png](http://vtr-verilog-to-routing.googlecode.com/svn/wiki/images/flow_diagram_text_updated.png)

The script is located here:
```

<vtr>/vtr_flow/scripts/run_vtr_flow.pl
```

## Usage ##
The usage is:
```

run_vtr_flow.pl <circuit_file> <architecture_file> [OPTIONS]
```

The script will create a '`./temp`' directory, unless otherwise specified with the **`-temp_dir`** option.   The circuit file and architecture file will be copied to the temporary directory.  All stages of the flow will be run within this directory.  Several intermediate files will be generated and deleted upon completion.  **Note: Users should ensure that no important files are kept in this directory as they may be deleted.**

## Options ##
**`-starting_stage <stage>`**: Start the VTR flow at the specified stage.  Acceptable values: _odin_, _abc_, _scripts_, _vpr_. Default value is _odin_.

**`-ending_stage <stage>`**: End the VTR flow at the specified stage.  Acceptable values: _odin_, _abc_, _scripts_, _vpr_. Default value is _vpr_.

**`-keep_intermediate_files`**: Do not delete the intermediate files.

**`-temp_dir`**: Temporary directory used for execution and intermediate files.  The script will automatically create this directory if necessary.  Default value: `'./temp`'.

## Output ##
The standard out of the script will produce a single line with the format:
```

<architecture>/<circuit_name>...<status>
```

If execution completed successfully the status will be 'OK'.  Otherwise, the status will indicate which stage of execution failed.

The script will also produce an output file (`*`.out) for each stage, which contains the standout output of the executable.