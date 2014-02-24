#ifndef WOTAN_IO_H
#define WOTAN_IO_H


#include <iostream>
#include <string>
#include <sstream>
#include "wotan_types.h"

/**** Defines ****/
#ifndef DEBUG_LEVEL
	#define DEBUG_LEVEL PP_MED
#endif

/* checks print priority */
#define CHECK_PP( pp ) (pp >= DEBUG_LEVEL)

/* prints string */
#define W_PRINT(str) do{ if( CHECK_PP(PP_HIGH) ) std::cout << str; } while(false)

/* prints string, checking print priority */
#define W_PRINT_PRIORITY(pp, str) do{ if( CHECK_PP(pp) ) std::cout << str; } while(false)

/* prints string preceded by WARNING */
#define W_WARN(str) do{ if( CHECK_PP(PP_HIGH) ) std::cout << "WARNING: " << str; } while(false)


/**** Enumerations ****/

/* Used to specify the debug level of a message where level PR_HIGH corresponds
*  to highest priority */
enum e_print_priority{
	PP_LOW = 0,
	PP_MED,
	PP_HIGH		
};

/**** Function Declarations ****/
/* returns true if extension of 'filename' matches 'extension' */
bool check_file_extension(INP std::string &filename, INP std::string extension);
/* function for opening files, with error checking */
void open_file(std::fstream *file, std::string path, std::ios_base::openmode mode);

/* Overloaded methods to convert specified type to string */
std::string convert_to_string(INP char *ch);
std::string convert_to_string(INP char const *ch);
std::string convert_to_string(INP std::stringstream &ss);
std::string convert_to_string(INP std::string &str);    //useful when called by template f'n

#endif
