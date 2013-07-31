#ifndef NEON_API_H
#define NEON_API_H

#include <string>
#include "neon_types.h"

/**** Function Declarations ****/
/* Reads in user options and initializes the tool */
void neon_init(INP int argc, INP char **argv);
/* returns true if extension of 'filename' matches 'extension' TODO: move elsewhere */
bool check_file_extension(INP std::string &filename, INP std::string extension);


/**** Class Declarations ****/
/* Class containing user-settable options */
class User_Options {
public:
	std::string arch_file;	/* path to architecture file */
		
};

#endif /* NEON_API_H */
