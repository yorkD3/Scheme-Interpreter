/*NEED TO:
	-> ADD CAPSTONE WORK 
		void isBracket()
		void addOpenBracket()
		void addClosedBracket()
		void isDot()
		void addQuote()
		
	-> FIX TOKENIZATIONERROR()

	VALGRIND doesn't work on MAC, move to REPL
*/



#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

Value *list = NULL;  

/*
	Error method that prints out message and token 
*/
void tokenizingError(char *message, char *token){ //sprintf()
	char *space = talloc(2);
	space[0] = ' ';
	space[1] = '\0';

	char* errorMessage;
    
	strcpy(errorMessage, message);

	if (!strcmp(token, space)){
        sprintf(errorMessage, "%s", message);
    } else {
        sprintf(errorMessage, "%s: %s", message, token);
    }
    
    printf("Syntax Error: %s \n", errorMessage);
    texit(1);
}

/*
	Adds a token to the list of tokens
	Takes in a valueType and string to be added
*/
void addToken(valueType tokenType, char *token){
	Value *toAdd = talloc (sizeof(Value));
	toAdd->type = tokenType;

	if(tokenType == STR_TYPE || tokenType == SYMBOL_TYPE ||
		 tokenType == OPEN_TYPE || tokenType == CLOSE_TYPE || tokenType == SINGLEQUOTE_TYPE || tokenType == OPENBRACKET_TYPE || tokenType == CLOSEBRACKET_TYPE ||tokenType == DOT_TYPE){
		toAdd->s = token;
	} else if (tokenType == INT_TYPE || tokenType == BOOL_TYPE){ // CHANGE BOOL TYPES
		toAdd->i = atoi(token);
	} else if(tokenType == DOUBLE_TYPE){
		char *ptr;
		toAdd->d = strtod(token, &ptr);
	} else{
		printf("NOT OF AVAILABLE TYPES");
		toAdd->s = token;
	}
	//to output each value as it is being tokened uncomment the following line 
	list = cons(toAdd, list);
}


/*
	Skips commented scheme code
*/
void skipComment(){
	char charRead = (char)fgetc(stdin);
	while(charRead != '\n' && charRead != EOF){
		charRead = (char)fgetc(stdin);
		}
}


/*
	Adds an open parenthesis token
*/
void addOpen(){
	char *token = talloc(sizeof(char)*2);
	token[0] = '(';
	token[1] = '\0';
	addToken(OPEN_TYPE, token);
}

/*
	Adds an close parenthesis token
*/
void addClose(){
	char *token = talloc(sizeof(char)*2);
	token[0] = ')';
	token[1] = '\0';
	addToken(CLOSE_TYPE, token);
}


/*
	Adds open bracket token
*/
void addOpenBracket(){
	char *token = talloc(sizeof(char) * 2);
	token[0] = '[';
	token[1] = '\0';
	addToken(OPENBRACKET_TYPE, token);
}

/*
	Adds close bracket token
*/
void addClosedBracket(){
	char *token = talloc(sizeof(char) * 2);
	token[0] = ']';
	token[1] = '\0';
	addToken(CLOSEBRACKET_TYPE, token);
}

/*
	Adds single quote token
*/
void addQuote(){
	char *token = talloc(sizeof(char) * 6); // for eventual conversion to 'quote'
	token[0] = '\'';
	token[1] = '\0';
	addToken(SINGLEQUOTE_TYPE, token);
}

/*
	Returns 1 if character is a dotted token, 0 otherwise
*/
int isDotted(char character){
	char nextChar = (char) fgetc(stdin);
	ungetc(nextChar, stdin);
	return character == '.' && nextChar == ' ';
}

/*
	Adds dotted token
*/
void addDotted(){
	char* token = talloc(sizeof(char) * 2);
	token[0] = '.';
	token[1] = '\0';
	addToken(DOT_TYPE, token);
}




/*
	Adds an boolean token
*/
void addBoolean(){
	char *token = talloc(sizeof(char)*2);
	char character = (char) fgetc(stdin);
	//token[0] = '#';
	if(character == 'f'){
		token[0] = '0';
		token[1] = '\0';
	}else if (character == 't'){
		token[0] = '1';
		token[1] = '\0';
	} else{
		token[0] = character;
		token[1] = '\0';
		tokenizingError("Not a boolean", token); 
	}
	valueType type = BOOL_TYPE;
	addToken(type, token);
}

/*
	Checks if a token is a number baseed on the scheme grammar table
*/
bool isNumber(char first){
	char nextChar = (char)fgetc(stdin);
	ungetc(nextChar, stdin);
	if(isdigit(first) && isdigit(nextChar)){
		return 1;
	} else if((first == '+' || first == '-') && isdigit(nextChar)){
		return 1;
	}
	return 0;
}


/*
	Adds a number token based on the scheme grammar table
		Takes in an initial character
*/
void addNumber(char first){
	char *number = talloc(sizeof(char)*301);
	
	if (isdigit(first) || first == '+' || first == '-'){
		number[0] = first;
	} else {
		number[0] = first;
		number[1] = '\0'; 
		tokenizingError("Not a number", number);
	}

	char character = (char)fgetc(stdin);
	int i = 1;
	bool isDouble = 0; //false
	
	while(character != '(' && character != ' ' && character != ')' && character != '\n' && character != EOF){

		if(!isdigit(character)){
			if(character == '.'){
				isDouble = 1; 
			} else{ // not a digit or decimal
				number[i] = character;
				number[i + 1] = '\0';
				tokenizingError("Number tokens may only include digits, +, -, or decimals", number);
			}
		}
		number[i] = character;
		character = (char) fgetc(stdin);
		i++;
	}
	ungetc(character, stdin); 
	number[i] = '\0';
	
	//Add token to list
	valueType type;
	if(isDouble){type = DOUBLE_TYPE;} else{type = INT_TYPE;}
	addToken(type, number);
	
}


/*
	Adds an string token
*/
void addString(){
	char *token = talloc(sizeof(char)*301);
	token[0]= '"';
	char character = (char)fgetc(stdin);
	int i=1; 
	while(character != '"'){
		char *space = talloc(2);
		space[0] = ' ';
		space[1] = '\0';
		if(character == EOF){
			tokenizingError("File ended without closing string", space);
		}else if (character == '\n'){
			tokenizingError("Line ended without closing string", space);
		}
		token[i]=character;
		i++;
		character = (char)fgetc(stdin);
	}
	token[i]='"';
	token [i+1] = '\0';
	addToken(STR_TYPE, token);
}

/*
	Checks if character is the start of a SYMBOL_TYPE token
*/
bool isInitialSymbol(char toCompare){
	char initalSymbol[16] = {'!', '$', '%', '&', '*', '/', ':', '<', '=', '>', '?', '~', '_', '^', '-', '+'}; 
	
	for(int i = 0; i<16; i++){
		if(toCompare == initalSymbol[i]){
			return 1;
		}
	}
	return 0;
}


/*
	Checks if character is of a SYMBOL_TYPE token
*/
bool isSubsequent(char toCompare){
	return (isalnum(toCompare) || isInitialSymbol(toCompare) || toCompare == '.');
}

/*
	Checks if character is a symbol ending character
*/
bool isValidEnding(char toCompare){ // Don't worry if it is EOF
	char fineEnding[4] = {' ', '(', ')', '\n'};
	for(int i = 0; i<4; i++){
		if(toCompare == fineEnding[i]){
			return 1;
		}
	}
	return 0;
}

/*
	Determines whether a token is of SYMBOL_TYPE
*/
bool isSymbol(char first){
	char nextChar = (char)fgetc(stdin);
	ungetc(nextChar, stdin);
	if(isalnum(first) || isInitialSymbol(first)){
		if(first == '+' || first == '-'){
			if(isValidEnding(nextChar)){return 1;}
		} else {return 1;}// is letter or valid symbol
	}
	return 0;
}


/*
	Adds an symbol token
*/
void addSymbol(char first){
	char *symbol = talloc(sizeof(char)*301);
	
	symbol[0]=first;
	int i = 1;
	char character = (char)fgetc(stdin);
	while(!(isValidEnding(character) || character == EOF) ){
		if(!(isSubsequent(character))){
			symbol[i] = character;
			symbol[i +1] = '\0';
			tokenizingError("Not a valid symbol character", symbol);
		}
		symbol[i] = character;
		i++;
		character = (char)fgetc(stdin);
		
	}
	symbol[i] = '\0';
	ungetc(character, stdin);
	addToken(SYMBOL_TYPE, symbol);
}



/* 
	Reads contents of a scheme file, and returns a linked list of
	tokens.
*/
Value *tokenize(){
	// Create linked list, add each value assigning type and saving as a string
	list = makeNull();

	char character;
	character = (char)fgetc(stdin);

	while(character != EOF){ //Check tokens based on first character
		if(character == ' ' || character == '\n'){
		} else if(character == '('){
			addOpen();
		} else if(character == ')'){
			addClose();
		} else if(character == '"'){
			addString();
		} else if(character == '#'){
			addBoolean();
		} else if(character == ';'){
			skipComment();
		} else if(character == '['){
			addOpenBracket();
		} else if(character == ']'){
			addClosedBracket();
		} else if(character == '\''){
			addQuote();
		}  else if(isDotted(character)){
			addDotted();
		} else if(isdigit(character) || character == '.'){
			addNumber(character);
		} else{
			if(isNumber(character)){
				addNumber(character);
			} else if(isSymbol(character)){
				addSymbol(character);
			} else { //anything else is invalid
				char* failure = talloc(2);
				failure[0] = character;
				failure[1] = '\0';
				tokenizingError("File ended without closing string", failure);
			}
		}
		character = (char)fgetc(stdin);
	}
	Value *revList = reverse(list);
	return revList;
}




/* 
	Displays the contents of the linked list as 'token:type'
*/
void displayTokens(Value *list){
// Move through list and add symbol and token type
	if (isNull(list)){
		return;
	}
	Value *token = car(list);
	switch(car(list)->type){
		case INT_TYPE:
			printf("%i:integer \n", token->i);
			break;
		case DOUBLE_TYPE:
			printf("%f:double \n", token->d);
			break;
		case STR_TYPE:
			printf("%s:string \n", token->s);
			break;
		case OPEN_TYPE:
			printf("%s:open \n", token->s);
			break;
		case CLOSE_TYPE:
			printf("%s:close \n", token->s);
			break;
		case BOOL_TYPE:
			printf("%s:boolean \n", token->s);
			break;
		case SYMBOL_TYPE:
			printf("%s:symbol \n", token->s);
			break;
		case OPENBRACKET_TYPE:
			printf("%s:open bracket \n", token->s);
			break;
		case CLOSEBRACKET_TYPE:
			printf("%s:close bracket \n", token->s);
			break;
		case SINGLEQUOTE_TYPE:
			printf("%s:quote \n", token->s);
			break;	
		case DOT_TYPE:
			printf("%s:dot \n", token->s);
			break;
		default:
			printf("NOT A TOKEN \n");
			return;
		}
		if(list->type == CONS_TYPE){displayTokens(cdr(list));} //recursive call
}


 