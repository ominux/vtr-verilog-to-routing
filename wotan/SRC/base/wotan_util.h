#ifndef WOTAN_UTIL_H
#define WOTAN_UTIL_H

#include <iostream>

/**** Classes ****/

/* represents a coordinate on a grid */
class Coordinate{
public:
	int x;
	int y;
	Coordinate(){
		x=y=0;
	}
	Coordinate(int s_x, int s_y){
		x=s_x; y=s_y;
	}
	/* Sets coordinate */
	void set(int s_x, int s_y);
	/* gets hypotenuse size between this and another coordinate */
	double get_dist(const Coordinate &obj);
	/* returns the absolute difference between x coordinates plus the absolute difference between the y coordinates i.e. dx+dy */
	int get_dx_plus_dy(const Coordinate &obj);
	/* defined for purposes of indexing into an std::map */
	bool operator < (const Coordinate &obj) const;
	/* compares this and another coordinate */
	bool operator== (const Coordinate &obj) const;
	/* overload equals operator */
	//Coordinate & operator= (const Coordinate &obj);
};
std::ostream& operator<<(std::ostream &os, const Coordinate &coord);




#endif
