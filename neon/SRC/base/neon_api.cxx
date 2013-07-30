#include <cstdlib>
#include <cstring>
#include <sstream>
#include "neon_api.h"
#include "globals.h"
#include "print.h"
#include "exception.h"

using namespace std;

/**** Static Function Declarations ****/
/* compares extension of specified filename with specified string
*  returns true if they are the same, false otherwise */
bool check_file_extension(INP string &filename, INP string extension){
	print_debug("Entering check_file_extension\n");

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

	print_debug("Exiting check_file_extension\n");
	return result;
}  



/**** Function Definitions ****/

/* Reads in architecture file, as well as user options */
void neon_init(INP int argc, INP char **argv){
	print_debug("Entering neon_init\n");

	//TODO: print title

	/* check that we have the minimum number of arguments */
	if (argc < 2){
		neon_print_usage();
		exit( EXIT_FAILURE );
	}

	string arch_file;

	neon_parse_command_args(argc, argv, arch_file);

	print_debug("Exiting neon_init\n");
	return;
}


/* Parses the command line options */
void neon_parse_command_args(INP int argc, INP char **argv, OUTP string &arch_file){
	print_debug("Entering neon_parse_command_args\n");

	int iopt = 0;
	/* first parameter is the name of the program -- skip */
	iopt++;	
	/* next parameter should be the architecture file name */
	arch_file = convert_to_string(argv[iopt]);
	bool arch_is_xml = check_file_extension(arch_file, convert_to_string(".xml"));
	if ( false == arch_is_xml ){
		stringstream ex_msg;
		ex_msg << "neon_parse_command_args: specified architecture file does not have .xml extension: "
			<< arch_file;
		neon_throw(ex_msg, INIT, __FILE__, __LINE__);
	}

	/* next we read in the options */
	iopt++;
	for ( ; iopt < argc; iopt++ ){
		if ( strcmp( argv[iopt], "-something" ) == 0 ){

		} else if ( strcmp(argv[iopt], "-something_else") == 0 ) {
		
		} else {
			stringstream ex_msg;
			ex_msg << "neon_parse_command_args: unrecognized command-line option: " << argv[iopt];
			neon_throw(ex_msg, INIT, __FILE__, __LINE__);
		}
	}
	


	print_debug("Exiting neon_parse_command_args\n");
	return;
}

/* Prints instructions for tool usage */
void neon_print_usage(){
	print_debug("Entering neon_print_usage\n");
	print_info(PP_HIGH, "Program usage:\n" << "Placeholder message\n");
	print_debug("Exiting neon_print_usage\n");
}
