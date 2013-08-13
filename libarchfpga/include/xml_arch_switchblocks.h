#ifndef XML_ARCH_SWITCHBLOCKS_H
#define XML_ARCH_SWITCHBLOCKS_H

/**** Defines ****/

/**** Structs ****/
struct s_formula_data{
	int dest_W; 	/* W in destination channel */
	int track;	/* incoming track index */
	int Fs;
	int Fc_in;
	int Fc_out;

	/* Initialize all members to 0 */
	s_formula_data(){ memset(this, 0, sizeof(s_formula_data)); }
};


/**** Classes ****/

/**** Function Declarations ****/
/* returns integer result according to the specified formula and data */
int get_sb_formula_result( INP const char* formula, INP const s_formula_data &mydata );



#endif /* XML_ARCH_SWITCHBLOCKS_H */
