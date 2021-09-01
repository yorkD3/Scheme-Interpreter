#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "value.h"

// Global head of used memory list
Value *usedMemHeader = NULL;

/*
	Create a NULL_TYPE Value and return it's pointer
*/
Value *tMakeNull(){
	Value *toReturn = malloc(sizeof(Value));
	toReturn->type = NULL_TYPE;
	return toReturn;
}

/*
	Check value for NULL_TYPE
*/
bool tIsNull(Value *value){
	return value->type == NULL_TYPE;
}

/* 
	Returns car of a list. Uses assert to make sure input is of CONS_TYPE
*/
Value *tCar(Value *list){
	assert(list != NULL);
	assert(list->type == CONS_TYPE);
	return list->c.car;
}

/* 
	Returns cdr of a list. Uses assert to make sure input is of CONS_TYPE
*/
Value *tCdr(Value *list){
	assert(list != NULL);
	assert(list->type == CONS_TYPE);
	return list->c.cdr;
}

/* 
	Create a new CONS_TYPE value.
*/
Value *tCons(Value *item, Value *ptr){
	Value *toReturn = malloc(sizeof(Value)); ;
	toReturn->type = CONS_TYPE;
	toReturn->c.car = item;
	toReturn->c.cdr = ptr;
	return toReturn;
}

/*
	Replacement for malloc that stores the pointers of allocated 
		memory in a linked list using the 't' structured functions
*/
void *talloc(size_t size){
	if(usedMemHeader == NULL){
	 	usedMemHeader = tMakeNull(); 		//  Don't start list until talloc call	
	} 																	

	void *item = malloc(size); //Malloc correct size

	Value *pointer = malloc(sizeof(Value));
	pointer->type = PTR_TYPE;
	pointer->p = item;
	Value *oldHead = usedMemHeader; // Weird property when using variable on itself

	usedMemHeader = tCons(pointer, oldHead);
	
	return item;

}
/*
	Frees memory of everything that uses talloc(), including the 
		used memory list of pointers.
*/
void tfree(){
	if((!tIsNull(usedMemHeader))){
		Value *usedMemValue = tCar(usedMemHeader);

		free(usedMemValue->p); // free value
		free(usedMemValue); 		// free active list pointer
	
		Value *temp = tCdr(usedMemHeader);

		free(usedMemHeader);
		usedMemHeader = temp;
		tfree();
	}else{
		free(usedMemHeader);
		usedMemHeader = NULL;
	}

}


/* 
	Replacement for the C function "exit", calls
	 	tfree before exiting. If an error happens,
 		you can exit your program, and all memory is automatically cleaned up.
*/
void texit(int status){
	tfree();
	exit(status);
}

