

#include <string.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include "util.h"
#include "arch_types.h"
#include "xml_arch_switchblocks.h"

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
static int parse_rpn_apply_op( INP const Formula_Object &arg1, INP const Formula_Object &arg2, 
					INP const Formula_Object &op );

/* checks if specified character represents an ASCII number */
static bool is_char_number ( INP const char &ch );

/**** Function Definitions ****/

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
	//we will have a stack for the operators, and a vector for the RPN output
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


	stack<Formula_Object> op_stack;		/* stack for handling operators in formula */
	Formula_Object fobj;			/* for parsing formula objects */

	int ichar = 0;
	const char *ch = NULL;
	/* go through formula and build rpn_output along with op_stack until \0 character is hit */
	while(1) {
		ch = &formula[ichar];

		if ('\0' == (*ch)){
			/* we're done */
			break;
		} else {
			/* skip spaces */
			if (' ' == (*ch)){
				ichar++;
				continue;
			}
		
			get_formula_object( ch, ichar, mydata, &fobj );
			switch (fobj.type){
				case E_FML_NUMBER:
					/* add to output vector */
					rpn_output.push_back( fobj );
					break;
				case E_FML_OPERATOR:
					handle_operator( fobj, rpn_output, op_stack);
					break;
				case E_FML_BRACKET:
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
			vpr_printf(TIO_MESSAGE_ERROR, "Mismatched brackets in user-provided formula\n");
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
		rpn_vec.at(ivec).data.num = parse_rpn_apply_op( rpn_vec.at(ivec-2), rpn_vec.at(ivec-1), rpn_vec.at(ivec) );
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
static int parse_rpn_apply_op( INP const Formula_Object &arg1, INP const Formula_Object &arg2, 
					INP const Formula_Object &op ){
	int result = -1;
	
	/* arguments must be numbers */
	if ( E_FML_NUMBER != arg1.type || 
	     E_FML_NUMBER != arg2.type){
		vpr_printf(TIO_MESSAGE_ERROR, "in parse_rpn_apply_op: one of the arguments is not a number\n");
		exit(1);
	}

	/* check that op is actually an operation */
	if ( E_FML_OPERATOR != op.type ){
		vpr_printf(TIO_MESSAGE_ERROR, "in parse_rpn_apply_op: the object specified as the operation is not of operation type\n");
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
			vpr_printf(TIO_MESSAGE_ERROR, "in parse_rpn_apply_op: invalid operation: %d\n", op.data.op);
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
