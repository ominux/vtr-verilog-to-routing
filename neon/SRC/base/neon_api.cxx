
#include "neon_api.h"
#include "globals.h"
#include "print.h"
#include "exception.h"

using namespace std;

/**** Static Function Declarations ****/
/* compares extension of specified filename with specified string
*  returns true if they are the same, false otherwise */
bool check_file_extension(INP string &filename, INP string extension){
	bool result = false;
	int ext_len = extension.length();
	int file_len = filename.length();

	/* check if either string is empty */
	if ( 0 == file_len || 0 == ext_len ){
		neon_throw("check_file_extension: zero-length strings not allowed.",
			INIT, __FILE__, __LINE__);
	}

	string file_ext = filename.substr( file_len - ext_len, file_len - 1 );
	if ( file_ext.compare( extension ) == 0 ){
		/* compare returns 0 if the two strings are the same */
		result = true;
	} else {
		result = false;
	}

	return result;
}  



/**** Function Definitions ****/

/* Reads in architecture file, as well as user options */
void neon_init(INP int argc, INP char **argv){

	string arch_file;

	neon_parse_command_args(argc, argv, arch_file);

	return;
}



void neon_parse_command_args(INP int argc, INP char **argv, OUTP string &arch_file){

	int iopt = 0;
	/* first parameter is the name of the program -- skip */
	iopt++;	
	/* next parameter should be the architecture file name */
	arch_file = char_to_string(argv[iopt]);
	bool arch_is_xml = check_file_extension(arch_file, char_to_string(".xml"));

	
	return;
}
