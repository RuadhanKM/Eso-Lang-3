#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "enums.h"

#define DEBUGLEVEL 1

#if DEBUGLEVEL > 1
#include <intrin.h>
#else
#define __debugbreak()
#endif // DEBUGLEVEL > 1


static void* smalloc(size_t size) {
	void* m = malloc(size);
	if (m == NULL) {
		printf("Out of memory!");
		exit(102);
	}
	return m;
}

static char* str_repeat(char* str, int times) {
	if (times < 1) return "";
	int ostrlen = strlen(str);
	int size = ostrlen * times + 1;
	char* ret = (char*) smalloc(sizeof(char) * size);
	do {
		for (int i = 0; i < ostrlen; i++) {
			ret[i + times * ostrlen] = str[i];
		}
	} while (times-- > 0);
	ret[size] = '\0';
	return ret;
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
		case TOKEN_BPR:
			return "Begin Parenthetical";
		case TOKEN_EPR:
			return "End Parenthetical";
		case TOKEN_BAR:
			return "Begin Array";
		case TOKEN_EAR:
			return "End Array";
		case TOKEN_BCB:
			return "Begin Code Block";
		case TOKEN_ECB:
			return "End Code Block";
		case TOKEN_DEF:
			return "Definition";
		case TOKEN_CON:
			return "Conditional";
		case TOKEN_RET:
			return "Return";
		case TOKEN_ARS:
			return "Array Seperator";
		default:
			return "UNKOWN / COMBINATION ";
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
	
	if (curChar == TOKENV_EQL) {
		if (nextChar == TOKEN_EQL) {
			curChar = getc(sourceFilePtr);
			return TOKEN_DEQ;
		}
		return TOKEN_EQL;
	}
	if (curChar == TOKENV_ADD) return TOKEN_ADD;
	if (curChar == TOKENV_SUB) return TOKEN_SUB;
	if (curChar == TOKENV_MUL) return TOKEN_MUL;
	if (curChar == TOKENV_DIV) return TOKEN_DIV;
	if (curChar == TOKENV_EXP) return TOKEN_EXP;
	if (curChar == TOKENV_EDL) return TOKEN_EDL;
	if (curChar == TOKENV_BPR) return TOKEN_BPR;
	if (curChar == TOKENV_EPR) return TOKEN_EPR;
	if (curChar == TOKENV_BCB) return TOKEN_BCB;
	if (curChar == TOKENV_ECB) return TOKEN_ECB;
	if (curChar == TOKENV_BAR) return TOKEN_BAR;
	if (curChar == TOKENV_EAR) return TOKEN_EAR;
	if (curChar == TOKENV_ARS) return TOKEN_ARS;

	if (curChar == TOKENV_GTT) {
		if (nextChar == TOKENV_EQL) {
			curChar = getc(sourceFilePtr);
			return TOKEN_GTE;
		}
		return TOKEN_GTT;
	}
	if (curChar == TOKENV_LST) {
		if (nextChar == TOKENV_EQL) {
			curChar = getc(sourceFilePtr);
			return TOKEN_LSE;
		}
		return TOKEN_LST;
	}

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
		if (!strcmp(tokenText, TOKENV_RET)) { free(tokenText); return TOKEN_RET; }

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

static int peekToken(FILE* sourceFilePtr, char** value) {
	int pos = ftell(sourceFilePtr);
	char* tValue = NULL;
	if (value == NULL) value = &tValue;

	int token = __nextToken(sourceFilePtr, value);
	if (DEBUGLEVEL > 2) printf("\x1b[35mNEXT TOKEN CALL: %s\n\x1b[0m", getTokenNameFromValue(token));
	if (DEBUGLEVEL > 2 && value != NULL && *value != NULL) printf("\x1b[31mNEXT TOKEN VALUE: %s\n\x1b[0m", *value);

	fseek(sourceFilePtr, pos, SEEK_SET);
	return token;
}

static int grammerDepth = 0;

static void grammerStatement(FILE* sourceFilePointer, int currentToken);

static void grammerCheck(int token, int check) {
	if (DEBUGLEVEL > 3) printf("\x1b[33mCHECKING GRAMMER: %s\n\x1b[0m", getTokenNameFromValue(check));
	if ((token & check) == 0) {
		printf("Syntax error: expected \"%s\", got \"%s\"", getTokenNameFromValue(check), getTokenNameFromValue(token));
		__debugbreak();
		exit(401);
	}
}

static void grammerMatch(FILE* sourceFilePtr, int check) {
	int token = nextToken(sourceFilePtr, NULL);
	grammerCheck(token, check);
}

static void grammerError(char* message, int token) {
	printf("Syntax error: expected \"%s\", got \"%s\"", message, getTokenNameFromValue(token));
	__debugbreak();
	exit(401);
}

static int grammerParenthasis(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER PARENTHASIS CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;
	grammerCheck(currentToken, TOKEN_BPR);
	currentToken = grammerComparison(sourceFilePtr, nextToken(sourceFilePtr, NULL));
	grammerCheck(currentToken, TOKEN_EPR);
	grammerDepth--;
	return nextToken(sourceFilePtr, NULL);
}

static int grammerArray(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER ARRAY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;
	grammerCheck(currentToken, TOKEN_BAR);
	do {
		currentToken = grammerComparison(sourceFilePtr, nextToken(sourceFilePtr, NULL));
	} while (currentToken != TOKEN_EAR);
	currentToken = nextToken(sourceFilePtr, NULL);
	grammerDepth--;
	return currentToken;
}

static int grammerCodeBlock(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER CODE BLOCK CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;
	grammerCheck(currentToken, TOKEN_BCB);
	currentToken = nextToken(sourceFilePtr, NULL);
	do {
		grammerStatement(sourceFilePtr, currentToken);
		currentToken = nextToken(sourceFilePtr, NULL);
	} while (currentToken != TOKEN_ECB);
	grammerDepth--;
	return currentToken;
}

static int grammerFunc(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER FUNC CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	grammerCheck(currentToken, TOKEN_VAR);
	currentToken = grammerArray(sourceFilePtr, nextToken(sourceFilePtr, NULL));
	grammerDepth--;
	return currentToken;
}

static int grammerPrimary(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER PRIMARY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	grammerCheck(currentToken, TOKEN_NUM | TOKEN_VAR | TOKEN_BPR | TOKEN_STR | TOKEN_BAR);

	int nt = peekToken(sourceFilePtr, NULL);
	if (currentToken == TOKEN_BPR) {
		nt = grammerParenthasis(sourceFilePtr, currentToken);
	} else if (currentToken == TOKEN_BAR) {
		nt = grammerArray(sourceFilePtr, currentToken);
	} else if (currentToken == TOKEN_VAR && nt == TOKEN_BAR) {
		nt = grammerFunc(sourceFilePtr, currentToken);
	} else {
		nt = nextToken(sourceFilePtr, NULL);
	}
	grammerDepth--;
	return nt;
}


static int grammerUnary(FILE* sourceFilePtr, int currentToken, char* currentValue) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER UNARY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	if (currentToken == TOKEN_SUB) {
		nextToken(sourceFilePtr, NULL);
	}
	currentToken = grammerPrimary(sourceFilePtr, currentToken, currentValue);
	grammerDepth--;
	return currentToken;
}

static int grammerTerm(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER TERM CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	currentToken = grammerUnary(sourceFilePtr, currentToken, NULL);
	while (currentToken == TOKEN_DIV || currentToken == TOKEN_MUL) {
		currentToken = nextToken(sourceFilePtr, NULL);
		currentToken = grammerUnary(sourceFilePtr, currentToken, NULL);
	}
	grammerDepth--;
	return currentToken;
}

static int grammerExpression(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER EXPRESSION CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	currentToken = grammerTerm(sourceFilePtr, currentToken);
	while ((currentToken & (TOKEN_ADD | TOKEN_SUB)) > 0) {
		currentToken = nextToken(sourceFilePtr, NULL);
		currentToken = grammerTerm(sourceFilePtr, currentToken);
	}
	grammerDepth--;
	return currentToken;
}

static int grammerComparison(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER COMPARISON CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	currentToken = grammerExpression(sourceFilePtr, currentToken);
	while ((currentToken & (TOKEN_DEQ | TOKEN_GTT | TOKEN_GTE | TOKEN_LST | TOKEN_LSE)) > 0) {
		currentToken = nextToken(sourceFilePtr, NULL);
		currentToken = grammerExpression(sourceFilePtr, currentToken);
	}
	grammerDepth--;
	return currentToken;
}

static void grammerStatement(FILE* sourceFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER STATEMENT CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	// Define var / function
	if (currentToken == TOKEN_DEF) {
		grammerMatch(sourceFilePtr, TOKEN_VAR);
		grammerMatch(sourceFilePtr, TOKEN_EQL);
		
		currentToken = nextToken(sourceFilePtr, NULL);
		
		// Def function
		if (currentToken == TOKEN_BCB) {
			grammerCodeBlock(sourceFilePtr, currentToken);
			currentToken = grammerArray(sourceFilePtr, nextToken(sourceFilePtr, NULL));
			grammerCheck(currentToken, TOKEN_EDL);
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mDEFINE FUNCTION\x1b[0m\n");
		}
		// Def var
		else {
			currentToken = grammerComparison(sourceFilePtr, currentToken);
			grammerCheck(currentToken, TOKEN_EDL);
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mDEFINE VAR\x1b[0m\n");
		}
		grammerDepth--;
		return;
	}
	
	// Call function
	if (currentToken == TOKEN_VAR) {
		currentToken = grammerFunc(sourceFilePtr, currentToken);
		grammerCheck(currentToken, TOKEN_EDL);
		if (DEBUGLEVEL > 0) printf("\x1b[1;36mCALL FUNCTION\x1b[0m\n");
		grammerDepth--;
		return;
	}

	// If statement
	if (currentToken == TOKEN_CON) {
		currentToken = grammerParenthasis(sourceFilePtr, nextToken(sourceFilePtr, NULL));
		grammerCodeBlock(sourceFilePtr, currentToken);
		grammerMatch(sourceFilePtr, TOKEN_EDL);
		if (DEBUGLEVEL > 0) printf("\x1b[1;36mIF STATEMENT\x1b[0m\n");
		grammerDepth--;
		return;
	}

	// Return statement
	if (currentToken == TOKEN_RET) {
		currentToken = grammerComparison(sourceFilePtr, nextToken(sourceFilePtr, NULL));
		grammerCheck(currentToken, TOKEN_EDL);
		if (DEBUGLEVEL > 0) printf("\x1b[1;36mRETURN STATEMENT\x1b[0m\n");
		grammerDepth--;
		return;
	}

	grammerError("=, if, or variable", currentToken);   
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

	char* out = "int main() {";

	// Read file
	grammerProgram(sourceFilePtr);
	fclose(sourceFilePtr);

	printf("-- FINAL PROGRAM --\n%s", out);

	return 0;
}