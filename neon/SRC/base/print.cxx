#include "print.h"

Neon_Error _neon_error;


/*
What do we want?
- message
- warning
- error
- log

- error will display file, function, line
- warning will display file, function -- should this only be when an option is turned on?
- log will output the input stream to a predifined file


- do these functions really need to inherit ostream? what are the pros/cons?
pros: access to all the various modifiers available to ostream
	- seems like this is the way to go
cons: some complexity?

cout should be used for printing program output
cerr should be used for printing info, messages, errors, warnings, etc.
this is done so that program output may be redirected to a file without affecting
any information or error output (which will still go to terminal)
*/





