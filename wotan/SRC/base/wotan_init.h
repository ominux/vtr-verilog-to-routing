#ifndef WOTAN_INIT_H
#define WOTAN_INIT_H

#include <string>
#include "wotan_types.h"

/**** Class Declarations ****/
/* Class containing user-settable options */
class User_Options {
public:
	std::string arch_file;	/* path to architecture file */
	
	//path to constraints file
	
	//various user options which may have been read out from constraints file

	int x;					/* x dimension of test area */
	int y;					/* y dimension of test area */
	int W;					/* channel width */
	e_directionality directionality;	/* directionality of this architecture. there can only be one! */

	bool shift_chan_with_coord;		/* shift the channel based on the coordinate (VPR does this) */
};


/**** Function Declarations ****/
/* Reads in user options and initializes the tool */
void wotan_init(INP int argc, INP char **argv);
/* gets directionality of the architecture */
e_directionality get_directionality(INP s_arch *arch);



#endif /* WOTAN_INIT_H */
