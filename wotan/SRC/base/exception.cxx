#include <string>
#include <sstream>
#include "exception.h"

using namespace std ;

/**** Class Function Definitions ****/
/*==== Wotan_Exception Class ====*/
/* Overload the << operator for the base exception class */
ostream& operator<<(ostream &os, const Wotan_Exception& ex){
	os << "\nEXCEPTION: In " << ex.file << " on line " << ex.line << ". Message:\n" << ex.message << endl;
	return os;
}
/*=== End Wotan_Exception Class ===*/
