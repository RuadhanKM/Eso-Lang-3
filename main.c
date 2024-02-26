#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "enums.h"

int nextToken(FILE* sourceFilePtr) {
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
                exit(201);
                return -1;
            };
        } while (curChar != TOKENV_EST);
        return TOKEN_STR;
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
        
        int tokenTextSize = ftell(sourceFilePtr)-oldPos+1;
        char* tokenText = (char*)malloc(tokenTextSize*sizeof(char));
        
        fseek(sourceFilePtr, oldPos, SEEK_SET);
        for (int i=0; i<tokenTextSize-1; i++) {
            tokenText[i] = getc(sourceFilePtr);
        }
        tokenText[tokenTextSize-1] = '\0';

        printf("%s\n", tokenText);

        if (!strcmp(tokenText, TOKENV_FNC)) return TOKEN_FNC;

        return TOKEN_VAR;
    }

    printf("Parsing Error: unkown token found - \"%c\" / \"%i\"", curChar, curChar);
    exit(202);
    return -1;
}

int main() {
    FILE* sourceFilePtr;
    char ch;
    
    // Open source code file
    sourceFilePtr = fopen("test.es3", "r");
    if (NULL == sourceFilePtr) {
        printf("File can't be opened");
        exit(101);
    }

    // Read file
    while (!feof(sourceFilePtr)) {
        int token = nextToken(sourceFilePtr);
        printf("%d\n", token);
    }
    fclose(sourceFilePtr);

    return 0;
}