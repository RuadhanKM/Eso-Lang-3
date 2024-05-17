#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "enums.h"

#define DEBUGLEVEL 0

#if DEBUGLEVEL > 2
#include <intrin.h>
#else
#define __debugbreak()
#endif // DEBUGLEVEL > 2

/**
 * Gets and emits the next statement, expects file buffer to be pointing to before first token
 * @param sourceFilePtr - file buffer of the source code
 * @param outFilePtr - file buffer of the output
 * @param currentToken - the value of the first TOKEN_... enum in the statement
 * @param an int representing whether the program is currently (1) defining functions (0) executing code
 * @return an int representin what the new funcDefMode state should be
 */
static int grammerStatement(FILE* sourceFilePointer, FILE* outFilePtr, int currentToken, int funcDefMode);

/**
 * Gets the next comparison, expects file buffer to be pointing to before first token
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the comparison
 * @return the first TOKEN_... enum after the comparison
 */
static int grammerComparison(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken);

/**
 * Errors with message and error code
 * @param message - message in error
 * @param code - error code
 */
static void genericError(int code, const char* const message, ...) {
	va_list args;
	va_start(args, message);

	vprintf(message, args);

	va_end(args);

	__debugbreak();
	exit(code);
}

/**
 * Wrapper for malloc that exits and errors if malloc fails
 * @param size - ammount of memory to be allocted, passed to malloc
 * @return Pointer to the memory, returned from malloc
 */
static void* smalloc(size_t size) {
	void* m = malloc(size);
	if (m == NULL) {
		genericError(102, "Out of memory!");
	}
	return m;
}

/**
 * Wrapper for realloc that exits and errors if realloc fails
 * @param size - ammount of memory to be allocted, passed to malloc
 * @return Pointer to the memory, returned from malloc
 */
static void* srealloc(void* _Block, size_t size) {
	void* m = realloc(_Block, size);
	if (m == NULL) {
		genericError(102, "Out of memory!");
	}
	return m;
}


/**
 * Allocates a new string that is the str repeated by times
 * @param str - char string to be repeated
 * @param times - the number of times to repeat str
 * @return The new string, must be freed
 */
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

/**
 * Returns a string literal representation of a token
 * @param token - a TOKEN_... enum
 * @return The string literal, does not need to be freed
 */
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

/**
 * Gets the next token, comusuming the reqired chars from sourceFilePtr
 * @param sourceFilePtr - file buffer of the source code
 * @param OUT value - if present, sets this variable to the value of the token, (e.g. variable name, string value, number value, ect.,) otherwise returns NULL
 * @return The TOKEN_... enum
 */
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
		int oldPos = ftell(sourceFilePtr);

		do {
			curChar = getc(sourceFilePtr);
			if (curChar == EOF) {
				genericError(202, "Parsing Error: unclosed string!");
				return -1;
			};
		} while (curChar != TOKENV_EST);

		int tokenTextSize = ftell(sourceFilePtr)-1 - oldPos;
		fseek(sourceFilePtr, oldPos, SEEK_SET);
		char* tokenText = (char*)smalloc((tokenTextSize+3) * sizeof(char));
		tokenText[0] = '"';

		for (int i = 0; i < tokenTextSize; i++) {
			tokenText[i+1] = getc(sourceFilePtr);
		}
		fseek(sourceFilePtr, 1L, SEEK_CUR);
		tokenText[tokenTextSize + 1] = '"';
		tokenText[tokenTextSize + 2] = '\0';
		if (value != NULL) *value = tokenText;

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

/**
 * Wrapper for __nextToken() that prints debug messages
 * @param sourceFilePtr - file buffer of the source code
 * @param OUT value - if present, sets this variable to the value of the token, (e.g. variable name, string value, number value, ect.,) otherwise returns NULL
 * @return The TOKEN_... enum
 */
static int nextToken(FILE* sourceFilePtr, char** value) {
	char* tValue = NULL;
	if (value == NULL) value = &tValue;
	
	int token = __nextToken(sourceFilePtr, value);
	if (DEBUGLEVEL > 2) printf("\x1b[35mNEXT TOKEN CALL: %s\n\x1b[0m", getTokenNameFromValue(token));
	if (DEBUGLEVEL > 2 && value != NULL && *value != NULL) printf("\x1b[31mNEXT TOKEN VALUE: %s\n\x1b[0m", *value);

	return token;
}

/**
 * Wrapper for __nextToken() that prints debug messages but does not consume chars from the file buffer
 * @param sourceFilePtr - file buffer of the source code
 * @param OUT value - if present, sets this variable to the value of the token, (e.g. variable name, string value, number value, ect.,) otherwise returns NULL
 * @param count - number of tokens to peek 
 @return The TOKEN_... enum
 */
static int peekToken(FILE* sourceFilePtr, char** value, int count) {
	int pos = ftell(sourceFilePtr);

	for (int i = 0; i < count - 1; i++) __nextToken(sourceFilePtr, NULL);

	char* tValue = NULL;
	if (value == NULL) value = &tValue;

	int token = __nextToken(sourceFilePtr, value);
	if (DEBUGLEVEL > 2) printf("\x1b[35mNEXT PEEK TOKEN CALL: %s\n\x1b[0m", getTokenNameFromValue(token));
	if (DEBUGLEVEL > 2 && value != NULL && *value != NULL) printf("\x1b[31mNEXT PEEK TOKEN VALUE: %s\n\x1b[0m", *value);

	fseek(sourceFilePtr, pos, SEEK_SET);
	return token;
}

static int grammerDepth = 0;

/**
 * Checks if token == check and errors if not. Bitwise or multiple tokens in check to check multiple values
 * @param token - the TOKEN_... enum to check
 * @param check - the TOKEN_... enum to check against
 */
static void grammerCheck(int token, int check) {
	char* combinedTokenString = smalloc(sizeof(char));
	combinedTokenString[0] = '\0';
	int checkPassed = 0;

	for (int i = 0; i < NUM_TOKENS; i++) {
		if ((check >> i & 1) == 1) {
			if ((token >> i & 1) == 1) {
				checkPassed = 1;
			}

			int firstInstance = strlen(combinedTokenString) == 0;

			const char* tokenString = getTokenNameFromValue(1 << i);
			char* tempCts = srealloc(
				combinedTokenString, 
				sizeof(char) * (strlen(combinedTokenString) + strlen(tokenString) + (firstInstance ? 1 : 3))
			);

			combinedTokenString = tempCts;

			if (!firstInstance) strcat(combinedTokenString, ", ");
			strcat(combinedTokenString, tokenString);
		}
	}

	if (DEBUGLEVEL > 3) printf("\x1b[33mCHECKING GRAMMER: %s\n\x1b[0m", combinedTokenString);

	if (!checkPassed) genericError(401, "Syntax error: expected \"%s\", got \"%s\"", combinedTokenString, getTokenNameFromValue(token));
	free(combinedTokenString);
	return;
}

/**
 * Wrapper for grammerCheck that first calls nextToken() and passes that to token
 * @param sourceFilePtr - file buffer of the source code
 * @param check - the TOKEN_... enum to check against
 * @return the TOKEN_... enum of the next token
 */
static int grammerMatch(FILE* sourceFilePtr, int check) {
	int token = nextToken(sourceFilePtr, NULL);
	grammerCheck(token, check);
	return token;
}

/**
 * Wrapper for grammerCheck that first calls peekToken() and passes that to token
 * @param sourceFilePtr - file buffer of the source code
 * @param check - the TOKEN_... enum to check against
 * @param count - the number of tokens ahead to check
 * @return the TOKEN_... enum of the next token
 */
static int grammerPeekMatch(FILE* sourceFilePtr, int check, int count) {
	int token = peekToken(sourceFilePtr, NULL, 1);
	grammerCheck(token, check);
	return token;
}

/**
 * Errors with message, which should be in the format "Syntax error: expected "[message]", got "[TOKEN_... enum]"
 * @param message - message in error
 * @param token - token in error
 */
static void grammerError(char* message, int token) {
	genericError("Syntax error: expected \"%s\", got \"%s\"", message, getTokenNameFromValue(token));
}

/**
 * Gets the next parenthasis
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the parenthasis
 * @return the first TOKEN_... enum after the parenthasis
 */
static int grammerParenthasis(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER PARENTHASIS CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	fputs("(", outFilePtr);

	grammerCheck(currentToken, TOKEN_BPR);
	grammerComparison(sourceFilePtr, outFilePtr, currentToken);
	grammerMatch(sourceFilePtr, TOKEN_EPR);

	fputs(")", outFilePtr);

	grammerDepth--;
	return TOKEN_EPR;
}

/**
 * Gets the next array
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the array
 * @return the first TOKEN_... enum after the array
 */
static int grammerArray(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken, int paramLike, int paramDefLike) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER ARRAY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	if (paramLike) fputs("(", outFilePtr); else fputs("{ ", outFilePtr);

	if (peekToken(sourceFilePtr, NULL, 1) != TOKEN_EAR && paramLike && paramDefLike) fputs("int ", outFilePtr);

	grammerCheck(currentToken, TOKEN_BAR);
	currentToken = grammerComparison(sourceFilePtr, outFilePtr, currentToken);
	while (nextToken(sourceFilePtr, NULL) != TOKEN_EAR) {
		fputs(", ", outFilePtr);
		if (paramLike && paramDefLike) fputs("int ", outFilePtr);
		currentToken = grammerComparison(sourceFilePtr, outFilePtr, currentToken);
	}

	if (paramLike) fputs(")", outFilePtr); else fputs(" }", outFilePtr);

	grammerDepth--;
	return TOKEN_EAR;
}

/**
 * Gets the next code block
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the code block
 * @return the first TOKEN_... enum after the code block
 */
static void grammerCodeBlock(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER CODE BLOCK CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;
	
	fputs("{\n", outFilePtr);
	grammerCheck(currentToken, TOKEN_BCB);
	do {
		grammerStatement(sourceFilePtr, outFilePtr, NULL, 0);
	} while (peekToken(sourceFilePtr, NULL, 1) != TOKEN_ECB);
	fputs("}\n", outFilePtr);

	grammerDepth--;
}

/**
 * Gets the next function call
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the function call
 * @return the first TOKEN_... enum after the function call
 */
static int grammerFunc(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER FUNC CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	// | testFunc[1, 2, 3]
	char* funcName = NULL;
	// testFunc | [1, 2, 3]
	currentToken = nextToken(sourceFilePtr, &funcName);
	fputs(funcName, outFilePtr);
	
	grammerCheck(currentToken, TOKEN_VAR);
	
	grammerArray(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL), 1, 0);

	grammerDepth--;
	return currentToken;
}

/**
 * Gets the next primary
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the primary
 * @return the first TOKEN_... enum after the primary
 */
static int grammerPrimary(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER PRIMARY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* potVarVal = NULL;
	int nToken = peekToken(sourceFilePtr, &potVarVal, 1);
	grammerCheck(nToken, TOKEN_NUM | TOKEN_VAR | TOKEN_BPR | TOKEN_STR | TOKEN_BAR);
	int n2Token = peekToken(sourceFilePtr, NULL, 2);

	if (nToken == TOKEN_BPR) {
		currentToken = nextToken(sourceFilePtr, NULL);
		currentToken = grammerParenthasis(sourceFilePtr, outFilePtr, currentToken);
	} else if (nToken == TOKEN_BAR) {
		currentToken = nextToken(sourceFilePtr, NULL);
		currentToken = grammerArray(sourceFilePtr, outFilePtr, currentToken, 0, 0);
	} else if (nToken == TOKEN_VAR && n2Token == TOKEN_BAR) {
		currentToken = grammerFunc(sourceFilePtr, outFilePtr, currentToken);
	} else {
		char* varVal = NULL;
		currentToken = nextToken(sourceFilePtr, &varVal);
		if (varVal == NULL) genericError(901, "Failed to parse var value!");

		fputs(varVal, outFilePtr);
		if (nToken == TOKEN_VAR) fputs("__raw", outFilePtr);

		free(varVal);
	}

	grammerDepth--;
	return currentToken;
}

/**
 * Gets the next unary
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the unary
 * @return the first TOKEN_... enum after the unary
 */
static int grammerUnary(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER UNARY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	if (peekToken(sourceFilePtr, NULL, 1) == TOKEN_SUB) {
		nextToken(sourceFilePtr, NULL);
		fputs("-", outFilePtr);
	}
	currentToken = grammerPrimary(sourceFilePtr, outFilePtr, currentToken);

	grammerDepth--;
	return currentToken;
}

/**
 * Gets the next term
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the term
 * @return the first TOKEN_... enum after the term
 */
static int grammerTerm(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER TERM CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	currentToken = grammerUnary(sourceFilePtr, outFilePtr, currentToken);
	int pToken = peekToken(sourceFilePtr, NULL, 1);
	while ((pToken & (TOKEN_MUL | TOKEN_DIV)) > 0) {
		fputs(pToken == TOKEN_MUL ? " * " : " / ", outFilePtr);
		currentToken = grammerUnary(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));
		pToken = peekToken(sourceFilePtr, NULL, 1);
	}

	grammerDepth--;
	return currentToken;
}

/**
 * Gets the next expression
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the expression
 * @return the first TOKEN_... enum after the expression
 */
static int grammerExpression(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER EXPRESSION CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	currentToken = grammerTerm(sourceFilePtr, outFilePtr, currentToken);
	int pToken = peekToken(sourceFilePtr, NULL, 1);
	while ((pToken & (TOKEN_ADD | TOKEN_SUB)) > 0) {
		fputs(pToken == TOKEN_ADD ? " + " : " - ", outFilePtr);
		currentToken = grammerTerm(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));
		pToken = peekToken(sourceFilePtr, NULL, 1);
	}

	grammerDepth--;
	return currentToken;
}

static int grammerComparison(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER COMPARISON CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	currentToken = grammerExpression(sourceFilePtr, outFilePtr, currentToken);
	int pToken = peekToken(sourceFilePtr, NULL, 1);
	while ((pToken & (TOKEN_DEQ | TOKEN_GTT | TOKEN_GTE | TOKEN_LST | TOKEN_LSE)) > 0) {
		if (pToken == TOKEN_DEQ) fputs(" == ", outFilePtr); else
		if (pToken == TOKEN_GTT) fputs(" > ", outFilePtr); else
		if (pToken == TOKEN_GTE) fputs(" >= ", outFilePtr); else
		if (pToken == TOKEN_LST) fputs(" < ", outFilePtr); else
		if (pToken == TOKEN_LSE) fputs(" <= ", outFilePtr);

		currentToken = grammerExpression(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));
		pToken = peekToken(sourceFilePtr, NULL, 1);
	}

	grammerDepth--;
	return currentToken;
}

static int grammerStatement(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken, int funcDefMode) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER STATEMENT CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* iVarVal = NULL;
	currentToken = peekToken(sourceFilePtr, &iVarVal, 1);

	grammerCheck(currentToken, TOKEN_DEF | TOKEN_VAR | TOKEN_CON | TOKEN_RET);

	// Define var / function
	if (currentToken == TOKEN_DEF) {
		nextToken(sourceFilePtr, NULL);

		char* potVarName = NULL;
		int varToken = nextToken(sourceFilePtr, &potVarName);
		if (potVarName == NULL) genericError(901, "Failed to parse function/var name");

		grammerCheck(varToken, TOKEN_VAR);
		currentToken = grammerMatch(sourceFilePtr, TOKEN_EQL | TOKEN_BAR);
		
		// Def function
		if (currentToken == TOKEN_BAR) {
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mDEFINE FUNCTION\x1b[0m\n");

			if (!funcDefMode) genericError(903, "Function defined not at top of file!");
			fputs("static void ", outFilePtr);
			fputs(potVarName, outFilePtr);

			free(potVarName);

			currentToken = grammerArray(sourceFilePtr, outFilePtr, currentToken, 1, 1);
			grammerMatch(sourceFilePtr, TOKEN_EQL);
			grammerCodeBlock(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));

			nextToken(sourceFilePtr, NULL);
			grammerMatch(sourceFilePtr, TOKEN_EDL);

			grammerDepth--;
			return 1;
		}
		// Def var
		else {
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mDEFINE VAR\x1b[0m\n");

			int peekType = 0;
			int i = 1;

			char* potVarTypeValue = NULL;
			while ((peekType & (TOKEN_STR | TOKEN_NUM | TOKEN_BAR | TOKEN_VAR)) == 0) {
				if (potVarTypeValue != NULL) free(potVarTypeValue);
				potVarTypeValue = NULL;

				peekType = peekToken(sourceFilePtr, &potVarTypeValue, i);

				i++;
			}

			const char* varType;
			switch (peekType) {
				case TOKEN_STR:
					varType = "char* ";
					break;
				case TOKEN_NUM:
					varType = "double ";
					break;
				case TOKEN_BAR:
					varType = "int ";
					break;
				case TOKEN_VAR:
					varType = strcat(srealloc(potVarTypeValue, strlen(potVarTypeValue) + 9), "__type ");
					break;
				default:
					genericError(904, "Failed to parse var type!");
					break;
			}
			
			fputs(varType, outFilePtr);
			fputs(potVarName, outFilePtr);
			fputs("__raw", outFilePtr);
			fputs(" = ", outFilePtr);

			currentToken = grammerComparison(sourceFilePtr, outFilePtr, currentToken);

			fputs("; typedef ", outFilePtr);
			fputs(varType, outFilePtr);
			fputs(potVarName, outFilePtr);
			fputs("__type;\n", outFilePtr);

			free(potVarName);
			if (potVarTypeValue != NULL) free(potVarTypeValue);

			grammerMatch(sourceFilePtr, TOKEN_EDL);
		}

		grammerDepth--;
		return 0;
	}
	
	// Call function / Redefine var
	if (currentToken == TOKEN_VAR) {
		// | a -> = <- 12;
		// | a -> [ <- 1, 2, 3];
		int pToken = peekToken(sourceFilePtr, NULL, 2);

		grammerCheck(pToken, TOKEN_EQL | TOKEN_BAR);

		// Redefine var
		if (pToken == TOKEN_EQL) {
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mREDEFINE VAR\x1b[0m\n");
			grammerMatch(sourceFilePtr, TOKEN_VAR); // a | = 12;
			grammerMatch(sourceFilePtr, TOKEN_EQL); // a = | 12;
			fputs(iVarVal, outFilePtr);
			fputs(" = ", outFilePtr);
			currentToken = grammerComparison(sourceFilePtr, outFilePtr, TOKEN_EQL);
			fputs(";\n", outFilePtr);
			grammerMatch(sourceFilePtr, TOKEN_EDL);
		} 
		// Call function
		else { 
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mCALL FUNCTION\x1b[0m\n");
			grammerFunc(sourceFilePtr, outFilePtr, currentToken);
			grammerMatch(sourceFilePtr, TOKEN_EDL);
			fputs(";\n", outFilePtr);
		}

		grammerDepth--;
		return 0;
	}

	// If statement
	if (currentToken == TOKEN_CON) {
		if (DEBUGLEVEL > 0) printf("\x1b[1;36mIF STATEMENT\x1b[0m\n");

		fputs("if ", outFilePtr);

		// | if (a > b) { ... };
		nextToken(sourceFilePtr, NULL);
		// if | (a > b) { ... };
		currentToken = grammerParenthasis(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));
		currentToken = nextToken(sourceFilePtr, NULL);
		// if (a > b) | { ... };
		grammerCodeBlock(sourceFilePtr, outFilePtr, currentToken);
		// if (a > b) { ... | };
		grammerMatch(sourceFilePtr, TOKEN_ECB);
		grammerMatch(sourceFilePtr, TOKEN_EDL);

		grammerDepth--;
		return 0;
	}

	// Return statement
	if (currentToken == TOKEN_RET) {
		if (DEBUGLEVEL > 0) printf("\x1b[1;36mRETURN\x1b[0m\n");

		nextToken(sourceFilePtr, NULL);

		fputs("return ", outFilePtr);

		grammerComparison(sourceFilePtr, outFilePtr, currentToken);
		grammerMatch(sourceFilePtr, TOKEN_EDL);

		fputs(";\n", outFilePtr);

		grammerDepth--;
		return 0;
	}
}

/**
 * Begins parsing the program
 * @param sourceFilePtr - file buffer of the source code
 * @return whether the program had any executable code (main function)
 */
static int grammerProgram(FILE* sourceFilePtr, FILE* outFilePtr) {
	int funcDefMode = 1;
	while (!feof(sourceFilePtr)) {
		int oldSourcePos = ftell(sourceFilePtr);
		int oldOutPos = ftell(outFilePtr);

		if (!grammerStatement(sourceFilePtr, outFilePtr, NULL, funcDefMode)) {
			if (funcDefMode) {
				fseek(sourceFilePtr, oldSourcePos, SEEK_SET);
				fseek(outFilePtr, oldOutPos, SEEK_SET);
				fputs("int main() {\n", outFilePtr);
			}
			funcDefMode = 0;
		}
	}
	return funcDefMode;
}

int main(int argc) {
	argc = 2;
	char* argv[2] = {"es3.exe", "test.es3"};

	if (argc < 2) genericError(100, "Too few arguments! Usage: es3 fileIn.es3 [fileOut]");
	if (argc > 3) genericError(100, "Too many arguments! Usage: es3 fileIn.es3 [fileOut]");

	FILE* sourceFilePtr;
	FILE* outFilePtr;

	char* outFileName = argc == 3 ? argv[2] : "out";
	char* outTransName = smalloc(sizeof(char) * (3 * strlen(outFileName)));
	char* outCompName = smalloc(sizeof(char) * (5 * strlen(outFileName)));
	sprintf(outTransName, "%s.c", outFileName);
	sprintf(outCompName, "%s.exe", outFileName);

	// Open source code file
	sourceFilePtr = fopen(argv[1], "r");
	// Open out code file
	outFilePtr = fopen(outTransName, "w");

	if (sourceFilePtr == NULL || outFilePtr == NULL) {
		printf("File can't be opened");
		exit(101);
	}

	fputs("#include <stdio.h>\n", outFilePtr);

	// Read file
	if (!grammerProgram(sourceFilePtr, outFilePtr)) {
		fputs("return 0;\n}", outFilePtr);
	}

	fclose(sourceFilePtr);
	fclose(outFilePtr);

	char* actualpath = _fullpath(NULL, outTransName, 260);
	char* command = (char*) smalloc(sizeof(char) * (9 + strlen(actualpath) + strlen(outCompName)));

	sprintf(command, "gcc %s -o %s", actualpath, outCompName);
	printf("%s\r\n", command);
	system(command);

	free(actualpath);

	actualpath = _fullpath(NULL, outCompName, 260);
	printf("%s\r\n", actualpath);
	if (DEBUGLEVEL == 0) system(actualpath);

	free(command);
	free(actualpath);

	if (DEBUGLEVEL == 0) unlink(outTransName);

	return 0;
}