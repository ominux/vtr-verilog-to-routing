#include <string>
#include <sstream>
#include "exception.h"
#include "print.h"

using namespace std ;

/**** Globals ****/
const char *neon_exception_name[] = {
	"INIT",
	"ARCH"
};

/**** Function Declarations ****/




/**** Function Definitions ****/
/* operator << override for the Neon_Exception class */
ostream &operator <<(ostream &stream, Neon_Exception obj){
	stream << "EXCEPTION(" << neon_exception_name[obj.type] <<  ") in " << obj.file << " at line " << obj.line << ". ";
	stream << "Exception message:\n" << obj.message << "\n";
	return stream;
}

///* Builds and throws a Neon_Exception based on the provided parameters */
// MOVED to header file since compiler doesn't like templates in cxx files
//template <typename T> void  neon_throw(T message, t_neon_exceptions type, char *file, int line){
//	string str = convert_to_string(message);	
//	Neon_Exception ex(str.c_str(), type, file, line);
//	throw ex;
//}

