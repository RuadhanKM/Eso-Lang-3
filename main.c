#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "enums.h"

static char* getTokenNameFromValue(int token) {
    switch (token) {
        case TOKEN_EQL:
            return "Equals";
        case TOKEN_ADD:
            return "Add";
        case TOKEN_SUB:
            return "Subtract";
        case TOKEN_MUL:
            return "Multiply";
        case TOKEN_DIV:
            return "Divide";
        case TOKEN_EXP:
            return "Exponent";
        case TOKEN_NUM:
            return "Number";
        case TOKEN_STR:
            return "String";
        case TOKEN_VAR:
            return "Variable";
        case TOKEN_EDL:
            return "End Line";
        case TOKEN_PAR:
            return "Parenthetical";
        case TOKEN_CBL:
            return "Code Block";
        case TOKEN_DEF:
            return "Definition";
    }
}

static void grammerMatch(FILE* sourceFilePtr, int check) {
    int token = nextToken(sourceFilePtr);
    if (token != check) {
        printf("Syntax error: expected \"%s\", got \"%s\"", getTokenNameFromValue(check), getTokenNameFromValue(token));
        exit(401);
    }
}

static void grammerError(char* message, int token) {
    printf("Syntax error: expected \"%s\", got \"%s\"", message, getTokenNameFromValue(token));
    exit(401);
}

static int grammerProgram(FILE* sourceFilePtr) {
    while (!feof(sourceFilePtr)) {
        grammerStatement(sourceFilePtr, nextToken(sourceFilePtr));
    }
}

static int grammerStatement(FILE* sourceFilePtr, int currentToken) {
    // Define var / function
    if (currentToken == TOKEN_DEF) {
        grammerMatch(sourceFilePtr, TOKEN_VAR);
        grammerMatch(sourceFilePtr, TOKEN_EQL);
        currentToken = nextToken(sourceFilePtr);
        
        // Def function
        if (currentToken == TOKEN_CBL) {
            grammerMatch(sourceFilePtr, TOKEN_PAR);
            grammerMatch(sourceFilePtr, TOKEN_EDL);
            printf("DEFINE FUNCTION\n");
        }
        // Def var
        else {
            grammerExpression(sourceFilePtr, currentToken);
            grammerMatch(sourceFilePtr, TOKEN_EDL);
            printf("DEFINE VAR\n");
        }
        return 0;
    }

    // Call function
    if (currentToken == TOKEN_VAR) {
        grammerMatch(nextToken(sourceFilePtr), TOKEN_PAR);
        grammerMatch(nextToken(sourceFilePtr), TOKEN_EDL);
        printf("CALL FUNCTION\n");
        return 0;
    }

    grammerError("\"=\" or variable", currentToken);

    return 0;
}

static int grammerExpression(FILE* sourceFilePtr, int currentToken) {
    nextToken(sourceFilePtr);
    return 0;
}

static int nextToken(FILE* sourceFilePtr) {
    char curChar = getc(sourceFilePtr);
    if (curChar == EOF) return -1;
    char nextChar = ungetc(getc(sourceFilePtr), sourceFilePtr);

    while (curChar == ' ' || curChar == '\t' || curChar == '\r' || curChar == '\n') {
        curChar = getc(sourceFilePtr);
        nextChar = ungetc(getc(sourceFilePtr), sourceFilePtr);
    }
    
    if (curChar == TOKENV_EQL) return TOKEN_EQL;
    if (curChar == TOKENV_ADD) return TOKEN_ADD;
    if (curChar == TOKENV_SUB) return TOKEN_SUB;
    if (curChar == TOKENV_MUL) return TOKEN_MUL;
    if (curChar == TOKENV_DIV) return TOKEN_DIV;
    if (curChar == TOKENV_EXP) return TOKEN_EXP;
    if (curChar == TOKENV_EDL) return TOKEN_EDL;

    if (curChar == TOKENV_BST) {
        do {
            curChar = getc(sourceFilePtr);
            if (curChar == EOF) {
                printf("Parsing Error: unclosed string");
                exit(202);
                return -1;
            };
        } while (curChar != TOKENV_EST);
        return TOKEN_STR;
    }

    if (curChar == TOKENV_BPR) {
        do {
            curChar = getc(sourceFilePtr);
            if (curChar == EOF) {
                printf("Parsing Error: unclosed parentheses");
                exit(203);
                return -1;
            };
        }
        while (curChar != TOKENV_EPR);
        return TOKEN_PAR;
    }

    if (curChar == TOKENV_BCB) {
        do {
            curChar = getc(sourceFilePtr);
            if (curChar == EOF) {
                printf("Parsing Error: unclosed code block");
                exit(204);
                return -1;
            };
        } while (curChar != TOKENV_ECB);
        return TOKEN_CBL;
    }

    if (isdigit(curChar)) {
        do {
            curChar = getc(sourceFilePtr);
            nextChar = ungetc(getc(sourceFilePtr), sourceFilePtr);

            if (curChar == '.' && isdigit(nextChar)) {
                do {
                    curChar = getc(sourceFilePtr);
                } while (isdigit(curChar));

                fseek(sourceFilePtr, -1L, SEEK_CUR);
            };
        } while (isdigit(curChar));

        return TOKEN_NUM;
    }

    if (isalpha(curChar)) {
        int oldPos = ftell(sourceFilePtr)-1;

        do {
            curChar = getc(sourceFilePtr);
        } while (isalnum(curChar));

        int tokenTextSize = ftell(sourceFilePtr)-oldPos;
        
        char* tokenText = (char*)malloc(tokenTextSize*sizeof(char));
        if (tokenText == NULL) {
            printf("Out of memory!");
            exit(102);
        }
        
        fseek(sourceFilePtr, oldPos, SEEK_SET);
        for (int i=0; i<tokenTextSize-1; i++) {
            tokenText[i] = getc(sourceFilePtr);
        }
        tokenText[tokenTextSize-1] = '\0';

        if (curChar == EOF) {
            fseek(sourceFilePtr, 0L, SEEK_END);
        }

        if (!strcmp(tokenText, TOKENV_DEF)) { free(tokenText); return TOKEN_DEF; }

        free(tokenText);

        return TOKEN_VAR;
    }

    printf("Parsing Error: unkown token found - \"%c\" / \"%i\"", curChar, curChar);
    exit(201);
    return -1;
}

int main() {
    FILE* sourceFilePtr;
    
    // Open source code file
    sourceFilePtr = fopen("test.es3", "r");
    if (NULL == sourceFilePtr) {
        printf("File can't be opened");
        exit(101);
    }

    // Read file
    grammerProgram(sourceFilePtr);
    fclose(sourceFilePtr);

    return 0;
}