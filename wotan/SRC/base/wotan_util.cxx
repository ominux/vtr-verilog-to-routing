
#include <cmath>
#include "wotan_util.h"

using namespace std;



/**** Class Function Definitions ****/
/*==== Coordinate Class====*/
/* Sets coordinate */
void Coordinate::set(int s_x, int s_y){
	x=s_x; y=s_y;
}
/* gets hypotenuse size between this and another coordinate */
double Coordinate::get_dist(const Coordinate &obj){
	double dist = std::sqrt( std::pow((double)(x-obj.x),2.0) + std::pow((double)(y-obj.y),2.0) );
	return dist;
}
/* returns the absolute difference between x coordinates plus the absolute difference between the y coordinates i.e. dx+dy */
int Coordinate::get_dx_plus_dy(const Coordinate &obj){
	int dx_plus_dy = abs(x - obj.x) + abs(y - obj.y);
	return dx_plus_dy;
}
/* defined for purposes of indexing into an std::map */
bool Coordinate::operator < (const Coordinate &obj) const{
	bool result;

	if (x < obj.x){
		result = true;
	} else {
		if (x == obj.x){
			if (y < obj.y){
				result = true;
			} else {
				result = false;
			}
		} else {
			result = false;
		}
	}
	return result;
}
/* compares this and another coordinate */
bool Coordinate::operator== (const Coordinate &obj) const{
	bool result;
	if (x == obj.x && y == obj.y){
		result = true;
	} else {
		result = false;
	}
	return result;
}
/* For printing Coordinate class */
std::ostream& operator<<(std::ostream &os, const Coordinate &coord){
	os << "(" << coord.x << "," << coord.y << ")";
	return os;
}
///* overload equals operator */
//Coordinate & Coordinate::operator= (const Coordinate &obj){
//	if (this == &obj){
//		/* do nothing for self-assignment */
//	} else {
//		x = obj.x;
//		y = obj.y;
//	}
//	return *this;
//}
/*=== End Coordinate Class ===*/
/**** END Class Function Definitions ****/

