# parse\_vtr\_flow.pl #

## Overview ##
This script parses statistics following the execution of single VTR flow.  If the user is using the task framework, [parse\_vtr\_task.pl](Parse_VTR_Task.md) should be used instead.

The script is located here:
```

<vtr>/vtr_flow/scripts/parse_vtr_flow.pl
```

## Usage ##
The usage is:
```

parse_vtr_flow.pl <parse_path> <parse_config_file>
```

**parse\_path**: Directory path that contains the files that will be parsed (vpr.out, odin.out, etc).

**parse\_config\_file**: Path to the [Parse Configuration File](ParseFiles.md).

## Output ##
The script will produce no standard output.  A single file named parse\_results.txt will be produced in the `<parse_path>` folder.  The file is tab delimited and contains two lines.  The first line is a list of field names that were searched for, and the second line contains the associated values.