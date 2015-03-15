# Requirements #
VTR can be installed in 32-bit or 64-bit Linux, or Cygwin.  However, large benchmarks may require more than 4GB of virtual memory, and will fail to run in 32-bit Linux or Cygwin.

# Downloading and Installing VTR #

  1. Download the VTR package [here](http://www.eecg.utoronto.ca/vtr/terms.html)
  1. Unpack to a directory of your choice (Hereafter referred to as `<vtr>`)
  1. Navigate to `<vtr>` and run ```
make```

# Verifying Installation #
To verify that VTR has been installed correctly, run:
```
<vtr>/vtr_flow/scripts/run_vtr_task.pl basic_flow```

The expected output is:
```

k6_N10_memSize16384_memData64_40nm_timing/ch_intrinsics...OK
```

If the task does not report 'OK', please check the [FAQ Page](FAQ.md) before raising an issue.

_Explanation:_ The above test will run the task named **basic\_flow**.  This task will run a single benchmark through the entire VTR flow from verilog to routing.  The output of the task is formatted as `<architecture>`/`<benchmark>`...`<status>`.  For more information see [Running VTR](RunningVTR.md).