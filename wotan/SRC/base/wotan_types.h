#ifndef WOTAN_TYPES_H
#define WOTAN_TYPES_H

#include <iostream>
#include <vector>
#include <map>
#include "physical_types.h"
#include "wotan_util.h"


/**** Forward Declarations ****/
class Edge;
class Track_Info;


/**** Enums ****/
/* specifies whether given channel goes in the x or y directions */
enum e_chan_type{
	CHANX = 0,	/* horizontal channel */
	CHANY		/* vertical channel */
};

/* specifies the directionality of a track */
enum e_direction{
	BIDIR = 0,	/* can go in either direction */
	INC,		/* track goes in +ve direction */
	DEC		/* track goes in -ve direction */
};


/**** Classes ****/
/* a node on the routing graph -- represents a track */
class Node{
public:
	/* properties useful for drawing a node */
	e_chan_type track_chan_type;	/* type of channel this node is in */
	int track_length;		/* length of track this node represents */
	e_direction track_dir;		/* directionality of the track this node represents */
	int track_index;		/* index of track which this node represents */
	Coordinate track_start;		/* coordinate at which this track begins (i.e. lowest channel coordinate of track) */
	int track_start_group;		/* group at which track starts (usually 0, but not necessarily true if track is at perimeter) */
	//do we need to know at what coord WP 0 occurs?

	int weight;			/* some kind of weight for this graph node. could be based on track's capacitance */
	//Track_Info *start_segment;	/* pointer to the start segment of the corresponding track */

	
	int mark;			/* for routing */
	int track_usage;		/* for routing TODO comment */

	std::vector<Edge* > outgoing;	/* list of edges leaving this node */
	std::vector<Edge* > incoming;	/* list of edges arriving at this node */

	Node(){
		weight = 0;
		mark = -1;
		track_usage = -1;
	}

	/* checks if outgoing connection to specified node already exists */
	bool has_outgoing( Node *to_node ) const;
};

/* an edge on the routing graph -- represents a switch connecting two tracks */
class Edge{
public:
	int weight;			/* some kind of weight for this graph edge. could be based on switch capacitance */
	//Coordinate coord;		/* the physical coordiante of the corresponding switch */

	Node *from;			/* pointer to the source node */
	Node *to;			/* pointer to the destination node */


	/* specify which two points of the source/dest track are connected with this edge. 

	   conn points:  	 0      1 2      3 4      5 6      7
	   track:		x--------x--------x--------x--------x
	   wire points:		0        1        2        3        0

	   Basically the connection point denotes both where along the wire the connection
	   happens, and on which side of the switchblock it happens at. 
	   i.e. conn_point 3 represents the left side of wire point 2

	   can convert from conn point to wire point by
		wire_point = ceil(conn_point / 2) % L
	*/
	int from_conn_point;
	int to_conn_point;

	Edge(){
		weight = 0;
		from_conn_point = to_conn_point = 0;
	}

	Edge (Node *f, Node *t, int from_conn_pt, int to_conn_pt, int w){
		from = f;
		to = t;
		from_conn_point = from_conn_pt;
		to_conn_point = to_conn_pt;
		weight = w;
	}
};



/* contains information on a section of a track in a specific channel */
class Track_Info{
public:
	Coordinate coord;		/* coordinate of this track section */
	int group;			/* the group of this track section */
	int length;			/* length of the track */
	int index;			/* index of this track in the channel */
//	int group_index;		/* index of this track within its own group */
	e_direction dir;		/* directionality of this track */

	int type_start;			/* track index at which this track's type starts (inclusive) */
	int type_end;			/* track index at which this track's type ends (inclusive) */

	const char *type_name_ptr;	/* pointer to the name of the wire type this track segment belongs to */

	/* overload equals operator */
//	Track_Info & operator = (const Track_Info &obj);
};

/* a class for indexing into correct coordinate/channel/track index */ 
class Track_Coord{
public:
	Coordinate coord;
	e_chan_type chan_type;
	int track_index;
	
	Track_Coord(){
	}

	Track_Coord(Coordinate c, e_chan_type ch_type, int track_ind){
		coord = c;
		chan_type = ch_type;
		track_index = track_ind;
	}

	Track_Coord & operator = (const Track_Coord &obj);
	/* defined for purposes of indexing into an std::map */
	bool operator < (const Track_Coord &obj) const;
};
std::ostream& operator<<(std::ostream &os, const Track_Coord &track_coord);

/* a class for indexing into a channel within the FPGA test area */
class Chan_Coord{
public:
	Coordinate coord;
	e_chan_type chan_type;

	Chan_Coord(){
	}

	Chan_Coord(Coordinate c, e_chan_type ch_type){
		coord = c;
		chan_type = ch_type;
	}
	
	/* defined for purposes of indexing into an std::map */
	bool operator < (const Chan_Coord &obj) const;
};
std::ostream& operator<<(std::ostream &os, const Chan_Coord &chan_coord);

/**** Typedefs ****/
/* allows to easily look up info about a track at some coordinate and channel */
typedef std::map< Track_Coord, Track_Info > t_chan_map;
/* allows to easily look up a graph node based on a coordinate/channel/track.
   because a single node represents an entire length of track, we want multiple coordinates to be able to 
   point to the same node. */
typedef std::map< Track_Coord, Node* > t_node_map;
/* specifies how many tracks we have been able to route to the given channel */
typedef std::map< Chan_Coord, int > t_flood_results;
#endif
