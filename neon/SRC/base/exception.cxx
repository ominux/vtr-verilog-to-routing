#include "exception.h"

using namespace std;

/**** Globals ****/
const char *neon_exception_name[] = {
	"INIT",
	"ARCH"
};


/**** Function Declarations ****/
/* operator << override for the Neon_Exception class */
ostream &operator <<(ostream &stream, Neon_Exception obj){
	stream << "EXCEPTION(" << neon_exception_name[obj.type] <<  ") in " << obj.file << " at line " << obj.line << ". ";
	stream << "Exception message:\n" << obj.message << "\n";
	return stream;
}

/* Builds and throws a Neon_Exception based on the provided parameters */
void neon_throw(char *message, t_neon_exceptions type, char *file, int line){
	Neon_Exception ex(message, type, file, line);
	throw ex;
}
