#include <iostream>
#include <string>
#include "print.h"
#include "raii_types.h"
#include "exception.h"
#include "neon_api.h"
#include "globals.h"
#include "../../libarchfpga/include/util.h"

using namespace std;


int main(int argc, char **argv){
	print_debug("Entering main\n");
	try{
		neon_init(argc, argv);

	} catch (Neon_Exception &ex){
		print_error(ex);
	}


	/*
	run init function - goes into neon_api.c
	- print welcome message
	- run init function
		- read in architecture name and options
		- read in architecture!
		- run results through a checker to see if what we have is valid
	*/

	print_debug("Exiting main\n");
	return 0;
}
