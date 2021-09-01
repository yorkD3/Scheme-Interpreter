
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"


//  Where do we convert SINGLEQUOTE_TYPE -> (quote )
//		- put quote along with subtree from stack


void printToken(Value *token);
void printTree(Value *tree);

/*
    Prints an error *with scheme code* and exits the code
*/
void parseError(char* message, char* tokenOrSubtree){
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
    
    printf("Syntax Error: %s \n", errorMessage);
    texit(1);
}

/*
    Returns 1 if a token is OPEN_TYPE
*/
bool isOpen(Value *token){
    if(token->type == OPEN_TYPE){return 1;}
    return 0;
}
    
/*
    Returns 1 if a token is OPEN_TYPE
*/
bool isClosed(Value *token){
    if(token->type == CLOSE_TYPE){return 1;}
    return 0;
}


/*
    Returns 1 if a token is a subtree
*/
bool isSubtree(Value *token){
    return token->type == CONS_TYPE && car(token)->type == CONS_TYPE;
}


/*
    Pushes a token onto the stack
*/
Value *push(Value *token, Value *stack){ // return stack 
    stack = cons(token, stack);
    return stack;
}

/*
    Peeks and returns a token off of the stack
*/
Value *peek(Value *stack){ //
    if(stack->type != CONS_TYPE){
        parseError("Peeking at an empty stack", " ");
    }
    Value *peekValue = car(stack);
    // stack = cdr(stack);
    return peekValue;
}

/*
    Removes a value from the stack 
*/
Value *pop(Value *stack){
    if(stack->type != CONS_TYPE){
        parseError("Popping from an empty stack", " ");
    }
    stack = cdr(stack);
    return stack;
}


/*
    Matches closed-open parenthesis and adds subtree to stack 

		// Take in type of close to check matching parens types
*/
Value *match(Value *stack, int *depth, Value *listOfSExpressions){
    Value *subtree = makeNull();
    while (stack->type == CONS_TYPE){
        Value *toAdd = peek(stack);
        
        if (toAdd->type == OPEN_TYPE){ // End of S-Expression
            // Check parens types here 
			stack = pop(stack); 
            *depth -= 1;
            
            // if(stack->type != NULL_TYPE){printToken(peek(stack)); printf("\tyes\n");}
            Value *quoteSymbol;
            if(stack->type != NULL_TYPE && peek(stack)->type == SINGLEQUOTE_TYPE){
                    // printToken(peek(stack));
                    // printf("\tquote Sign\n");
                quoteSymbol = peek(stack);
                stack = pop(stack);
                    // printTree(stack);
                    // printf("\tStack \n");
                    subtree = cons(subtree, makeNull()); // make it a list
                    
                    subtree = cons(quoteSymbol, subtree);
                    // subtree = cons(subtree, makeNull()); 
                    
                    // printTree(subtree);// printf(" type: %u", cdr(cdr(cdr(subtree)))->type); // Cons the two together
                    // printf("%u", cdr(subtree)->type);
                    // printf("\tSubtree \n");
                // Add qyotesymbol to subtree
            } else if(listOfSExpressions->type != NULL_TYPE && peek(listOfSExpressions)->type == SINGLEQUOTE_TYPE){ // not consing as i want it to 
                // printf("quote found\t \n");
                quoteSymbol = peek(listOfSExpressions);
                listOfSExpressions = pop(listOfSExpressions);
                // subtree = cons(subtree, makeNull);
                // printToken(subtree);
                // printf("\t Before \n");
                subtree = cons(quoteSymbol, cons(subtree, makeNull()));
                // printToken(subtree);
                // printf("\t After Cons \n");
            }
            
            stack = cons(subtree, stack);
            // printToken(car(stack));
            return stack;
        }

        subtree = cons(toAdd, subtree);
        // printf("Displaying: \n");
        // display(subtree);

        stack = cdr(stack);
    }
    //Mismatching parens

    parseError("Close Parenthesis does not have a matching Open Parenthesis", " ");
    return subtree;// Useless as parseError closes the file
}


// Value *matchQuote(Value *stack, int *depth, Value *listOfSExpressions){
    
// }



/*
    Takes a list of tokens from a Scheme program, and returns a
        pointer to a parse tree representing that program.
*/
Value *parse(Value *tokens){ 
    Value *stack = makeNull();
    Value *listOfSExpressions = makeNull();
    Value *subtree;
    int *depth = talloc(sizeof(int));


    *depth = 0;
    assert(tokens != NULL && "Parse Error: Null list of tokens");
    // Move through tokens, identify their type, add to stack if not closed paren, if closed pop until open, push list back on stack
    while (tokens->type == CONS_TYPE){ // Mpre tokens
        // printf(" -Depth: %i \tStack: ", *depth);
        // display(stack);
        // printf("\n");
        Value *curToken = car(tokens);
        if (curToken->type == SINGLEQUOTE_TYPE){strcpy(curToken->s, "quote");}       /////////// BIG CHANGE, coorect way??
        if (curToken->type == CLOSE_TYPE){ 
            stack = match(stack, depth, listOfSExpressions);
            // printf("Matching\n");
            if (*depth == 0){
                subtree = peek(stack);
                stack = pop(stack);
                listOfSExpressions = cons(subtree, listOfSExpressions);
                // printf("\n");
                // printTree(listOfSExpressions);
                // printf("\n");
            }
        } else if(*depth == 0 && curToken->type != OPEN_TYPE){ 
            if (curToken->type == DOT_TYPE || curToken->type == CLOSE_TYPE){ //|| curToken->type == SINGLEQUOTE_TYPE
                parseError("Syntax Error: Not a valid symbol", " ");
            }
            // Match here too
            listOfSExpressions = cons(curToken, listOfSExpressions);

		} else {
            if(curToken->type == OPEN_TYPE){*depth += 1;}
            stack = push(curToken, stack);
        }
        tokens = cdr(tokens);
        
    }
	if (*depth != 0){parseError("Syntax Error: Extra Open parentheses", " ");}

    //No more tokens
    // printf("Here is the tree: ");
    // printTree(listOfSExpressions);
    return reverse(listOfSExpressions);
}





void printToken(Value *token){
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
            while(token->type == CONS_TYPE){
                printToken(car(token));
                token = cdr(token);
                if (token->type != NULL_TYPE){
                    printf(" ");
                } else{printf(") ");} // END OF EXPRESSION
            }
            break;
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
        default:
            printf("Bad %u\n", token->type);
    }
}



/* 
    Prints the tree to the screen in a readable fashion. It should look just like
        Scheme code; use parentheses to indicate subtrees.
*/
void printTree(Value *tree){
    if(tree == NULL){
        printf("Empty Tree");
        return;
    }
  
    Value *expression;
    while(tree->type == CONS_TYPE){
        expression = car(tree);
        if(expression->type == CONS_TYPE){ //is subtree
            printToken(expression);
        } else {
            printToken(expression);
            printf(" ");
		}
       
        tree = cdr(tree);
    }
    printf("\n");

}
