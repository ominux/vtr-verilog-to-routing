# Overview #

This script is used to execute one or more tasks.  For more information about tasks and task execution see [Running VTR](RunningVTR.md).  See also [Creating and Modifying Tasks](CreatingTasks.md).

# Usage #
The usage is:
```

<vtr>/vtr_flow/scripts/run_vtr_task.pl <task_name1> <task_name2> ... [OPTIONS]
```

# Options #
**`-p <N>`**: Perform parallel execution using N threads.  **Note**: Large benchmarks will use very large amounts of memory (several gigabytes).  Because of this, parallel execution often saturates the physical memory, requiring the use of swap memory, which will cause slower execution.  Be sure you have allocated a sufficiently large swap memory or errors may result.

**`-l <task_list_file>`**: A file containing a list of tasks to execute.  Each task name should be on a separate line.

**`-hide_runtime`**: Do not show runtime estimates.

**Note**: At least one task must be specified, either directly as a parameter or through the `-l` option.

# Output #
Each task will execute the script specified in the configuration file for every benchmark/circuit combination.  The standard output of the underlying script will be forwarded to the output of this script.  Headers will be added to each task in the following format:
```

```

<task_name1>
--------------------
<underlying script output>

<task_name2>
--------------------
<underlying script output>
```
```

If golden results exist (See [parse\_vtr\_task.pl](Parse_VTR_Task.md)), they will be inspected for runtime values.  Any entries in the golden results with with the field names _pack\_time_, _place\_time_, _route\_time_, _min\_chan\_width\_route\_time_, or _crit\_path\_route\_time_ will be summed to determine an estimated runtime for the benchmark.  This information will be output in the following format before each circuit/benchmark combination:

<pre>
Current time: Jan-01 01:00 AM.  Expected runtime of next benchmark: 3 minutes<br>
</pre>

Depending on the estimated runtime the units will automatically change between seconds, minutes and hours.  This will not be output if the golden results file cannot be found, if the `-hide_runtime` option is used, or if the underlying script is changed from the default [run\_vtr\_flow.pl](Run_VTR_Flow.md).