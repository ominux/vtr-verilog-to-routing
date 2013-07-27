#ifndef NEON_API_H
#define NEON_API_H

#include <string>
#include "neon_types.h"

/**** Function Declarations ****/
void neon_init(INP int argc, INP char **argv);
void neon_parse_command_args(INP int argc, INP char **argv, OUTP std::string &arch_file); 
bool check_file_extension(INP std::string &filename, INP std::string extension);


/**** Class Declarations ****/
class Neon_Options {
public:
		
};

#endif /* NEON_API_H */
