
#include <fstream>
#include "io.h"
#include "exception.h"

using namespace std;


/**** Function Definitions ****/
/* compares extension of specified filename with specified string
*  returns true if they are the same, false otherwise */
bool check_file_extension(INP string &filename, INP string extension){

	bool result = false;
	int ext_len = extension.length();
	int file_len = filename.length();

	/* check if either string is empty */
	if ( 0 == file_len || 0 == ext_len ){ 
		result = false;
	} else if (file_len <= ext_len){
		result = false;
	} else {
		string file_ext = filename.substr( file_len - ext_len, file_len - 1 );
		if ( file_ext.compare( extension ) == 0 ){
			/* compare returns 0 if the two strings are the same */
			result = true;
		} else {
			result = false;
		}
	}
	return result;
}

/* open file with error checking. 'mode' is ios::in, ios::out, or (ios::in | ios::out)
   ios::in is used to write things to file, ios::out is for read */
void open_file(fstream *file, string path, ios_base::openmode mode){
	if (NULL == file){
		W_THROW("Specified file pointer is null!");
	}	

	file->close();
	file->open(path.c_str(), mode);
	if (file->is_open() && file->good()){
		cout << "Opened file: " << path << endl;
	} else {
		W_THROW("Could not open file: " << path << "\nDoes it exist?");
		
	}
}


/* Various overloaded methods to convert given type/class to string */
string convert_to_string(INP char *ch){         //overloaded for char*
        stringstream ss;
        ss << ch;

        string result = ss.str();
        return result;
}
string convert_to_string(INP char const *ch){   //overloaded for char const*
        stringstream ss;
        ss << ch;

        string result = ss.str();
        return result;
}
string convert_to_string(INP stringstream &ss){ //overloaded for stringstream
        string result = ss.str();
        return result;
}
string convert_to_string(INP string &str){      //overloaded for string; useful when called by template f'n
        string result = str;
        return result;
}

