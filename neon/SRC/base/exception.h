#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include "print.h"

/**** Defines ****/


/**** Enumerations ****/
/* NOTE: if you change this enum, modify neon_exception_name array in exception.cxx accordingly! */
typedef enum e_neon_exceptions{
	INIT = 0,
	ARCH
} t_neon_exceptions;
extern const char *neon_exception_name[]; 

/**** Function Definitions ****/
void neon_throw(char *message, t_neon_exceptions type, char *file, int line);


/**** Class Declarations ****/
/* The Neon Exception class contains basic info on the exception, including
*  line number, file name, and message. The inserter operator is overloaded to
*  allow printing to an ostream */
class Neon_Exception{
private:

public:
	const std::string exception_name;
	
	t_neon_exceptions type;
	std::string message;
	std::string file;
	int line;

	/* Constructor */
	Neon_Exception(char *m, t_neon_exceptions t, char *f, int l) : 
	exception_name("Neon Exception")
	{
 		this->message = char_to_string(m);
		this->type = t;
		this->file = char_to_string(f);
		this->line = l;
	}
};
/* prototype for Neon_Exception's << overloaded operator */
std::ostream &operator <<(std::ostream &stream, Neon_Exception obj);




#endif /*EXCEPTION_H*/
