#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "enums.h"

#define DEBUGLEVEL 1

static void* smalloc(size_t size) {
    void* m = malloc(size);
    if (m == NULL) {
        printf("Out of memory!");
        exit(102);
    }
    return m;
}

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
        case TOKEN_ARR:
            return "Array";
        case TOKEN_CBL:
            return "Code Block";
        case TOKEN_DEF:
            return "Definition";
        case TOKEN_CON:
            return "Conditional";
        default:
            return "UNKOWN / COMBINATION";
    }
}

static int __nextToken(FILE* sourceFilePtr, char** value) {
    char curChar = getc(sourceFilePtr);
    if (curChar == EOF) return -1;
    char nextChar = ungetc(getc(sourceFilePtr), sourceFilePtr);

    while (curChar == ' ' || curChar == '\t' || curChar == '\r' || curChar == '\n') {
        curChar = getc(sourceFilePtr);
        nextChar = ungetc(getc(sourceFilePtr), sourceFilePtr);
    }

    if (DEBUGLEVEL > 4) printf("\x1b[38;5;241mCUR CHAR: %c\n\x1b[0m", curChar);
    
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

    if (curChar == TOKENV_BAR) {
        do {
            curChar = getc(sourceFilePtr);
            if (curChar == EOF) {
                printf("Parsing Error: unclosed array");
                exit(204);
                return -1;
            };
        } while (curChar != TOKENV_EAR);
        return TOKEN_ARR;
    }

    if (curChar == TOKENV_BCB) {
        do {
            curChar = getc(sourceFilePtr);
            if (curChar == EOF) {
                printf("Parsing Error: unclosed code block");
                exit(205);
                return -1;
            };
        } while (curChar != TOKENV_ECB);
        return TOKEN_CBL;
    }

    if (isdigit(curChar)) {
        int oldPos = ftell(sourceFilePtr) - 1;

        do {
            curChar = getc(sourceFilePtr);
        } while (isdigit(curChar));
        if (curChar == '.') {
            curChar = getc(sourceFilePtr);
            if (!isdigit(curChar)) {
                printf("Parsing Error: unfinished digit");
                exit(206);
                return -1;
            }
            while (isdigit(curChar)) {
                curChar = getc(sourceFilePtr);
            }
        }

        int tokenTextSize = ftell(sourceFilePtr) - oldPos;

        char* tokenText = (char*)smalloc(tokenTextSize * sizeof(char));

        fseek(sourceFilePtr, oldPos, SEEK_SET);
        for (int i = 0; i < tokenTextSize - 1; i++) {
            tokenText[i] = getc(sourceFilePtr);
        }
        tokenText[tokenTextSize - 1] = '\0';

        if (curChar == EOF) {
            fseek(sourceFilePtr, 0L, SEEK_END);
        }

        if (value != NULL) *value = tokenText; else free(tokenText);

        return TOKEN_NUM;
    }

    if (isalpha(curChar)) {
        int oldPos = ftell(sourceFilePtr)-1;

        do {
            curChar = getc(sourceFilePtr);
        } while (isalnum(curChar));

        int tokenTextSize = ftell(sourceFilePtr)-oldPos;
        
        char* tokenText = (char*)smalloc(tokenTextSize*sizeof(char));
        
        fseek(sourceFilePtr, oldPos, SEEK_SET);
        for (int i=0; i<tokenTextSize-1; i++) {
            tokenText[i] = getc(sourceFilePtr);
        }
        tokenText[tokenTextSize-1] = '\0';

        if (curChar == EOF) {
            fseek(sourceFilePtr, 0L, SEEK_END);
        }

        if (!strcmp(tokenText, TOKENV_DEF)) { free(tokenText); return TOKEN_DEF; }
        if (!strcmp(tokenText, TOKENV_CON)) { free(tokenText); return TOKEN_CON; }

        if (value != NULL) *value = tokenText; else free(tokenText);
        
        return TOKEN_VAR;
    }

    printf("Parsing Error: unkown token found - \"%c\" / \"%i\"", curChar, curChar);
    exit(201);
    return -1;
}

static int nextToken(FILE* sourceFilePtr, char** value) {
    char* tValue = NULL;
    if (value == NULL) value = &tValue;
    
    int token = __nextToken(sourceFilePtr, value);
    if (DEBUGLEVEL > 2) printf("\x1b[35mNEXT TOKEN CALL: %s\n\x1b[0m", getTokenNameFromValue(token));
    if (DEBUGLEVEL > 2 && value != NULL && *value != NULL) printf("\x1b[31mNEXT TOKEN VALUE: %s\n\x1b[0m", *value);

    return token;
}

static void grammerCheck(int token, int check) {
    if (DEBUGLEVEL > 3) printf("\x1b[33mCHECKING GRAMMER: %s\n\x1b[0m", getTokenNameFromValue(check));
    if ((token & check) == 0) {
        printf("Syntax error: expected \"%s\", got \"%s\"", getTokenNameFromValue(check), getTokenNameFromValue(token));
        exit(401);
    }
}

static void grammerMatch(FILE* sourceFilePtr, int check) {
    int token = nextToken(sourceFilePtr, NULL);
    grammerCheck(token, check);
}

static void grammerError(char* message, int token) {
    printf("Syntax error: expected \"%s\", got \"%s\"", message, getTokenNameFromValue(token));
    exit(401);
}

static void grammerFunc(FILE* sourceFilePtr, int currentToken) {
    if (DEBUGLEVEL > 1) printf("\x1b[32mGRAMMER FUNC CALL: %s\n\x1b[0m", getTokenNameFromValue(currentToken));

    grammerCheck(currentToken, TOKEN_VAR);
    grammerMatch(sourceFilePtr, TOKEN_ARR);
}

static int grammerPrimary(FILE* sourceFilePtr, int currentToken) {
    if (DEBUGLEVEL > 1) printf("\x1b[32mGRAMMER PRIMARY CALL: %s\n\x1b[0m", getTokenNameFromValue(currentToken));

    grammerCheck(currentToken, TOKEN_NUM | TOKEN_VAR | TOKEN_PAR);
    int nt = nextToken(sourceFilePtr, NULL);
    if (currentToken == TOKEN_VAR && nt == TOKEN_ARR) {
        nt = nextToken(sourceFilePtr, NULL);
    }

    return nt;
}

static int grammerUnary(FILE* sourceFilePtr, int currentToken, char* currentValue) {
    if (DEBUGLEVEL > 1) printf("\x1b[32mGRAMMER UNARY CALL: %s\n\x1b[0m", getTokenNameFromValue(currentToken));

    if (currentToken == TOKEN_SUB) {
        nextToken(sourceFilePtr, NULL);
    }
    return grammerPrimary(sourceFilePtr, currentToken, currentValue);
}

static int grammerTerm(FILE* sourceFilePtr, int currentToken) {
    if (DEBUGLEVEL > 1) printf("\x1b[32mGRAMMER TERM CALL: %s\n\x1b[0m", getTokenNameFromValue(currentToken));

    currentToken = grammerUnary(sourceFilePtr, currentToken, NULL);
    while (currentToken == TOKEN_DIV || currentToken == TOKEN_MUL) {
        currentToken = nextToken(sourceFilePtr, NULL);
        currentToken = grammerUnary(sourceFilePtr, currentToken, NULL);
    }
    return currentToken;
}

static int grammerExpression(FILE* sourceFilePtr, int currentToken) {
    if (DEBUGLEVEL > 1) printf("\x1b[32mGRAMMER EXPRESSION CALL: %s\n\x1b[0m", getTokenNameFromValue(currentToken));

    currentToken = grammerTerm(sourceFilePtr, currentToken);
    while (currentToken == TOKEN_ADD || currentToken == TOKEN_SUB) {
        currentToken = nextToken(sourceFilePtr, NULL);
        currentToken = grammerTerm(sourceFilePtr, currentToken);
    }
    return currentToken;
}


static void grammerStatement(FILE* sourceFilePtr, int currentToken) {
    if (DEBUGLEVEL > 1) printf("\x1b[32mGRAMMER STATEMENT CALL: %s\n\x1b[0m", getTokenNameFromValue(currentToken));

    // Define var / function
    if (currentToken == TOKEN_DEF) {
        grammerMatch(sourceFilePtr, TOKEN_VAR);
        grammerMatch(sourceFilePtr, TOKEN_EQL);
        
        currentToken = nextToken(sourceFilePtr, NULL);
        
        // Def function
        if (currentToken == TOKEN_CBL) {
            grammerMatch(sourceFilePtr, TOKEN_ARR);
            grammerMatch(sourceFilePtr, TOKEN_EDL);
            if (DEBUGLEVEL > 0) printf("\x1b[1;36mDEFINE FUNCTION\x1b[0m\n");
        }
        // Def var
        else {
            currentToken = grammerExpression(sourceFilePtr, currentToken);
            grammerCheck(currentToken, TOKEN_EDL);
            if (DEBUGLEVEL > 0) printf("\x1b[1;36mDEFINE VAR\x1b[0m\n");
        }
        return;
    }
    
    // Call function
    if (currentToken == TOKEN_VAR) {
        grammerFunc(sourceFilePtr, currentToken);
        grammerMatch(sourceFilePtr, TOKEN_EDL);
        if (DEBUGLEVEL > 0) printf("\x1b[1;36mCALL FUNCTION\x1b[0m\n");
        return;
    }

    // If statement
    if (currentToken == TOKEN_CON) {
        grammerMatch(sourceFilePtr, TOKEN_PAR);
        grammerMatch(sourceFilePtr, TOKEN_CBL);
        grammerMatch(sourceFilePtr, TOKEN_EDL);
        if (DEBUGLEVEL > 0) printf("\x1b[1;36mIF STATEMENT\x1b[0m\n");
        return;
    }

    grammerError("=, if, or variable", currentToken);   

    return;
}



static void grammerProgram(FILE* sourceFilePtr) {
    while (!feof(sourceFilePtr)) {
        grammerStatement(sourceFilePtr, nextToken(sourceFilePtr, NULL));
    }
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