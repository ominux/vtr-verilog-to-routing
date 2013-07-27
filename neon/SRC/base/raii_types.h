#ifndef RAII_TYPES_H
#define RAII_TYPES_H

#include "print.h"

//this is pointless because C++ has the vector type, which is much cleaner 
//template<class T> class RAII_Array{
//	public:
//
//	T *ptr;
//	
//	inline RAII_Array( int size ){
//		try{
//			ptr = new T[size];
//		} catch (std::bad_alloc &e) {
//			//error
//		}
//	}
//
//	inline ~RAII_Array(){
//		delete [] ptr;
//	}
//
//};


#endif /*RAII_TYPES_H*/
