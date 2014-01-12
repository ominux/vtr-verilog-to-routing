#ifndef XML_ARCH_SWITCHBLOCKS_H
#define XML_ARCH_SWITCHBLOCKS_H

/**** Includes ****/
#include "ezxml.h"

/**** Enums ****/

/**** Structs ****/
/* contains data passed in to the switchblock parser */
struct s_formula_data{
	int dest_W; 	/* W in destination channel */
	int track;	/* incoming track index */
	int Fs;
	int Fc_in;
	int Fc_out;
};



/**** Function Declarations ****/
/* Loads permutation funcs specified under Node into t_switchblock_inf */
void read_sb_switchfuncs( INP ezxml_t Node, INOUTP t_switchblock_inf *sb );

/* Reads-in the wire connections specified for the switchblock in the xml arch file */
void read_sb_wireconns( INP ezxml_t Node, INOUTP t_switchblock_inf *sb );

/* returns integer result according to the specified formula and data */
int get_sb_formula_result( INP const char* formula, INP const s_formula_data &mydata );



#endif /* XML_ARCH_SWITCHBLOCKS_H */
