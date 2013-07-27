#include <iostream>
#include <string>
#include <sstream>
#include "exception.h"
#include "print.h"

using namespace std;

/**** Defines ****/

/**** Function Declarations ****/


/**** Function Definitions ****/
/* uses stringstream to convert char array to string */
string char_to_string(INP char *ch){
	stringstream ss;
	ss << ch;

	string result = ss.str();
	return result;
}

