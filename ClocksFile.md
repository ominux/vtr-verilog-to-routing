# Circuit Clock Files #

## Overview ##
The second stage of the VTR flow, ABC, removes the clock signals from the latches in the .blif file it produces.  However, VPR requires these clock signals.  The clock signals must be reinserted into the blif file prior to VPR execution.  This is one of the roles of the 'Script' stage of the VTR flow.

The 'Script' stage will check two different files for the name of the clock signal for the circuit it is working with.  The first file is for circuits included in the VTR release, and the second file is for the user to provide his/her own circuits.  If the circuit name is found in both files, the clock signal name will be taken from the user provided file.

## Released Benchmarks ##
The following file is provides as part of the VTR release, and includes the name of the clock signal for all released benchmarks:

```

<vtr>/vtr_flow/benchmarks/misc/benchmark_clocks.clock
```

Each line of the file contains a benchmark name and associated clock signal name, separated by white space.  The benchmark name and clock name are case sensitive.

Example file:
```

or1200			clk
diffeq1         	clk
stereovision2   	tm3_clk_v0
stereovision3   	tm3_clk_v0
sha             	clk_i
mkPktMerge      	CLK
mkSMAdapter4B   	wciS0_Clk
render          	i_clk100
fpu_full_unit   	clk
bgm			clock
```

## User Benchmarks ##
For user provided benchmarks, the user should create his/her own file, which must be located here:

```

<vtr>/vtr_flow/benchmarks/misc/user_clocks.clock
```

This file is intentionally not included as part of the release, so that any future releases will not overwrite user created data.