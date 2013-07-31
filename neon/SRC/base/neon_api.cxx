#include <cstdlib>
#include <cstring>
#include <sstream>
#include "neon_api.h"
#include "globals.h"
#include "print.h"
#include "exception.h"
#include "read_xml_arch_file.h" /* taken from VPR -- is VPR specific in some cases */
#include "physical_types.h"

using namespace std;

/**** Function Declarations ****/ 
static void neon_parse_command_args(INP int argc, INP char **argv);
static void neon_print_usage();
static void neon_print_title();
 
/**** Function Definitions ****/
/* compares extension of specified filename with specified string
*  returns true if they are the same, false otherwise */
bool check_file_extension(INP string &filename, INP string extension){
	print_debug("Entering check_file_extension\n");

	bool result = false;
	int ext_len = extension.length();
	int file_len = filename.length();

	/* check if either string is empty */
	if ( 0 == file_len || 0 == ext_len ){ 
		//neon_throw("check_file_extension: zero-length strings not allowed.",
		//	INIT, __FILE__, __LINE__);
		result = false;
	} else if (file_len <= ext_len){
		result = false;
	} else {
		string file_ext = filename.substr( file_len - ext_len, file_len - 1 );
		if ( file_ext.compare( extension ) == 0 ){
			/* compare returns 0 if the two strings are the same */
			result = true;
		} else {
			result = false;
		}
	}
	print_debug("Exiting check_file_extension\n");
	return result;
}  



/**** Function Definitions ****/

/* Reads in architecture file, as well as user options */
void neon_init(INP int argc, INP char **argv){
	print_debug("Entering neon_init\n");

	//TODO: print title
	neon_print_title();

	/* check that we have the minimum number of arguments */
	if (argc < 2){
		neon_print_usage();
		exit( EXIT_FAILURE );
	}

	/* parse user-specified options into global g_user_opts variable */
	neon_parse_command_args(argc, argv);

	/* initialize arch to 0 */
	memset(&g_arch, 0, sizeof(t_arch));

	/* load the specified architecture */
	boolean timing_enabled = TRUE;
	int num_types;
	s_type_descriptor *type_descriptors = NULL;
	/* note, in the future XmlReadArch will start throwing VPR exceptions */
	XmlReadArch(g_user_opts.arch_file.c_str(), timing_enabled, &g_arch, &type_descriptors, &num_types);
	print_info(PP_MED, "Read in architecture: " << g_user_opts.arch_file << "\n");

	/* Next step is to add in improved fabric descrioption to the architecture language */

	print_debug("Exiting neon_init\n");
	return;
}


/* Parses the command line options 
   Options are parsed into the global g_user_opts variable */
static void neon_parse_command_args(INP int argc, INP char **argv){
	print_debug("Entering neon_parse_command_args\n");

	int iopt = 0;
	/* first parameter is the name of the program -- skip */
	iopt++;	
	/* next parameter should be the architecture file name */
	g_user_opts.arch_file = convert_to_string(argv[iopt]);
	bool arch_is_xml = check_file_extension(g_user_opts.arch_file, convert_to_string(".xml"));
	if ( false == arch_is_xml ){
		stringstream ex_msg;
		ex_msg << "neon_parse_command_args: specified architecture file does not have .xml extension: "
			<< g_user_opts.arch_file;
		NEON_THROW(ex_msg, INIT);
	}

	/* next we read in the options */
	iopt++;
	for ( /*iopt++*/; iopt < argc; iopt++ ){
		if ( strcmp( argv[iopt], "-something" ) == 0 ){

		} else if ( strcmp(argv[iopt], "-something_else") == 0 ) {
		
		} else {
			stringstream ex_msg;
			ex_msg << "neon_parse_command_args: unrecognized command-line option: " << argv[iopt];
			NEON_THROW(ex_msg, INIT);
		}
	}

	print_debug("Exiting neon_parse_command_args\n");
	return;
}

/* Prints instructions for tool usage */
static void neon_print_usage(){
	print_debug("Entering neon_print_usage\n");
	print_info(PP_HIGH, "Program usage:\n" << "Placeholder message\n");
	print_debug("Exiting neon_print_usage\n");
}

/* Prints intro title for the tool */
static void neon_print_title(){
	print_info(PP_HIGH, "Placeholder Title\n\n");
}
