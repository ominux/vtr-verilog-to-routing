# parse\_vtr\_task.pl #

## Overview ##

This script is used to parse the output of one or more tasks.  The values that will be parsed are specified using a [Parse Configuration File](ParseFiles.md), which is specified as part of the overall task configuration (See [Creating and Modifying Tasks](CreatingTasks.md)).

The script will always parse the results of the latest execution of the task.

## Usage ##
The usage is:
```

<vtr>/vtr_flow/scripts/parse_vtr_task.pl <task_name1> <task_name2> ... [OPTIONS]
```

## Options ##
**`-l <task_list_file>`**: A file containing a list of tasks to parse.  Each task name should be on a separate line.


**Note**: At least one task must be specified, either directly as a parameter or through the `-l` option.

**`-create_golden`**: The results will be stored as golden results.  If previous golden results exist they will be overwritten.  The golden results are located here:
```

<vtr>/vtr_flow/tasks/<task_name>/config/golden_results.txt
```
**`-verify_golden`**: The results will be compared to the golden results using the [Pass Requirements File](PassRequirementsFiles.md) specified in the task configuration.  A 'Pass' or 'Fail' will be output for each task (see below).  In order to compare against the golden results, they must already exist, and have the same architectures, circuits and parse fields, otherwise the script will report 'Error'.  If the golden results are missing, or need to be updated, use the **`-create_golden`** option.



## Output ##
This script produces no standard output, unless **`-verify_golden`** is used.  A tab delimited file containing the parse results will be produced for each task.  The file will be located here:

```

<vtr>/vtr_flow/tasks/<task_name>/run<#>/parse_results.txt
```

If **`-verify_golden`** is used, the script will output one line for each task in the format:

```

<task_name>...<status>
```

The status will be `[Pass]`,`[Fail]`, or `[Error]`.