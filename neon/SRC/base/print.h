#ifndef PRINT_H
#define PRINT_H

#include <iostream>

class Neon_Error : public std::ostream{
	//nothing yet...
};
template<class T> Neon_Error& operator <<(Neon_Error &stream, T &data){
	std::cout << data;
	//TODO: print to some log file?
	return stream;
}
extern Neon_Error _neon_error;
#define neon_error _neon_error << "ERROR: In file: " << __FILE__ << "  function: " << __func__ << "\n"


#endif /*PRINT_H*/
