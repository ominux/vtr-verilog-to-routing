#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include "print.h"

/**** Defines ****/
/* function for simplifying throwing method so that file & line info is automatically entered for us */
#define NEON_THROW( message, type ) neon_throw(message, type, __FILE__, __LINE__)

/**** Enumerations ****/
/* NOTE: if you change this enum, modify neon_exception_name array in exception.cxx accordingly! */
typedef enum e_neon_exceptions{
	INIT = 0,
	ARCH
} t_neon_exceptions;
extern const char *neon_exception_name[]; 



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

	/* Constructor for chars */
	Neon_Exception(std::string m, t_neon_exceptions t, char *f, int l) : 
	exception_name("Neon Exception")
	{
 		this->message = m;
		this->type = t;
		this->file = convert_to_string(f);
		this->line = l;
	}

	
};
/* prototype for Neon_Exception's << overloaded operator */
std::ostream &operator <<(std::ostream &stream, Neon_Exception obj);



/**** Function Declarations ****/
template <typename T> void  neon_throw(T &message, t_neon_exceptions type, char *file, int line);


/**** Function Definitions ****/
/* Builds and throws a Neon_Exception based on the provided parameters */
//sadly can't put templates in cxx file as compiler doesn't see definition at the same time as function is used... wonky
template <typename T> void  neon_throw(T &message, t_neon_exceptions type, char *file, int line){
	std::string str = convert_to_string(message);	
	Neon_Exception ex(str.c_str(), type, file, line);
	throw ex;
}

#endif /*EXCEPTION_H*/
