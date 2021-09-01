
#include "talloc.h"
#include "linkedlist.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>



static Value *eval(Value *tree, Frame *frame);
static void printValue(Value *token);
static int contains(Value *list, Value *value);
static void printFrame(Frame *frame);



/*
    Error function that leaves a message, frees memory and gracefully exits the program
        - If " " input for tokenOrSubtree, only prints the message
*/
void evalError(char* message, char* tokenOrSubtree){
	char *space = talloc(2);
	space[0] = ' ';
	space[1] = '\0';

	char* errorMessage = talloc(100);

	strcpy(errorMessage, message);
    
	if (!strcmp(tokenOrSubtree, space)){
        sprintf(errorMessage, "%s", message);
    } else {
        sprintf(errorMessage, "%s: %s", message, tokenOrSubtree);
    }
    
    printf("Evaluation Error: %s \n", errorMessage);
    texit(1);
}

/*
    Searches for a binding in accessable frames. 
        - Returns the value associated with the symbol 
*/
Value *lookupsymbol(Value *tree, Frame *frame){
	Value *listOfBindings = frame->bindings;
	
	if(isNull(listOfBindings) || !contains(listOfBindings, tree)){
		if(frame->parent != NULL){ return lookupsymbol(tree, frame->parent);}
		evalError("Symbol is not initialized", tree->s);
	}

	Value *curBinding;
	while(listOfBindings->type == CONS_TYPE){
		curBinding = car(listOfBindings);
		if(strcmp(car(curBinding)->s, tree->s) == 0){ 
		    return car(cdr(curBinding));
		}
		listOfBindings = cdr(listOfBindings);
	} // Not in current frame
	
    evalError("Symbol not in itialized", tree->s);
	return listOfBindings;
}


/*
	Search function that takes a Value linked list and a Value to be looked for.
		Returns 1 if found, 0 if not. 
		<ADD different valueTypes to find>	
*/
int contains(Value *list, Value *value){
	while(list->type != NULL_TYPE){
		if(strcmp(car(car(list))->s, value->s) == 0){
			return 1;
		} 
		list = cdr(list);
	}
	return 0;

}
/* Check if a value is numeric*/
int isNumeric(Value *val){return val->type == INT_TYPE || val->type == DOUBLE_TYPE;} // evaluate the value here??
int isInt(Value *val){return (val->d);} //Not finished delete if not used 

/*Primitives*/
/* PRIMITIVES TO ADD
	* <=,>=
	* integer?, list?, negative? number?, odd? not?... 
	* 
	* http://jscheme.sourceforge.net/jscheme/doc/R4RSprimitives.html 
	*
*/
Value *primitiveCons(Value *args){
	if(length(args) != 2){
		evalError("Wrong number of arguments for cons", " ");
	}
	Value *first = car(args);
	Value *second = car(cdr(args));
	return cons(first, second);
}

Value *primitiveCar(Value *args){
	if(length(args) != 1){
		evalError("Wrong number of arguments given to car", " ");
	} else if (car(args)->type != CONS_TYPE){ // Extra check 
		evalError("Can only request the cdr of a cons cell", " ");
	}
	return car(car(args));
}

Value *primitiveCdr(Value *args){
	if(length(args) != 1){
		evalError("Wrong number of arguments given to cdr", " ");
	} else if (car(args)->type != CONS_TYPE){ // Extra check 
		evalError("Can only request the car of a cons cell", " ");
	}
	return (cdr(car(args)));
}

Value *primitiveNull(Value *args){
	if(length(args) != 1){
		evalError("Cannot check type of multiple arguments", " ");
	}
	Value *boolean = makeNull();
	boolean->type = BOOL_TYPE;
	if(isNull(car(args))){
		boolean->i = 1;
	} else {
		boolean->i = 0;
	}
	return boolean;
}

/*
	Helper function for arithmetic
*/
Value *mathHelp(Value *args, char *operator){ // args should be evaluated already
	if ((!strcmp(operator, "/") ||!strcmp(operator, "<") || 
	  !strcmp(operator, ">") || !strcmp(operator, "=")) && length(args) > 2){ // Should I have 2 values?
		evalError("Comparison operators can only take two values", " ");
	}
	
	Value *result = makeNull();
	result->type = DOUBLE_TYPE;
	if (!strcmp(operator, "*") || !strcmp(operator, "*")){result->d = 1;} else {result->d = 0;}
	//set this to the first diget
	//args = cdr(args);//set args to cdr args
	int allInts = 1;
	while(args->type != NULL_TYPE){
		Value *curValue = car(args); // Should be evaluated before being given in
		
		if(!strcmp(operator, "+") || !strcmp(operator, "*")){
			//Check for valid input
			if(!isNumeric(curValue)){evalError("Can only do math on numeric values", " ");}
			
			if(!strcmp(operator, "+")){
				if(curValue->type == DOUBLE_TYPE){
					allInts = 0;
					result->d = curValue->d + result->d;
				} else {
					result->d = curValue->i + result->d;
				}
				args = cdr(args);
			} else if (!strcmp(operator, "*")){
				if(curValue->type == DOUBLE_TYPE){
					allInts = 0;
					result->d = curValue->d * result->d;
				} else {
					result->d = curValue->i * result->d;
				}
				args = cdr(args);
			} //Args == NULL_Type
		} else if(!strcmp(operator, "/")){
			Value *val1 = car(args);
			Value *val2 = car(cdr(args));
			args = cdr(cdr(args)); // Should be a null type value

			if(val1->type == DOUBLE_TYPE || val2->type == DOUBLE_TYPE){
				allInts = 0;
				if (val1->type == DOUBLE_TYPE && val2->type != DOUBLE_TYPE){
					result->d = val1->d / (double)val2->i;
				} else if(val1->type != DOUBLE_TYPE && val2->type == DOUBLE_TYPE){
					if(val1->i < val2->d){allInts = 0;}
					result->d = (double)val1->i / val2->d;
				} else {
					result->d = val1->d / val2->d;
				}
			} else {
				if(val1->i < val2->i){allInts = 0;}
				result->d = (double)val1->i / (double)val2->i;
			} 
		} else if(!strcmp(operator, "mod")){
			Value *val1 = car(args);//result
			Value *val2 = car(cdr(args)); // car(cdr(args))
			args = cdr(cdr(args)); // cdr(cdr(args)) Should be a null type value, ends loop

			result->d = val1->i % val2->i;
			allInts = 1;// remove for testing
		} else if (!strcmp(operator, "-")){
			Value *val1 = car(args);
			Value *val2 = car(cdr(args));
			args = cdr(cdr(args)); // Should be a null type value

			if(val1->type == DOUBLE_TYPE || val2->type == DOUBLE_TYPE){
				allInts = 0;
				if (val1->type == DOUBLE_TYPE && val2->type != DOUBLE_TYPE){
					result->d = val1->d - val2->i;
				} else if(val1->type != DOUBLE_TYPE && val2->type == DOUBLE_TYPE){
					result->d = val1->i - val2->d;
				} else {
					result->d = val1->d - val2->d;
				}
			} else {result->d = val1->i - val2->i;} 
		}
	}
	if(allInts == 1){
		result->type = INT_TYPE;
		result->i = (int)result->d;
	}
	return result;
}

/*
	Helper function for comparisons
*/
Value *compareHelp(Value *args){ // How interpreter deals with non 1 values 
	Value *boolean = makeNull();
	boolean->type = BOOL_TYPE;
	// Compare two values
	Value *val1 = car(args);
	Value *val2 = car(cdr(args));

	if(val1->type == DOUBLE_TYPE || val2->type == DOUBLE_TYPE){
		if(val1->type == DOUBLE_TYPE && val2->type !=DOUBLE_TYPE){
			boolean->i = val1->d - val2->i;
		} else if(val1->type != DOUBLE_TYPE && val2->type == DOUBLE_TYPE){
			boolean->i = val1->i - val2->d;
		} else {
			boolean->i = val1->d - val2->d;
		}
	} else {
		boolean->i = val1->i - val2->i; // if equal will become false, else true
	}
	return boolean;
}

Value *primitiveAddition(Value *args) {
	if (length(args) == 0){evalError("Addition requires at least one argument", " ");
	} else if (length(args) == 1){return car(args);}
	
	return mathHelp(args, "+"); // fix. should only be 2 lines 
}

Value *primitiveSubtraction(Value *args){
	if (length(args) < 2){
		evalError("Subtraction requires more than one argument", " ");
	} 
	return mathHelp(args, "-");
}

Value *primitiveMultiply(Value *args){
	if (length(args) < 2){
		evalError("Multiplication requires more than one argument", " ");
	} 
	return mathHelp(args, "*");
}

Value *primitiveDivision(Value *args){
	if (length(args) != 2){
		evalError("Division requires two aguments", " ");
	} 
	return mathHelp(args, "/");
}

Value *primitiveModulo(Value *args){
	if (length(args) != 2){
		evalError("Modulo requires only two aguments", " ");
	} 
	return mathHelp(args, "mod");
}

Value *primitiveGreater(Value *args){
	if (length(args) != 2){
		evalError("Greater than comparison requires two arguments", " ");
	}

	Value *result = compareHelp(args);
	if (result->i <= 0){
		result->i = 0;
	}
	return result;
}

Value *primitiveLess(Value *args){
	if (length(args) != 2){
		evalError("Less than comparison requires two arguments", " ");
	}
	Value *result = compareHelp(args);
	result->i = result->i * -1;
	// 4<12 heper gets -8 *-1 =8
	if (result->i <= 0){
		result->i = 0;
	}
	return result;
}

Value *primitiveEqual(Value *args){ 
	if (length(args) != 2){
		evalError("Equal comparison requires two arguments", " ");
	}
	Value *result = compareHelp(args);
	if (result->i != 0){
		result->i = 0;
	} else {result->i = 1;}
	return result;
}

// Value *evalArgs(Value *args, Frame *frame){
// 	Value *evaledArgs = makeNull();		
// 	while(args->type != NULL_TYPE){ // evaluate each arg and THEN pass it to args
// 		evaledArgs = cons(eval(car(args), frame), evaledArgs);
// 		args = cdr(args);
// 	}
// 	evaledArgs = reverse(evaledArgs);// should it be reversed?
// 	return evaledArgs;
// }

/* Special Forms*/
Value *evalAnd(Value *args, Frame *frame){ // args should be a list of evaluated expressions
	if (args->type != CONS_TYPE && args->type != BOOL_TYPE && !isNumeric(args)){
		evalError("And requires one or more values", " ");
	}
		// printf("\nor is running \n");

	Value *boolean = makeNull();
	boolean->type = BOOL_TYPE;
	boolean->i = 1;
	while(args->type != NULL_TYPE){
		Value *curExpression = eval(car(args), frame);
		if (curExpression->i == 0){ //|| curExpression->d == 0){
			boolean->i = 0;
			return boolean;
		}
		args = cdr(args);
	}
	return boolean;
}

Value *evalOr(Value *args, Frame *frame){ // args should be a list of evaluated expressions
	if (args->type != CONS_TYPE && args->type != BOOL_TYPE && !isNumeric(args)){
		evalError("Or requires one or more values", " ");
	}
	// printf("\nor is running \n");
	Value *boolean = makeNull();
	boolean->type = BOOL_TYPE;
	boolean->i = 0;
	// printValue(args);
	// 	printf("\targs\n");
	while(args->type != NULL_TYPE){
		Value *curExpression = eval(car(args), frame);
		// printValue(curExpression);
		// printf("\n");
		if(curExpression->type == VOID_TYPE){
			boolean->i = 0;
		} else if(curExpression->i != 0){ //|| curExpression->d != 0){
			boolean->i = 1;
			return boolean;
		}
		args = cdr(args);
	}
	return boolean;

	//printValue(curExpression);
		// //printf("\tCur expression\n");
		// if (curExpression->i != 0 || curExpression->d !=0){
		// 	//printf("true case triggered\n");
		// 	boolean->i = 1;
		// 	// return curExpression; // Did not work, boolean value should be returned, can be 
		// 	return boolean;
		// 	// (or 0 0 0 7) --> 7 not 1
		// }
}


int setBinding(Value *variable, Value *newVal, Frame *frame){
	newVal = cons(eval(newVal, frame), makeNull());

	Value *bindings = frame->bindings;
	// printFrame(frame);
	while(bindings->type != NULL_TYPE || frame->parent != NULL){
		
		// printValue(bindings);
		// printf("\n");
		Value *curBinding = car(bindings);
		// printValue(curBinding);
		// printf("\t curbinding\t");
		// printf("Variable: ");
		// printValue(variable);
		// printf("\n");
		

		if(!strcmp(variable->s, car(curBinding)->s)){//found binding
			// printf("\n-------------------------\n");
			// printValue(newbinding);
			// curBinding = newbinding;
			curBinding->c.cdr = newVal; // not finding the right binding?
			// printValue(curBinding);
			// printf("Being set");

			// printf("\ngot past complex line\n");
			// printf("\n\n\n");
			// printValue(cdr(car(frame->bindings)));
			// printValue(car(frame->bindings));
			// printf("\n\n\n");

			return 1;
		}
		bindings = cdr(bindings);
		if(bindings->type == NULL_TYPE && frame->parent != NULL){
			frame = frame->parent; 
			bindings = frame->bindings;
		}
	}
	return 0;
}

Value *evalSet(Value *args, Frame *frame){
	if (length(args) < 2){evalError("set! syntax invalid", "(set! var expr)");} // Maybe better errorchecks
	lookupsymbol(car(args), frame); // Error check for variable
	// syntax: (set! var expr) 
	//Evaluate expression, assign var to expression value	
	
	// printf("\n\n\n\n\n");
	// printValue(args);
	// printf("\t args\n");
	// printValue(car(cdr(args)));
	// printf("\t Expression\n");

	// 	printf("\n\n");
	// printFrame(frame);
		//printFrame(frame->parent);

	// printf("\t set frame\n");
	Value *variable =  car(args);
	Value *expression = car(cdr(args));
	// printValue(expression);
	// if not a valid assignment, eval error 
	//Change Bindng value
	int bound = setBinding(variable, expression, frame);
	if (bound == 0){evalError("set! variable has not been initialized", variable->s);}
	// printf("\n\n evaluating at end\n");
	// printFrame(frame);

	Value *toReturn = makeNull();
	toReturn->type = VOID_TYPE;
	return toReturn;
}

Value *evalBegin(Value *args, Frame *frame){
	// syntax: (begin expr1 expr2 ...)
	// Evaluate each expression and return the final evaluated expression
	
	//Any error checks??
	while(args->type != NULL_TYPE){
		Value *curExpression = car(args);
		// printf("\n1111111111111111111111111111\n");
		// printValue(curExpression);
		// printf("\n1111111111111111111111111111\n");

		if(cdr(args)->type != NULL_TYPE){
			eval(curExpression, frame);

			args = cdr(args);
		} else {return eval(curExpression, frame);}
	}
	Value *toReturn = makeNull(); //if no arguments to begin, return void type
	toReturn->type = VOID_TYPE;
	printf("\n No value returned from begin statement\n");
	return toReturn;
} 


/*
	Evaluates a quote statement
		- Returns the unevaluated statement
*/
Value *evalQuote(Value* args){
	if(length(args) != 1){
		evalError("More then one argument withing quote", "");
	}
		return car(args);
}
/*
    Evaluates an if expression
        - Returns answer as a Value* 
*/
Value *evalIf(Value *args, Frame *frame){
	if(length(args)!= 3){ evalError("Incorrect number of arguments", " ");}  
	Value *condition = car(args);																
	Value *trueExpr = car(cdr(args));
	Value *falseExpr = car(cdr(cdr(args)));
	condition = eval(condition, frame);

	if(!(condition->type == BOOL_TYPE || condition->type == INT_TYPE)){
		evalError("Condition statement must evaluate to a boolean or number", " ");
	}
	if(condition->i == 0){
		return eval(falseExpr, frame);
	}else{
		return eval(trueExpr, frame);
	}
}

Value *evalCond(Value *args, Frame *frame){
	// (cond (if expr)
	// 			(else if expr)
	// 			(else if expr)
	// )
		// if car(car(args)) == #t, then return eval of cdr(car(args))
	// else set args = cdr(args) 
	// if args is null then return void type //have this at the start of our code
	
	if(args->type == NULL_TYPE){ // This sectioin is at the bottom too, hoow doo we get it just at the top?
		Value *voidReturn = makeNull();
		voidReturn->type = VOID_TYPE;
		return voidReturn;
	} else if (args->type != CONS_TYPE && args->type != NULL_TYPE){
		evalError("No conditional arguments given", "INSERT CONDITIONAL SYNTAX ON LINE 477");
	}
	while (args->type != NULL_TYPE){
		Value *expression = car(args);
			if(expression->type != CONS_TYPE || length(expression) != 2){ // Maybe just < 2
				evalError("Invalid cond syntax", "INSERT CONDITIONAL SYNTAX ON LINE 482");
			} 
		Value *condition;
		if(car(expression)->type == SYMBOL_TYPE){
			condition = car(expression);
		} else {
			condition = eval(car(expression), frame);
		}
			if(condition->type != BOOL_TYPE && condition->type != INT_TYPE && condition->type != DOUBLE_TYPE && condition->type != SYMBOL_TYPE){
				evalError("Invalid condition given", " ");
			}

		if (condition->type == SYMBOL_TYPE){
			Value *body = car(cdr(expression));
			return eval(body,frame);
		} else if (condition->i == 0){// || condition->d == 0){
			args = cdr(args);
		} else {
			Value *body = car(cdr(expression));
			return eval(body, frame); // may have problems evaluating
		}
	}
	Value *voidReturn = makeNull();
	voidReturn->type = VOID_TYPE;
	return voidReturn;
}

void letErrorChecks(Value *args, Frame *frame){
	Value *inputBindings = car(args);
		if(inputBindings->type != CONS_TYPE && inputBindings->type != NULL_TYPE){evalError("Incorrect use of let bindings", " ");}
		if(inputBindings->type != NULL_TYPE){
				if(car(inputBindings)->type != CONS_TYPE || cdr(car(inputBindings))->type == NULL_TYPE){
						evalError("there is no value to set the paring symbol equal to", "");
				}
			}
	Value *body = cdr(args);
    if(body->type == NULL_TYPE){evalError("Incorrect use of let body", " ");}
}

Value *evalLetStar(Value *args, Frame *frame){
	//syntax: (let* ((var expr) ...) body1 body2 ...) 
	//Returns the eval of the last body expression
	//	Each expr can use the previous bidnings
	//	- Anna's note: create a new frame for each binding.
	//	- Dybvig: Let* can be converted to a set of nested let expressions	
	//New frame: bindings = curBinding; parent = letFrame 

	Value *inputBindings = car(args);

    //edge cases
    if(inputBindings->type != CONS_TYPE && inputBindings->type != NULL_TYPE){evalError("Incorrect use of let bindings", " ");}
    if(inputBindings->type != NULL_TYPE){
        if(car(inputBindings)->type != CONS_TYPE || cdr(car(inputBindings))->type == NULL_TYPE){
            evalError("there is no value to set the paring symbol equal to", "");
        }
    }
		
    Value *body = cdr(args);
    if(body->type == NULL_TYPE){evalError("Incorrect use of let body", " ");}

	Frame *letFrame = talloc(sizeof(Frame));
	letFrame->bindings = makeNull();
	letFrame->parent = frame;
	 
	while(inputBindings->type != NULL_TYPE){ 
		Value *bindingList = makeNull();
		Frame *bindingFrame = talloc(sizeof(Frame));
		bindingFrame->parent = letFrame;
		letFrame = bindingFrame;
		Value *binding = makeNull();
		
			//edge cases
			if (inputBindings->type != CONS_TYPE) {evalError("Incorrect use of let bindings", " ");} 
			binding = car(inputBindings);
			if (binding->type == NULL_TYPE){evalError("Empty binding", " ");}
		
		Value *variable = car(binding);

			//edge cases
			Value *bindingListTester = bindingList;
			while(bindingListTester->type != NULL_TYPE){ // Check past bindings for same variable name
				if(strcmp(car(car(bindingListTester))->s, variable->s) == 0){
					evalError("varible of the same name has already been bound by this ", " ");
				}
				bindingListTester = cdr(bindingListTester);
			}
			if(variable->type != SYMBOL_TYPE){evalError("Can't assign a value to a non-symbol type", " ");}
			if(car(cdr(binding))->type == NULL_TYPE){evalError("No value was given to be assigned", variable->s);}

    Value *value = eval(car(cdr(binding)), letFrame->parent); // use parent frame, not new frame

    //create a binding
		Value *newBinding = makeNull();
		newBinding = cons(value, newBinding);
		newBinding = cons(variable, newBinding);
		bindingList = cons(newBinding, bindingList);

		//Add binding to 'bindingFrame'
		letFrame->bindings = bindingList;

		inputBindings = cdr(inputBindings);
	}


	Value *bodyExpression; 
	while (cdr(body)->type != NULL_TYPE){
		bodyExpression = car(body);
		eval(bodyExpression, letFrame); // want this to be the last frame we've created
		
		body = cdr(body);
	}
	bodyExpression = car(body);

	return eval(bodyExpression, letFrame);
}

Value *evalLetrec(Value *args, Frame *frame){
	//printf("starting letrec\n");

	// syntax: (letrec ((var expr) ...) body1 body2 ...) 
	// Returns the eval of the last body expression
	//	eval THEN bind??(may be expressed in terms of let and set! as)
	letErrorChecks(args, frame);
	Value *inputBindings = car(args);

	Value *body = cdr(args);

	Frame *letFrame = talloc(sizeof(Frame));
	letFrame->bindings = makeNull();
	letFrame->parent = frame;
	
	// Value *values = makeNull();
	// while (inputBindings->type != NULL_TYPE){
	// 	Value *binding = car(inputBindings);
	// 		if (inputBindings->type != CONS_TYPE || length(binding) != 2){evalError("Incorrect use of let bindings", car(car(inputBindings))->s);}

	// 	//Evaluating the binding values 
	// 	values = cons(eval(car(cdr(binding)), letFrame), letFrame->bindings); // CAN'T Access other values in let expr
	// 	inputBindings = cdr(inputBindings);
	// }
	// values = reverse(values);
	// printValue(values);
	// printf("\t non-evaluated values \n");

	Value *bindingList = makeNull();
	//Loop through values, giving variable and value(not evaluated)
	while (inputBindings->type != NULL_TYPE){
		Value *binding = car(inputBindings);

			if (inputBindings->type != CONS_TYPE || binding->type == NULL_TYPE) {evalError("Incorrect use of let bindings", car(car(inputBindings))->s);} 
		
		Value *variable = car(binding);
			if(variable->type != SYMBOL_TYPE){evalError("Can't assign a value to a non-symbol type", " ");}
			//edge cases
			// Value *bindingListTester = inputBindings;
			// while(bindingListTester->type != NULL_TYPE){ // Check all bindings for same variable name
			// 	if(strcmp(car(car(bindingListTester))->s, variable->s) == 0){
			// 		evalError("Cannot assign two bindings to the same name", " ");
			// 	}
			// 	bindingListTester = cdr(bindingListTester);
			// }
			
			if(car(cdr(binding))->type == NULL_TYPE){evalError("No value was given to be assigned in letrec", variable->s);}
    	Value *value = car(cdr(binding));
			
			
			// from line 597, i think it's bad bc i evaluated the values before they were assigned to a binding 
		// 	if (values->type == NULL_TYPE){evalError("Too many values given", " ");}
		// Value *value = car(values); // Pull from values
		// values = cdr(values);

		//create a binding
		Value *newBinding = makeNull();
		newBinding = cons(value, newBinding);
		newBinding = cons(variable, newBinding);
		bindingList = cons(newBinding, bindingList);
		// printValue(newBinding);
		// printf("\n\t new binding \n");

		//move to next binding
		inputBindings = cdr(inputBindings);
	}

	bindingList = reverse(bindingList); // Should this be reversed?
	// //printValue(bindingList);
	// //printf("\n\t bindingList \n");
	//evaluate binding values
	Value *newBindingList = makeNull();
	while (bindingList->type != NULL_TYPE){
		Value *variable = car(car(bindingList));
		Value *value = eval(car(cdr(car(bindingList))), letFrame); // should we eval in the parent?
		
		Value *binding = makeNull();
		binding = cons(value, binding);
		binding = cons(variable, binding);

		newBindingList = cons(binding, newBindingList);
		
		bindingList = cdr(bindingList);
	}

	letFrame->bindings = newBindingList;
	// printValue(letFrame->bindings);
	
	Value *bodyExpression; 
	while (cdr(body)->type != NULL_TYPE){
		bodyExpression = car(body);
		eval(bodyExpression, letFrame); // want this to be the last frame we've created
		
		body = cdr(body);
	}
	bodyExpression = car(body);
	// printf("finished letrec\n");
	return eval(bodyExpression, letFrame);
}
// Function creation ends here 


/*
    Evaluates a let expression
        - Returns answer as a Value* 
*/
Value *evalLet(Value *args, Frame *frame){
	Value *inputBindings = car(args);

    //edge cases
    if(inputBindings->type != CONS_TYPE && inputBindings->type != NULL_TYPE){evalError("Incorrect use of let bindings", " ");}
    if(inputBindings->type != NULL_TYPE){
        if(car(inputBindings)->type != CONS_TYPE || cdr(car(inputBindings))->type == NULL_TYPE){
            evalError("there is no value to set the paring symbol equal to", "");
        }
    }
		
    Value *body = cdr(args);

    //edge case
    if(body->type == NULL_TYPE){evalError("Incorrect use of let body", " ");}

	Frame *letFrame = talloc(sizeof(Frame));
	letFrame->parent = frame;
	
	 
	Value *bindingList = makeNull();
	while(inputBindings->type != NULL_TYPE){
		Value *binding = makeNull();
		
        //edge cases
    if (inputBindings->type != CONS_TYPE) {evalError("Incorrect use of let bindings", " ");} 
		binding = car(inputBindings);
    if (binding->type == NULL_TYPE){evalError("Empty binding", " ");}
		
		Value *variable = car(binding);

        //edge cases
		Value *bindingListTester = bindingList;
    while(bindingListTester->type != NULL_TYPE){ // Check past bindings for same variable name
			if(strcmp(car(car(bindingListTester))->s, variable->s) == 0){
				evalError("varible of the same name has already been bound by this ", " ");
			}
			bindingListTester = cdr(bindingListTester);
		}
		if(variable->type != SYMBOL_TYPE){evalError("Can't assign a value to a non-symbol type", " ");}
      if(car(cdr(binding))->type == NULL_TYPE){evalError("No value was given to be assigned", variable->s);}
		
        
        Value *value = eval(car(cdr(binding)), frame); // use parent frame, not new frame
		
        //create a single binding
		Value *newBinding = makeNull();
		newBinding = cons(value, newBinding);
		newBinding = cons(variable, newBinding);
		bindingList = cons(newBinding, bindingList);

		inputBindings = cdr(inputBindings);
	}
	letFrame->bindings = reverse(bindingList);

 
    Value *bodyExpression; 
    while (cdr(body)->type != NULL_TYPE){
        bodyExpression = car(body);
        eval(bodyExpression, letFrame); 
        
        body = cdr(body);
    }
    bodyExpression = car(body);
		// printValue(bodyExpression);
		// printf("\nBindings: ");
	 	// printFrames(letFrame);
		// printf("\n");
    return eval(bodyExpression, letFrame);
}


/*
	Evaluates a 'define' expression. Adds the function to the top level frame.
		- Returns a void type Value*
*/
Value *evalDefine(Value *args, Frame *frame){ // Won't work for more than 1 arg
	if(length(args) < 2){evalError("Incorrect number of arguments for define statements\t - (define var expr)", " ");}
	if(frame->parent != NULL){evalError("Define statements must be at the top level", " ");}

	Value *voidReturn = makeNull();
	voidReturn->type = VOID_TYPE;
	
	Value *variable = car(args);
	if(variable->type != SYMBOL_TYPE){
		evalError("define must assign a value to a symbol type this is not a symbol type", " ");
	}
	Value *value = eval(car(cdr(args)), frame); // is this evaluating everything i need?

	Value *bindingChecker = frame->bindings;
	while (bindingChecker->type == CONS_TYPE){
		if(contains(bindingChecker, variable)){
			evalError("Can't define functions with the same name", variable->s);
		}
		bindingChecker = cdr(bindingChecker);
	}

	//Add binding to frame
	Value* binding = makeNull();
	binding = cons(value, binding);
	binding = cons(variable, binding);
	Value* temp = frame->bindings;
	frame->bindings = cons(binding, temp);
	return voidReturn;
}

int LambdaContains(Value *list, Value *value){ // Should generalize and make a full contains method
	while(list->type != NULL_TYPE){							 // Needs to be able to eval nums, symbols, types, etc.
		if(strcmp(car(list)->s, value->s) == 0){
			return 1;
		} 
		list = cdr(list);
	}
	return 0;
}

Value *evalLambda(Value *args, Frame *frame){
	if(args->type == NULL_TYPE){
		evalError("Invalid type for lambda operation", " ");
	}
	if(length(args) < 2){
			evalError("lambda needs two argument", " ");
		}
	
	Value *lambdaArgs = car(args);
	
	// for length of car args, add to lambda args 
	if (lambdaArgs->type != NULL_TYPE && lambdaArgs->type != CONS_TYPE && lambdaArgs->type != SYMBOL_TYPE){
		evalError("Invalid type for lambda args", " ");
	}  else if (lambdaArgs->type == CONS_TYPE && car(lambdaArgs)->type != SYMBOL_TYPE){
		evalError("Invalid arg name in lambda", " ");
	}

	//Check duplicates
	Value *lambdaArgs2 = lambdaArgs;
	if(lambdaArgs2->type != NULL_TYPE && length(lambdaArgs2) > 1){
		if(cdr(lambdaArgs2)->type == CONS_TYPE && LambdaContains(cdr(lambdaArgs2), car(lambdaArgs2))){
			evalError("Duplicate argument use", " ");
		}
		lambdaArgs2 = cdr(lambdaArgs2);
	}

	Value *newClosure = makeNull();
	newClosure->type = CLOSURE_TYPE;
	// printf("Closure param type: %u\n", car(args)->type);
	newClosure->closure.params = car(args);
	newClosure->closure.fnBody = car(cdr(args));
	newClosure->closure.frame = frame;
	// printf("\n in lambda");
	// printFrame(newClosure->closure.frame);
	// printValue(newClosure->closure.params);
	return newClosure;

}

Value *apply(Value *function, Value *args){
	//Closure check
	if (function->type == CLOSURE_TYPE){
	// Construct a new frame
	Frame *appFrame = talloc(sizeof(Frame));
	appFrame->bindings = makeNull();
	appFrame->parent = function->closure.frame;


	// Add bindings with formal args mapped to args
	//Add bindings to the new frame mapping each formal parameter (found in the closure) to the corresponding actual parameter (found in args).
	if(function->closure.params->type == CONS_TYPE){
		Value *variable = function->closure.params; // better naming later

		while (args->type != NULL_TYPE){

			Value *binding = makeNull();

			Value *value = car(args);
			binding = cons(value, binding);
			binding = cons(car(variable), binding);

			//Add binding to frame
			appFrame->bindings = cons(binding, appFrame->bindings);

			variable = cdr(variable);
			value = cdr(args);
			args = cdr(args);
		} // Not checking if args is also null
	}
	// printf("\n");
	// printValue(appFrame->bindings);
	// printf("\t appFrame bindings\n");
	return eval(function->closure.fnBody, appFrame);
	} else if (function->type == PRIMITIVE_TYPE){
		return (function->primitivefn)(args);
	} 
	evalError("A function was not input to be applied", " ");
	return makeNull();
	
}

//New function for evaluating arguments
//don't eval args until your inside of an and or
Value *evalArgs(Value *args, Frame *frame){
	Value *evaledArgs = makeNull();		
	while(args->type != NULL_TYPE){ // evaluate each arg and THEN pass it to args
		evaledArgs = cons(eval(car(args), frame), evaledArgs);
		args = cdr(args);
	}
	evaledArgs = reverse(evaledArgs);// should it be reversed?
	return evaledArgs;
}

/*
    Evaluates a given expression
        - Returns a Value*
*/ 
Value *eval(Value *tree, Frame *frame) {
    
    switch (tree->type){
    case INT_TYPE: {
        return tree;
        break;
    } case STR_TYPE: {
        return tree;
        break;
    } case DOUBLE_TYPE: {
        return tree;
        break;
    } case BOOL_TYPE: {
        return tree;
        break;
		} case SYMBOL_TYPE: { //Print and examine the frame
        return lookupsymbol(tree, frame);
        break;
		} /*case CLOSURE_TYPE: {
				printf("WHAT DO WE DO HERE??");
				// return eval(tree->fnBody, tree->frame);
				break;
		}case SINGLEQUOTE_TYPE: {
			printValue(tree);
      // return cdr(tree);
      break;
    }*/ case CONS_TYPE: { // How do i consistently get the right spot?
      Value *first = car(tree);
			Value *args = cdr(tree);	

      Value *result;
			if (strcmp(first->s, "quote") == 0) { 
				result = evalQuote(args);
			} else if (strcmp(first->s, "if") == 0) {
				result = evalIf(args, frame);
			} else if (strcmp(first->s, "cond") == 0) {
				result = evalCond(args, frame);
			} else if (strcmp(first->s, "let") == 0) {
				result = evalLet(args, frame);
			} else if (strcmp(first->s, "let*") == 0) {
				result = evalLetStar(args, frame);
			} else if (strcmp(first->s, "letrec") == 0) {
				result = evalLetrec(args, frame);
			} else if (strcmp(first->s, "define") == 0){
				result = evalDefine(args, frame);
			} else if (strcmp(first->s, "lambda") == 0){
				result = evalLambda(args, frame);
			} else if (strcmp(first->s, "and") == 0){
				//Eval args and then
				//args = evalArgs(args, frame);
				result = evalAnd(args, frame);
			} else if (strcmp(first->s, "or") == 0){
				//Eval args first
				//args = evalArgs(args, frame);
				result = evalOr(args, frame);
			} else if(strcmp(first->s, "set!") == 0){
				result = evalSet(args, frame);
			} else if(strcmp(first->s, "begin") == 0){
				result = evalBegin(args, frame);
			} else { // Eval user defined functions here
				Value *function = eval(first, frame);
				Value *evaledArgs = evalArgs(args, frame);// should we eval before or during
				result = apply(function, evaledArgs);
				
     }
			return result;
      break;

     } default: {printf("ANOT WORKING CORRECTLY");}
    }    
    printf("BNOT WORKING CORRECTLY");
		evalError("Unrecognizable car(tree)->type", " ");
		return tree;
}

/*
 * Adds a binding between the given name
 * and the input function. Used to add
 * bindings for primitive funtions to the top-level
 * bindings list.
 */
void bind(char *name, Value *(*function)(Value *), Frame *frame) {
		Value *nameValue = makeNull();
		nameValue->type = SYMBOL_TYPE;
		nameValue->s = name;

		// No checks??
		

		Value *primitiveFunction = makeNull();
		primitiveFunction->type = PRIMITIVE_TYPE;
		primitiveFunction->primitivefn = function;

		Value *binding = makeNull();
		binding = cons(primitiveFunction, binding);
		binding = cons(nameValue, binding);
		frame->bindings = cons(binding, frame->bindings);
		
		// printValue(binding); // KEEP
		// printf("\tbinding\n");
		
}


/*
    Prints the value of a Value*
*/
void printValue(Value *token){
    switch(token->type){
        case INT_TYPE:
            printf("%i", token->i);
            break;
        case DOUBLE_TYPE:
            printf("%f", token->d);
            break;
        case STR_TYPE:
            printf("%s", token->s);
            break;
        case CONS_TYPE:
					printf("(");
					if(cdr(token)->type != CONS_TYPE && cdr(token)->type != NULL_TYPE){ // Printing Dotted Pair
						printValue(car(token));
						printf(" . ");
						printValue(cdr(token));
						printf(")");
					}else{ // NEEDED TO HAVE CASES FOR BOTH DOTTED PAIR ENDINGS AND NON ENDING PAIRS *ADDED*
            while(token->type == CONS_TYPE){
                printValue(car(token));
                token = cdr(token);
								if (token->type != CONS_TYPE && token->type != NULL_TYPE){
									printf(" . ");
									printValue(token);
									printf(")");
								} else if (token->type != NULL_TYPE){
                    printf(" ");
                } else{printf(")");} // END OF EXPRESSION
            }
            break;
					}
        case OPEN_TYPE:
            printf("%s", token->s);
            break;
        case CLOSE_TYPE:
            printf("%s", token->s);
            break;
        case BOOL_TYPE:
            if (token->i == 0){
                printf("#f");
            } else {
                printf("#t");
            }
            break;
        case SYMBOL_TYPE:
            printf("%s", token->s);
            break;
        case OPENBRACKET_TYPE:
            printf("%s", token->s);
            break;
        case CLOSEBRACKET_TYPE:
            printf("%s", token->s);
            break;
        case DOT_TYPE:
            printf("%s", token->s);
            break;
        case SINGLEQUOTE_TYPE:
            printf("%s", token->s);
            break;
        case NULL_TYPE: // Item with null type '()
						printf("()");
            break;
				case VOID_TYPE:
					// printf("Definition added");
					break;
				case CLOSURE_TYPE:
					printf("#<procedure>");
					break;
				case PRIMITIVE_TYPE:
					printf("#<primitivefn>");
					break;
        default:
            printf("Bad %u\n", token->type);
            break;
    }
}

void printFrame(Frame *frame){
	int level = 0;

	Frame *tracker = frame;
	while(tracker->parent != NULL){level++; tracker = tracker->parent;}

	printf("Frame %i: \n\t", level);
	printValue(frame->bindings);
	printf("\n");
}

// void printFrames(Frame *frame){
// 	int level = 0;

// 	Frame *tracker = frame;
// 	while(tracker->parent != NULL){printFrame(tracker); tracker = tracker->parent;}
// }

/* 
	Binds primitives to the topLevelFrame
*/
void bindPrimitives(Frame *frame){
	bind("car", primitiveCar, frame);
	bind("cdr", primitiveCdr, frame);
	bind("null?", primitiveNull, frame);
	bind("cons", primitiveCons, frame);
	bind("+", primitiveAddition, frame);
	bind("-", primitiveSubtraction, frame);
	bind("*", primitiveMultiply, frame);
	bind("/", primitiveDivision, frame);
	bind("modulo", primitiveModulo, frame);
	bind(">", primitiveGreater, frame);
	bind("<", primitiveLess, frame);
	bind("=", primitiveEqual, frame);
}


/*
    Takes in a parsed tree and evaluates it
        - Prints the evaluated tree
*/
void interpret(Value *tree){
	Frame *topLevelFrame = talloc(sizeof(Frame));
	topLevelFrame->parent = NULL;
	topLevelFrame->bindings = makeNull();
	
	bindPrimitives(topLevelFrame);

	while(tree->type != NULL_TYPE){
		printValue(eval(car(tree), topLevelFrame));
		printf("\n");

		// printFrame(topLevelFrame);

		tree = cdr(tree);
  }
}
