#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include <string>
#include "neon_types.h"

/**** Defines ****/
#ifndef DEBUG_LEVEL
	#define DEBUG_LEVEL 0
#endif

/* checks display priority */
#define CHECK_PP( pp ) (pp >= DEBUG_LEVEL)

/* prints string preceded by ERROR */
#define print_error(str) do{ std::cout << "\nERROR: " << str << endl; } while(false)

/* prints string, check print priority */
#define print_info(pp, str) do{ if( CHECK_PP(pp) ) std::cout << str; } while(false)

/* overloads print_info above to only take str -- priority is automatically set to low */
#define print_debug(str) print_info(PP_LOW, str)

/* prints string preceded by WARNING, checks print priority */
#define print_warn(pp, str) do{ if( CHECK_PP(pp) ) std::cout << "WARNING: " << str; } while(false)


/**** Enumerations ****/

/* Used to specify the debug level of a message where level PR_HIGH corresponds
*  to highest priority */
enum e_print_priority{
	PP_LOW = 0,
	PP_MED,
	PP_HIGH		
};


/**** Function Definitions ****/
/* Convert specified character array to string */
std::string char_to_string(INP char *ch);


#endif /*PRINT_H*/
