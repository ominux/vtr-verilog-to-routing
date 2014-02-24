
#include "io.h"
#include "exception.h"
#include "wotan_types.h"

using namespace std;



/**** Class Function Definitions ****/
/*==== Track_Info Class ====*/
/* overload equals operator */
//Track_Info & Track_Info::operator = (const Track_Info &obj){
//	if (this == &obj){
//		/* nothing for self-assignment */
//	} else {
//		coord = obj.coord;
//		group = obj.group;
//		length = obj.length;
//		index = obj.index;
//		dir = obj.dir;
//		type_start = obj.type_start;
//		type_end = obj.type_end;
//	}
//	return *this;
//}
/*==== End Track_Info Class ====*/


/*==== Node Class ====*/
/* checks if outgoing connection to specified node already exists */
bool Node::has_outgoing( Node* to_node ) const{
	bool exists = false;
	vector< Edge* >::const_iterator it;
	for (it = this->outgoing.begin(); it != this->outgoing.end(); it++){
		if ((*it)->to == to_node){
			exists = true;
			break;
		}
	}
	return exists;
}
/*==== End Node Class ====*/

/*==== Track_Cooord Class ====*/
Track_Coord & Track_Coord::operator = (const Track_Coord &obj){
	if (this == &obj){
		/* do nothing in case of self-assignment */
	} else {
		coord = obj.coord;
		chan_type = obj.chan_type;
		track_index = obj.track_index;
	}
	return *this;
}

/* for indexing into an std::map */
bool Track_Coord::operator < (const Track_Coord &obj) const{
	bool result;
	if (coord < obj.coord){
		result = true;
	} else {
		if (coord == obj.coord){
			if ((int)chan_type < (int)obj.chan_type){
				result = true;
			} else {
				if ((int)chan_type == (int)obj.chan_type){
					if (track_index < (int)obj.track_index){
						result = true;
					} else {
						result = false;
					}
				} else {
					result = false;
				}
			}
		} else {
			result = false;
		}
	}

	return result;
}

ostream& operator<<(std::ostream &os, const Track_Coord &track_coord){
	os << "[chan type: " << track_coord.chan_type << ", coord: " << track_coord.coord << ", track index: " << track_coord.track_index << "]";
	return os;
}
/*==== END Track_Coord Class ====*/

/*==== Chan_Coord Class ====*/
/* defined for purposes of indexing into an std::map */
bool Chan_Coord::operator < (const Chan_Coord &obj) const{
	bool result;
	if (coord < obj.coord){
		result = true;
	} else {
		if (coord == obj.coord){
			if ((int)chan_type < (int)obj.chan_type){
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

ostream& operator<<(std::ostream &os, const Chan_Coord &chan_coord){
	os << "[chan_type: " << chan_coord.chan_type << ", coord: " << chan_coord.coord << "]";
	return os;
}
/*==== END Chan_Coord Class ====*/
