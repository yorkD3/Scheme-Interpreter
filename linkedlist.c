/* Darryl York III & Lev Shuster
 * Interpreter 1: Linked List
 * 	A basic and limited linked list structure for Scheme data types 
 * 	Finished on April 28, 2021
*/

#include <stdbool.h>
#include "value.h"
#include "talloc.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//ONLY USE TALLOC NOT MALLOC

/* 
	Create a new NULL_TYPE value node.
*/
Value *makeNull(){
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = NULL_TYPE;
	return toReturn;
}

/* 
	Create a new CONS_TYPE value node.
*/
Value *cons(Value *newCar, Value *newCdr){
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = CONS_TYPE;
	toReturn->c.car = newCar;
	toReturn->c.cdr = newCdr;
	return toReturn;
}

/*
	Check if pointing to a NULL_TYPE value. 
*/
bool isNull(Value *value){
	return value->type == NULL_TYPE;
}


/* 
	Returns car of a list. Uses assert to make sure input is of CONS_TYPE
*/
Value *car(Value *list){
	assert(list->type == CONS_TYPE && "The list is not of CONS_TYPE!!");
	return list->c.car;
}

/* 
	Returns cdr of a list. Uses assert to make sure input is of CONS_TYPE
*/
Value *cdr(Value *list){
	assert(list->type == CONS_TYPE && "The list is not of CONS_TYPE!!");
	return list->c.cdr;
}


/* 
	Measure length of list. Uses assert to make sure that a value is either
		NULL_TYPE or CONS_TYPE
*/
int length(Value *value){
	assert(value->type == NULL_TYPE || value->type == CONS_TYPE && "You didn't pass in a CONS_TYPE or NULL_TYPE type into length(Value *value)");
	int counter=1;
	switch (value->type) {
		case NULL_TYPE:
			return 0;
		case CONS_TYPE:
			// a dotted pair may be a problem because the last item wont have a null to it
			if (!isNull(cdr(value))){
				counter += length(cdr(value));
			}
			return counter;
		default: //acts as an else statement. 
			return 1;
	}
}


/* 
	Return a new list that is the reverse of the one that is passed in. All
 		content within the list should be duplicated; there should be no shared
		memory whatsoever between the original list and the new one.
*/
Value *reverse(Value *list){
// nested lists likely won't work with reverse
// Don't duplicate list data
// should return a new list with a new set of CONS_TYPE Value nodes,

	Value *output = makeNull();
	Value *temp = list;
	
	for (int i = 0; i<length(list); i++){
		output = cons(car(temp), output);
		temp = cdr(temp);
	}
	return output;
}




/* 
	Displays the contents of the linked list to the screen in some kind of
 		readable format
*/
void display(Value *list){
	if (isNull(list)){
		printf("Empty List\n");
		return;
	}
	switch (car(list)->type) {
		case NULL_TYPE:
			printf("null reached )\n");
			return; 
		case INT_TYPE:
			printf("%i	", list->c.car->i);
			break;// Breaks from switch
		case DOUBLE_TYPE:
			printf("%f	", list->c.car->d);
			break;
		case STR_TYPE:
			printf("%s	", list->c.car->s);
			break;
		case CONS_TYPE:
			display(car(list));
			break;
		case  PTR_TYPE:
			printf("%p	", list->c.car->p);
			break;
		default: //Return the type of the token
			printf("%u  ", list->c.car->type);
	}

	if (!isNull(cdr(list))){
		display(cdr(list));
	}
	printf("\n");
}

// Basic use of functions here
/*
int main(){
	Value *test = makeNull();
	printf("%u\n", test->type);

	Value *myInt = malloc(sizeof(Value));
	myInt->type = INT_TYPE;
	myInt->i = 5;
	Value *myDouble = malloc(sizeof(Value));
	myDouble->type = DOUBLE_TYPE;
	myDouble->d = 5.234;

	Value *test0= cons(myDouble, test);
	Value *test1 = cons(myInt, test0);
	test1 = cons (test1, test);

	printf("Our Cons Cell reads (%i, %f)\n", test1->c.car->i, test1->c.cdr->d);
	display(test1);
	Value *reversed = reverse(test1);
	display(reversed);
	printf("The length is %i\n", length(&myInt));
	cleanup(test1);
	cleanup(reversed);
 }
 */