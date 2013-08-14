/* 
===== Description =====
The contents of this file assist in parsing switchblock descriptions specified
in the XML architecture file.

A large chunk of this file is dedicated to helping parse the initial switchblock
specificaiton in the XML arch file, providing error checking, etc.

Another large chunk of this code is dedicated to parsing the actual formulas 
specified by the switchblock permutation functions into their numeric counterparts.
*/


#include <string.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <utility>
#include "util.h"
#include "read_xml_util.h"
#include "arch_types.h"
#include "physical_types.h"
#include "parse_switchblocks.h"

using namespace std;

/**** Defines ****/

/**** Enums ****/
typedef enum e_formula_obj{
	E_FML_UNDEFINED = 0,
	E_FML_NUMBER,
	E_FML_BRACKET,
	E_FML_OPERATOR,
	E_FML_NUM_FORMULA_OBJS
} t_formula_obj;

typedef enum e_operator{
	E_OP_UNDEFINED = 0,
	E_OP_ADD,
	E_OP_SUB,
	E_OP_MULT,
	E_OP_DIV,
	E_OP_NUM_OPS
} t_operator;

/**** Class Definitions ****/
/* This class is used to represent an object in a formula, such as 
   a number, a bracket, an operator, or a variable */
class Formula_Object{
public:
	/* indicates the type of formula object this is */
	t_formula_obj type;

	/* object data, accessed based on what kind of object this is */
	union u_Data {
		int num;		/*for number objects*/
		t_operator op;		/*for operator objects*/
		bool left_bracket;	/*for bracket objects -- specifies if this is a left bracket*/

		u_Data(){ memset(this, 0, sizeof(u_Data)); }
	} data;

	Formula_Object(){
		this->type = E_FML_UNDEFINED;
	}
	
};



/**** Function Declarations ****/
/* converts specified formula to a vector in reverse-polish notation */
static void formula_to_rpn( INP const char* formula, INP const s_formula_data &mydata, 
				INOUTP vector<Formula_Object> &rpn_output );

static void get_formula_object( INP const char *ch, INOUTP int &ichar, INP const s_formula_data &mydata,
				 INOUTP Formula_Object *fobj );

/* returns integer specifying precedence of passed-in operator. higher integer 
   means higher precedence */
static int get_fobj_precedence( INP const Formula_Object &fobj );

/* Returns associativity of the specified operator */
static bool op_associativity_is_left( INP const t_operator &op );

/* used by the shunting-yard formula parser to deal with operators such as add and subtract */
static void handle_operator( INP const Formula_Object &fobj, INOUTP vector<Formula_Object> &rpn_output,
				INOUTP stack<Formula_Object> &op_stack);

/* used by the shunting-yard formula parser to deal with brackets, ie '(' and ')' */
static void handle_bracket( INP const Formula_Object &fobj, INOUTP vector<Formula_Object> &rpn_output,
				INOUTP stack<Formula_Object> &op_stack);

/* parses revere-polish notation vector to return formula result */
static int parse_rpn_vector( INOUTP vector<Formula_Object> &rpn_vec );

/* applies operation specified by 'op' to the given arguments. arg1 comes before arg2 */
static int apply_rpn_op( INP const Formula_Object &arg1, INP const Formula_Object &arg2, 
					INP const Formula_Object &op );

/* checks if specified character represents an ASCII number */
static bool is_char_number( INP const char &ch );



/**** Function Definitions ****/

/* Reads-in the wire connections specified for the switchblock in the xml arch file */
void read_sb_wireconns( INP ezxml_t Node, INOUTP t_switchblock_inf *sb ){
	
	/* Make sure that Node is a switchblock */
	CheckElement(Node, "switchblock");
	
	int num_wireconns;
	ezxml_t SubElem;
	const char *char_prop;
	int int_prop;	

	/* count the number of specified wire connections for this SB */
	num_wireconns = CountChildren(Node, "wireconn", 1);
	
	/* allocate the wireconn list for the SB */
	sb->num_wireconns = num_wireconns;
	sb->wireconns = (t_wireconn_inf *) my_malloc( num_wireconns * sizeof(t_wireconn_inf) );

	for (int i = 0; i < num_wireconns; i++){
		SubElem = ezxml_child(Node, "wireconn");

		/* get first wire type */
		char_prop = FindProperty(SubElem, "T1", TRUE);
		sb->wireconns[i].type1 = my_strdup(char_prop);
		ezxml_set_attr(SubElem, "T1", NULL);

		/* get second wire type */
		char_prop = FindProperty(SubElem, "T2", TRUE);
		sb->wireconns[i].type2 = my_strdup(char_prop);
		ezxml_set_attr(SubElem, "T2", NULL);

		/* get first wire group */
		int_prop = GetIntProperty(SubElem, "G1", TRUE, -1);
		sb->wireconns[i].group1 = int_prop;
		ezxml_set_attr(SubElem, "G1", NULL);

		/* get first wire group */
		int_prop = GetIntProperty(SubElem, "G2", TRUE, -1);
		sb->wireconns[i].group2 = int_prop;
		ezxml_set_attr(SubElem, "G2", NULL);

		FreeNode(SubElem);
	}

	return;
}

/* Loads permutation funcs specified under Node into t_switchblock_inf. Node should be 
   <switchfuncs> */
void read_sb_switchfuncs( INP ezxml_t Node, INOUTP t_switchblock_inf *sb ){
	
	/* Make sure the passed-in is correct */
	CheckElement(Node, "switchfuncs");
	
	ezxml_t SubElem;

	/* Make structure that pairs switch going in one direction with a switch going in other direction.
	   This is useful for the bidir case where only 6 of the 12 permutation functions are specified.
	   Each element of a pair is a pointer to a vector in the switchblock struct that specifies the permutation
	   function(s) for that particular side->side connection. */
	pair< vector<const char*> *, vector<const char*> * > func_pairs[6] = 
					{
						make_pair( &(sb->lr), &(sb->rl) ), 
						make_pair( &(sb->lt), &(sb->tl) ), 
						make_pair( &(sb->lb), &(sb->bl) ), 
						make_pair( &(sb->rt), &(sb->tr) ), 
						make_pair( &(sb->rb), &(sb->br) ), 
						make_pair( &(sb->bt), &(sb->tb) )
					};
	/* an enumeration to help access the above array of pairs */
	enum{
		E_LR = 0,
		E_LT,
		E_LB,
		E_RT,
		E_RB,
		E_BT,
		E_NUM_FUNC_PAIRS
	};

	/* Used to designate if a predifined function such as 'wilton' has been found.
	   If a predifined function is specified, only one function entry is allowed */
	bool predefined_sb_found = false;

	/* is switchblock bidirectional? */
	bool is_bidir = (BI_DIRECTIONAL == (*sb).directionality);
	
	/* get the number of specified permutation functions */
	int num_funcs = CountChildren(Node, "func", 1);

	const char * func_type;
	const char * func_formula;
	vector<const char *> * func_ptr;

	/* now we iterate through all the specified permutation functions, and 
	   load them into the switchblock structure as appropriate */
	for (int ifunc = 0; ifunc < num_funcs; ifunc++){
		/* get the next switchblock function */
		SubElem = ezxml_child(Node, "func");
		/* get function type */
		func_type = FindProperty(SubElem, "type", TRUE);
		if (!func_type){
			vpr_printf(TIO_MESSAGE_ERROR, "empty function specification in switchblock\n");
			exit(1);
		}
		ezxml_set_attr(SubElem, "type", NULL);
		/* get function formula */
		func_formula = FindProperty(SubElem, "formula", TRUE);
		if (!func_formula){
			vpr_printf(TIO_MESSAGE_ERROR, "empty formula specification in switchblock\n");
			exit(1);
		}
		ezxml_set_attr(SubElem, "formula", NULL);


		/* a predefined function should be the only entry */
		if (predefined_sb_found && ifunc > 0){
			vpr_printf(TIO_MESSAGE_ERROR, "Predefined switchblock function should be the only entry in the switchfuncs section. Other entry found: %s = %s\n", func_type, func_formula);
			exit(1);
		}

		/* go through all the possible cases of func_type */
		if (0 == strcmp(func_type, "lt")){
			func_ptr = func_pairs[E_LT].first;
		} else if (0 == strcmp(func_type, "lr")) {
			func_ptr = func_pairs[E_LR].first; 
		} else if (0 == strcmp(func_type, "lb")) {
			func_ptr = func_pairs[E_LB].first; 
		} else if (0 == strcmp(func_type, "tl")) {
			func_ptr = func_pairs[E_LT].second; 
		} else if (0 == strcmp(func_type, "tb")) {
			func_ptr = func_pairs[E_BT].second; 
		} else if (0 == strcmp(func_type, "tr")) {
			func_ptr = func_pairs[E_RT].second; 
		} else if (0 == strcmp(func_type, "rt")) {
			func_ptr = func_pairs[E_RT].first; 
		} else if (0 == strcmp(func_type, "rl")) {
			func_ptr = func_pairs[E_LR].second; 
		} else if (0 == strcmp(func_type, "rb")) {
			func_ptr = func_pairs[E_RB].first; 
		} else if (0 == strcmp(func_type, "bl")) {
			func_ptr = func_pairs[E_LB].second; 
		} else if (0 == strcmp(func_type, "bt")) {
			func_ptr = func_pairs[E_BT].first; 
		} else if (0 == strcmp(func_type, "br")) {
			func_ptr = func_pairs[E_RB].second; 
		} else if (0 == strcmp(func_type, "predefined")){
			/* a predifined permutation function */
			predefined_sb_found = true;
		} else {
			/* unknown permutation function */
			vpr_printf(TIO_MESSAGE_ERROR, "Unknown permutation function specified: %s\n", func_type);
			exit(1);
		}

		/* Here we load the specified switch function(s) */
		if (predefined_sb_found){
			//TODO: load_predefined_switchfuncs( const char *predef_switch, ...? );
		} else {
			func_ptr->push_back( my_strdup(func_formula) );
		}
	
		func_ptr = NULL;
		FreeNode(SubElem);
	}
	
	/* Here we check the completeness of the specified switchblock.
	   In the bidirectional case, only 6 of the 12 functions have to be specified 
	   therefore we have to check that only one entry in each func pair has been
	   specified. For the unidirectional case, all 12 funcs must be specified*/
	/* In the bidir case, the remaining 6 funcs are filled in later */
	if (!predefined_sb_found){
		for (int ifunc = 0; ifunc < E_NUM_FUNC_PAIRS; ifunc ++){
			if (func_pairs[ifunc].first->size() && func_pairs[ifunc].second->size()){
				if (is_bidir){
					/* only one pair entry can have size > 0 */
					vpr_printf(TIO_MESSAGE_ERROR, "Bidir switchblock specification incomplete\n");
					exit(1);
				} else {
					//Correct. Nothing to do.
				}
			} else {
				if (is_bidir){
					/* Bidir case doesn't have the required 6 functions specified */
					if ( func_pairs[ifunc].first->empty() && func_pairs[ifunc].second->empty() ){
						vpr_printf(TIO_MESSAGE_ERROR, "Required switch is not specified in bidir switchblock\n");
						exit(1);
					}
				} else {
					/* Unidir case doesn't have all 12 functions specified */
					vpr_printf(TIO_MESSAGE_ERROR, "Not all 12 permutation functions specified for unidirectional switchblock.\n");
					exit(1);
				}
			}
		}
	}

	return;
} /* read_sb_switchfuncs */












/* returns integer result according to the specified formula and data */
int get_sb_formula_result( INP const char* formula, INP const s_formula_data &mydata ){
	/* the result of the formula will be an integer */
	int result = -1;

	/* check formula */
	if (NULL == formula){
		vpr_printf(TIO_MESSAGE_ERROR, "in get_sb_formula_result: SB formula pointer NULL\n");
		exit(1);
	} else if ('\0' == formula[0]){
		vpr_printf(TIO_MESSAGE_ERROR, "in get_sb_formula_result: SB formula empty\n");
		exit(1);
	}
	
	vector<Formula_Object> rpn_output;	/* output in reverse-polish notation */

	/* now we have to run the shunting-yard algorithm to convert formula to reverse polish notation */
	formula_to_rpn( formula, mydata, OUTP rpn_output );
	
	/* then we run an RPN parser to get the final result */
	result = parse_rpn_vector(rpn_output);

	printf("result: %d\n\n", result);
	
	return result;
}


/* Parses the specified formula using a shunting yard algorithm (see wikipedia). The function's result 
   is stored in the rpn_output vector in reverse-polish notation */
static void formula_to_rpn( INP const char* formula, INP const s_formula_data &mydata, 
				INOUTP vector<Formula_Object> &rpn_output ){

	stack<Formula_Object> op_stack;		/* stack for handling operators and brackets in formula */
	Formula_Object fobj;		 	/* for parsing formula objects */

	int ichar = 0;
	const char *ch = NULL;
	/* go through formula and build rpn_output along with op_stack until \0 character is hit */
	while(1) {
		ch = &formula[ichar];

		if ('\0' == (*ch)){
			/* we're done */
			break;
		} else if (' ' == (*ch)){
			/* skip space */
		} else {
			/* parse the character */
			get_formula_object( ch, ichar, mydata, &fobj );
			switch (fobj.type){
				case E_FML_NUMBER:
					/* add to output vector */
					rpn_output.push_back( fobj );
					break;
				case E_FML_OPERATOR:
					/* operators may be pushed to op_stack or rpn_output */
					handle_operator( fobj, rpn_output, op_stack);
					break;
				case E_FML_BRACKET:
					/* brackets are only ever pushed to op_stack, not rpn_output */
					handle_bracket( fobj, rpn_output, op_stack);
					break;
				default:
					vpr_printf(TIO_MESSAGE_ERROR, "in formula_to_rpn: unknown formula object type: %d\n", fobj.type);
					break;
			}
		}
		ichar++;
	}

	/* pop all remaining operators off of stack */
	Formula_Object fobj_dummy;
	while ( !op_stack.empty() ){
		fobj_dummy = op_stack.top();

		if (E_FML_BRACKET == fobj_dummy.type){
			vpr_printf(TIO_MESSAGE_ERROR, "in formula_to_rpn: Mismatched brackets in user-provided formula\n");
			exit(1);
		}		

		rpn_output.push_back( fobj_dummy );
		op_stack.pop();
	}

	return;
}

/* Fills the formula object fobj according to specified character and mydata, 
   which help determine which numeric value, if any, gets assigned to fobj
   ichar is incremented by the corresponding count if the need to step through the 
   character array arises */
static void get_formula_object( INP const char *ch, INOUTP int &ichar, INP const s_formula_data &mydata,
				 INOUTP Formula_Object *fobj ){

	/* the character can either be part of a number, or it can be an object like W, t, (, +, etc
	   here we have to account for both possibilities */

	if ( is_char_number(*ch) ){
		/* we have a number -- use atoi to convert */
		stringstream ss;
		while ( is_char_number(*ch) ){
			ss << (*ch);
			ichar++;
			ch++;	
		}
		ichar --;
		fobj->type = E_FML_NUMBER;
		fobj->data.num = my_atoi(ss.str().c_str());	//TODO: convoluted. rework?
	} else {
		switch ((*ch)){
			case 'W':
				fobj->type = E_FML_NUMBER;
				fobj->data.num = mydata.dest_W;
				break;
			case 't':
				fobj->type = E_FML_NUMBER;
				fobj->data.num = mydata.track;
				break;
			case '+':
				fobj->type = E_FML_OPERATOR;
				fobj->data.op = E_OP_ADD;
				break;
			case '-':
				fobj->type = E_FML_OPERATOR;
				fobj->data.op = E_OP_SUB;
				break;
			case '/':
				fobj->type = E_FML_OPERATOR;
				fobj->data.op = E_OP_DIV;
				break;
			case '*':
				fobj->type = E_FML_OPERATOR;
				fobj->data.op = E_OP_MULT;
				break;
			case '(':
				fobj->type = E_FML_BRACKET;
				fobj->data.left_bracket = true;
				break;
			case ')':
				fobj->type = E_FML_BRACKET;
				fobj->data.left_bracket = false;
				break;
			default:
				vpr_printf(TIO_MESSAGE_ERROR, "in get_formula_object: unsupported character: '%c'\n", ch);
				break; 
		}	
	}
	
	return;
}

/* returns integer specifying precedence of passed-in operator. higher integer 
   means higher precedence */
static int get_fobj_precedence( INP const Formula_Object &fobj ){
	int precedence = 0;

	if (E_FML_BRACKET == fobj.type){
		precedence = 0;
	} else if (E_FML_OPERATOR == fobj.type){
		t_operator op = fobj.data.op;
		switch (op){
			case E_OP_ADD: 
				precedence = 2;
				break;
			case E_OP_SUB: 
				precedence = 2;
				break;
			case E_OP_MULT: 
				precedence = 3;
				break;
			case E_OP_DIV: 
				precedence = 3;
				break;
			default:
				vpr_printf(TIO_MESSAGE_ERROR, "in get_fobj_precedence: unrecognized operator: %d\n", op);
				exit(1);
				break; 
		}
	} else {
		vpr_printf(TIO_MESSAGE_ERROR, "in get_fobj_precedence: no precedence possible for formula object type %d\n", fobj.type);
		exit(1);
	}
	
	return precedence;
}

/* Returns associativity of the specified operator */
static bool op_associativity_is_left( INP const t_operator &op ){
	bool is_left = true;
	
	/* associativity is 'left' for all but the power operator, which is not yet implemented */
	//TODO:
	//if op is 'power' set associativity is_left=false and return

	return is_left;
}

/* used by the shunting-yard formula parser to deal with operators such as add and subtract */
static void handle_operator( INP const Formula_Object &fobj, INOUTP vector<Formula_Object> &rpn_output,
				INOUTP stack<Formula_Object> &op_stack){
	if ( E_FML_OPERATOR != fobj.type){
		vpr_printf(TIO_MESSAGE_ERROR, "in handle_operator: passed in formula object not of type operator\n");
		exit(1);
	}

	int op_pr = get_fobj_precedence( fobj );
	bool op_assoc_is_left = op_associativity_is_left( fobj.data.op );

	Formula_Object fobj_dummy;
	bool keep_going = false;
	do{
		/* here we keep popping operators off the stack onto back of rpn_output while
		   associativity of operator is 'left' and precedence op_pr = top_pr, or while
		   precedence op_pr < top_pr */

		/* determine whether we should keep popping operators off the op stack */
		if ( op_stack.empty() ){
			keep_going = false;
		} else {
			/* get precedence of top operator */
			int top_pr = get_fobj_precedence ( op_stack.top() );

			keep_going = ( (op_assoc_is_left && op_pr==top_pr)
					|| op_pr<top_pr );
			
			if (keep_going){
				/* pop top operator off stack onto the back of rpn_output */
				fobj_dummy = op_stack.top();
				rpn_output.push_back( fobj_dummy );
				op_stack.pop();
			}
		}

	} while (keep_going);

	/* place new operator object on top of stack */
	op_stack.push(fobj);
	
	return;
}

/* used by the shunting-yard formula parser to deal with brackets, ie '(' and ')' */
static void handle_bracket( INP const Formula_Object &fobj, INOUTP vector<Formula_Object> &rpn_output,
				INOUTP stack<Formula_Object> &op_stack ){
	if ( E_FML_BRACKET != fobj.type){
		vpr_printf(TIO_MESSAGE_ERROR, "in handle_bracket: passed-in formula object not of type bracket\n");
		exit(1);
	}

	/* check if left or right bracket */
	if ( fobj.data.left_bracket ){
		/* left bracket, so simply push it onto operator stack */
		op_stack.push(fobj);
	} else {
		bool keep_going = false;
		do{
			/* here we keep popping operators off op_stack onto back of rpn_output until a 
			   left bracket is encountered */

			if ( op_stack.empty() ){
				/* didn't find an opening bracket - mismatched brackets */
				vpr_printf(TIO_MESSAGE_ERROR, "Ran out of stack while parsing brackets -- bracket mismatch\n");
				exit(1); 
				keep_going = false;
			}
	
			Formula_Object next_fobj = op_stack.top();
			if (E_FML_BRACKET == next_fobj.type){
				if (next_fobj.data.left_bracket){
					/* matching bracket found -- pop off stack and finish */
					op_stack.pop();
					keep_going = false;
				} else {
					/* should not find two right brackets without a left bracket in-between */
					vpr_printf(TIO_MESSAGE_ERROR, "Mismatched brackets encountered in user-specified formula\n");
					exit(1); 
					keep_going = false;
				}
			} else if (E_FML_OPERATOR == next_fobj.type){
				/* pop operator off stack onto the back of rpn_output */
				Formula_Object fobj_dummy = op_stack.top();
				rpn_output.push_back( fobj_dummy );
				op_stack.pop();
				keep_going = true;
			} else {
				vpr_printf(TIO_MESSAGE_ERROR, "Found unexpected formula object on operator stack: %d\n", next_fobj.type);
				exit(1);
				keep_going = false;
			}
		} while (keep_going);
	}
	return;
}


/* parses a reverse-polish notation vector corresponding to a switchblock formula
   and returns the integer result */
static int parse_rpn_vector( INOUTP vector<Formula_Object> &rpn_vec ){
	int result = -1;

	/* first entry should always be a number */
	if (E_FML_NUMBER != rpn_vec.at(0).type){
		vpr_printf(TIO_MESSAGE_ERROR, "parse_rpn_vector: first entry is not a number\n");
		exit(1);
	}

	Formula_Object fobj;
	int ivec = 0;
	/* keep going until we have gone through the whole vector */
	while ( !rpn_vec.empty() ){
		
		/* keep going until we have hit an operator */
		do{
			ivec++;		/* first item should never be operator anyway */
		} while ( E_FML_OPERATOR != rpn_vec.at(ivec).type );

		/* now we apply the selected operation to the two previous entries */
		/* the result is stored in the object that used to be the operation */
		rpn_vec.at(ivec).data.num = apply_rpn_op( rpn_vec.at(ivec-2), rpn_vec.at(ivec-1), rpn_vec.at(ivec) );
		rpn_vec.at(ivec).type = E_FML_NUMBER;

		/* remove the previous two entries from the vector */
		rpn_vec.erase(rpn_vec.begin() + ivec - 2, rpn_vec.begin() + ivec - 0);
		ivec -= 2;

		/* if we're down to one element, we are done */
		if (1 == rpn_vec.size()){
			result = rpn_vec.at(ivec).data.num;
			rpn_vec.erase(rpn_vec.begin() + ivec);
		}
	}

	return result;
}

/* applies operation specified by 'op' to the given arguments. arg1 comes before arg2 */
static int apply_rpn_op( INP const Formula_Object &arg1, INP const Formula_Object &arg2, 
					INP const Formula_Object &op ){
	int result = -1;
	
	/* arguments must be numbers */
	if ( E_FML_NUMBER != arg1.type || 
	     E_FML_NUMBER != arg2.type){
		vpr_printf(TIO_MESSAGE_ERROR, "in apply_rpn_op: one of the arguments is not a number\n");
		exit(1);
	}

	/* check that op is actually an operation */
	if ( E_FML_OPERATOR != op.type ){
		vpr_printf(TIO_MESSAGE_ERROR, "in apply_rpn_op: the object specified as the operation is not of operation type\n");
		exit(1);
	}

	/* apply operation to arguments */
	switch (op.data.op){
		case E_OP_ADD:
			result = arg1.data.num + arg2.data.num;
			break;
		case E_OP_SUB:
			result = arg1.data.num - arg2.data.num;
			break;
		case E_OP_MULT:
			result = arg1.data.num * arg2.data.num;
			break;
		case E_OP_DIV:
			result = arg1.data.num / arg2.data.num;
			break;
		default:
			vpr_printf(TIO_MESSAGE_ERROR, "in apply_rpn_op: invalid operation: %d\n", op.data.op);
			exit(1);
			break;
	}
	
	return result;
} 


/* checks if specified character represents an ASCII number */
static bool is_char_number ( INP const char &ch ){
	bool result = false;
	
	if ( ch >= '0' && ch <= '9' ){
		result = true;
	} else {
		result = false;
	}

	return result;
}
