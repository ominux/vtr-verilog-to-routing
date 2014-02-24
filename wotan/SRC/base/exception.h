#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include <sstream>

/**** Defines ****/
/* Macro for throwing general-purpose exceptions. Auto-includes file and line number.
   Used (almost) the way you'd use a stream. As in:
	W_THROW("some text" << my_obj << 123 << endl); */
#define W_THROW(message) do {std::stringstream ss; ss << message;					\
			Wotan_Exception ex(ss.str(), __FILE__, __LINE__, EX_G);			\
			throw ex;} while(false) 
/* Same as W_THROW, but exception type explicitly specified */
#define W_THROW_TYPE(message, ex_type) do {std::stringstream ss; ss << message;			\
				Wotan_Exception ex(ss.str(), __FILE__, __LINE__, ex_type);		\
				throw ex;} while(false) 

/* for converting to nearest integer */
#define NINT(num) (int)(num+0.5)

/**** Enums ****/
/* enumerate various exceptions */
enum e_ex_type{
	EX_G = 0,		/* general-purpose; should be used for almost everything */
	EX_USER_EXIT		/* user pressed exit, or something similar */
};

/* Classes */
/* Basic exception class. Includes message, file name, and line number */
class Wotan_Exception{
	public:
	std::string message;
	int line;
	std::string file;
	e_ex_type type;

	/* Couple of constructors for different params */
	Wotan_Exception(char *m, char *f, int l, e_ex_type t){
		std::stringstream ss_m, ss_f;
		ss_m << m; ss_f << f;
		message = ss_m.str();
		file = ss_f.str();
		line = l;
		type = t;
	}
	Wotan_Exception(std::string m, char *f, int l, e_ex_type t){
		std::stringstream ss_f;
		ss_f << f;
		message = m;
		file = ss_f.str();
		line = l;
		type = t;
	}
};
/* overlaod << operator for use by cout */
std::ostream& operator<<(std::ostream &os, const Wotan_Exception& ex);

#endif /*EXCEPTION_H*/
