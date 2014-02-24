#ifndef ROUTE_H
#define ROUTE_H

#include "wotan_init.h"
#include "wotan_types.h"


/**** Function Declarations ****/
/* starts at some node and uses a maze router to 'flood' the routing area */
void perform_and_analyze_flood_fill(INP User_Options user_opts, INP t_node_map *node_map, 
		INOUTP t_flood_results *flood_results);




#endif
