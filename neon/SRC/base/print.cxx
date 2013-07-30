#include <iostream>
#include <string>
#include <sstream>
#include "exception.h"
#include "print.h"

using namespace std;

/**** Defines ****/

/**** Function Declarations ****/


/**** Function Definitions ****/
/* Various overloaded methods to convert given type/class to string */
string convert_to_string(INP char *ch){		//overloaded for char*
	stringstream ss;
	ss << ch;

	string result = ss.str();
	return result;
}
string convert_to_string(INP char const *ch){	//overloaded for char const*
	stringstream ss;
	ss << ch;

	string result = ss.str();
	return result;
}
string convert_to_string(INP stringstream &ss){	//overloaded for stringstream
	string result = ss.str();
	return result;
}
string convert_to_string(INP string &str){	//overloaded for string; useful when called by template f'n
	string result = str;
	return result;
}


