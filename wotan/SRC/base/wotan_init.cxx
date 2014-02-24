#include <cstdlib>
#include <cstring>
#include <sstream>
#include "wotan_init.h"
#include "globals.h"
#include "exception.h"
#include "io.h"
#include "read_xml_arch_file.h" /* taken from VPR -- is VPR specific in some cases */
#include "physical_types.h"

using namespace std;

/**** Function Declarations ****/ 
static void wotan_parse_command_args(INP int argc, INP char **argv, INOUTP User_Options *user_opts);
static void wotan_print_usage();
static void wotan_print_title();
 



/**** Function Definitions ****/

/* Reads in architecture file, as well as user options */
void wotan_init(INP int argc, INP char **argv){

	wotan_print_title();

	/* check that we have the minimum number of arguments */
	if (argc < 2){
		wotan_print_usage();
		exit( EXIT_FAILURE );
	}

	/* parse user-specified options into global g_user_opts variable */
	wotan_parse_command_args(argc, argv, &g_user_opts);

	/* initialize arch to 0 */
	memset(&g_arch, 0, sizeof(t_arch));

	/* load the specified architecture */
	boolean timing_enabled = TRUE;
	int num_types;
	s_type_descriptor *type_descriptors = NULL;
	/* note, in the future XmlReadArch will start throwing VPR exceptions */
	XmlReadArch(g_user_opts.arch_file.c_str(), timing_enabled, &g_arch, &type_descriptors, &num_types);
	W_PRINT("Read in architecture: " << g_user_opts.arch_file << "\n");

	/* set directionality defined by architecture */
	g_user_opts.directionality = get_directionality(&g_arch);
	g_user_opts.shift_chan_with_coord = true;	//TODO


	return;
}

/* gets directionality of the architecture */
e_directionality get_directionality(INP s_arch *arch){
	e_directionality result;
	result = arch->Segments[0].directionality;
	return result;
}

/* Parses the command line options 
   Options are parsed into the user_opts variable */
static void wotan_parse_command_args(INP int argc, INP char **argv, INOUTP User_Options *user_opts){

	int iopt = 0;
	/* first parameter is the name of the program -- skip */
	iopt++;	
	/* next parameter should be the architecture file name */
	user_opts->arch_file = convert_to_string(argv[iopt]);
	bool arch_is_xml = check_file_extension(user_opts->arch_file, convert_to_string(".xml"));
	if ( false == arch_is_xml ){
		W_THROW("specified architecture file does not have .xml extension: "
			   << user_opts->arch_file );
	}

	/* next we read in the options */
	iopt++;
	for ( /*iopt++*/; iopt < argc; iopt++ ){
		if ( strcmp( argv[iopt], "-chan_width" ) == 0 ){
			iopt++;
			user_opts->W = atoi(argv[iopt]);
		} else if ( strcmp(argv[iopt], "-size") == 0 ) {
			iopt++;
			user_opts->x = user_opts->y = atoi(argv[iopt]);
		} else {
			W_THROW("unrecognized command-line option: " << argv[iopt]);
		}
	}

	return;
}

/* Prints instructions for tool usage */
static void wotan_print_usage(){
	W_PRINT("Program usage:\n" << "Placeholder message\n");
}

/* Prints intro title for the tool */
static void wotan_print_title(){
	W_PRINT("GARMITO PAR\n\n");
}
