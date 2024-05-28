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
static char* grammerComparison(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken);

/**
 * Errors with message and error code
 * @param message - message in error
 * @param code - error code
 */
static void genericError(FILE* sourceFilePtr, int code, const char* const message, ...) {
	va_list args;
	va_start(args, message);

	if (sourceFilePtr != NULL) {
		int lineNum = 0;
		fseek(sourceFilePtr, 1L, SEEK_CUR);
		while (ftell(sourceFilePtr) > 1) {
			fseek(sourceFilePtr, -1L, SEEK_CUR);
			char c = ungetc(getc(sourceFilePtr), sourceFilePtr);
			if (c == '\n') lineNum++;
		}

		printf("Error on line %i: ", lineNum+1);
	}

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
		genericError(NULL, 102, "Out of memory!");
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
		free(m);
		genericError(NULL, 102, "Out of memory!");
	}
	_Block = m;
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
 * Wrapper for strcat that resizes the string to fit before calling
 * @param destination - original string
 * @param source - string to add
 * @return The new string, must be freed
 */
static char* sstrcat(char* destination, const char* source) {
	int size = strlen(destination) + strlen(source) + 1;
	destination = srealloc(destination, size);
	strcat(destination, source);
	return destination;
}

/**
 * Resizes destination to fit both strings and prepends source to destination
 * @param destination - original string
 * @param source - string to add
 * @return The new string, must be freed
 */
static char* sstrpre(char* destination, const char* source) {
	size_t len = strlen(source);
	destination = srealloc(destination, strlen(destination) + len + 1);
    memmove(destination + len, destination, strlen(destination) + 1);
	destination = memcpy(destination, source, len);
	return destination;
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
		case TOKEN_DEQ:
			return "Double Equals";
		case TOKEN_GTT:
			return "Greater Than";
		case TOKEN_GTE:
			return "Greater Than or Equals To";
		case TOKEN_LST:
			return "Less Than";
		case TOKEN_LSE:
			return "Less Then or Equals To";
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
		case TOKEN_LOP:
			return "Loop";
		case TOKEN_EOF:
			return "End of File";
		case TOKEN_TRU:
			return "True";
		case TOKEN_FLS:
			return "False";
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
	if (curChar == EOF) return TOKEN_EOF;
	char nextChar = ungetc(getc(sourceFilePtr), sourceFilePtr);

	while (curChar == ' ' || curChar == '\t' || curChar == '\r' || curChar == '\n') {
		curChar = getc(sourceFilePtr);
		nextChar = ungetc(getc(sourceFilePtr), sourceFilePtr);
	}
	if (DEBUGLEVEL > 4) printf("\x1b[38;5;241mCUR CHAR: %c, NEXT CHAR: %c\n\x1b[0m", curChar, nextChar);
	if (curChar == EOF) return TOKEN_EOF;
	
	if (curChar == TOKENV_EQL) {
		if (nextChar == TOKENV_EQL) {
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
				genericError(sourceFilePtr, 202, "Parsing Error: unclosed string!");
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
				genericError(sourceFilePtr, 206, "Parsing Error: unfinished digit");
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
		if (!strcmp(tokenText, TOKENV_LOP)) { free(tokenText); return TOKEN_LOP; }
		if (!strcmp(tokenText, TOKENV_TRU)) { free(tokenText); return TOKEN_TRU; }
		if (!strcmp(tokenText, TOKENV_FLS)) { free(tokenText); return TOKEN_FLS; }

		if (value != NULL) *value = tokenText; else free(tokenText);
		
		return TOKEN_VAR;
	}

	genericError(sourceFilePtr, 201, "Parsing Error: unkown token found - \"%c\" / \"%i\"\n", curChar, curChar);
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
static void grammerCheck(FILE* sourceFilePtr, int token, int check) {
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

	if (!checkPassed) genericError(sourceFilePtr, 401, "Syntax error: expected \"%s\", got \"%s\"\n", combinedTokenString, getTokenNameFromValue(token));
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
	grammerCheck(sourceFilePtr, token, check);
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
	grammerCheck(sourceFilePtr, token, check);
	return token;
}

/**
 * Errors with message, which should be in the format "Syntax error: expected "[message]", got "[TOKEN_... enum]"
 * @param message - message in error
 * @param token - token in error
 */
static void grammerError(FILE* sourceFilePtr, char* message, int token) {
	genericError(sourceFilePtr, "Syntax error: expected \"%s\", got \"%s\"\n", message, getTokenNameFromValue(token));
}

/**
 * Gets the next parenthasis
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the parenthasis
 * @return the first TOKEN_... enum after the parenthasis
 */
static char* grammerParenthasis(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER PARENTHASIS CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* outPar = smalloc(1);
	outPar[0] = '\0';
	outPar = sstrcat(outPar, "(");

	grammerCheck(sourceFilePtr, currentToken, TOKEN_BPR);

	char* inComp = grammerComparison(sourceFilePtr, outFilePtr, currentToken);
	outPar = sstrcat(outPar, inComp);
	free(inComp);

	grammerMatch(sourceFilePtr, TOKEN_EPR);

	outPar = sstrcat(outPar, ")");

	grammerDepth--;
	return outPar;
}

/**
 * Gets the next array
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the array
 * @return the first TOKEN_... enum after the array
 */
static char* grammerArray(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken, int paramLike, int paramDefLike) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER ARRAY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* primOut = smalloc(1);
	primOut[0] = '\0';

	if (paramLike) primOut = sstrcat(primOut, "("); else primOut = sstrcat(primOut, "{");

	if (peekToken(sourceFilePtr, NULL, 1) != TOKEN_EAR && paramLike && paramDefLike) primOut = sstrcat(primOut, "ES3Var ");

	grammerCheck(sourceFilePtr, currentToken, TOKEN_BAR);
	
	char* compIn;

	if (paramDefLike) {
		char* paramNameIn = NULL;
		nextToken(sourceFilePtr, &paramNameIn);
		if (paramNameIn == NULL) genericError(sourceFilePtr, 905, "Invalid function parameter!");
		primOut = sstrcat(primOut, paramNameIn);
		primOut = sstrcat(primOut, "__raw");
		free(paramNameIn);
	} else {
		compIn = grammerComparison(sourceFilePtr, outFilePtr, currentToken);
		primOut = sstrcat(primOut, compIn);
		free(compIn);
	}

	while (nextToken(sourceFilePtr, NULL) != TOKEN_EAR) {
		primOut = sstrcat(primOut, ", ");
		if (paramLike && paramDefLike) primOut = sstrcat(primOut, "ES3Var ");
		if (paramDefLike) {
			char* paramNameIn = NULL;
			nextToken(sourceFilePtr, &paramNameIn);
			if (paramNameIn == NULL) genericError(sourceFilePtr, 905, "Invalid function parameter!");
			primOut = sstrcat(primOut, paramNameIn);
			primOut = sstrcat(primOut, "__raw");
			free(paramNameIn);
		} else {
			compIn = grammerComparison(sourceFilePtr, outFilePtr, currentToken);
			primOut = sstrcat(primOut, compIn);
			free(compIn);
		}
	}

	if (paramLike) primOut = sstrcat(primOut, ")"); else primOut = sstrcat(primOut, "}");

	grammerDepth--;
	return primOut;
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
	grammerCheck(sourceFilePtr, currentToken, TOKEN_BCB);
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
static char* grammerFunc(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER FUNC CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* primOut = smalloc(1);
	primOut[0] = '\0';

	// | testFunc[1, 2, 3]
	char* funcName = NULL;
	// testFunc | [1, 2, 3]
	currentToken = nextToken(sourceFilePtr, &funcName);
	primOut = sstrcat(primOut, funcName);
	primOut = sstrcat(primOut, "__raw");
	
	grammerCheck(sourceFilePtr, currentToken, TOKEN_VAR);
	
	char* arrIn = grammerArray(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL), 1, 0);
	primOut = sstrcat(primOut, arrIn);
	free(arrIn);

	grammerDepth--;
	return primOut;
}

/**
 * Gets the next primary
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the primary
 * @return the first TOKEN_... enum after the primary
 */
static char* grammerPrimary(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER PRIMARY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* potVarVal = NULL;
	int nToken = peekToken(sourceFilePtr, &potVarVal, 1);
	grammerCheck(sourceFilePtr, nToken, TOKEN_NUM | TOKEN_VAR | TOKEN_BPR | TOKEN_STR | TOKEN_BAR | TOKEN_TRU | TOKEN_FLS);
	int n2Token = peekToken(sourceFilePtr, NULL, 2);

	if (nToken == TOKEN_BPR) {
		currentToken = nextToken(sourceFilePtr, NULL);
		return grammerParenthasis(sourceFilePtr, outFilePtr, currentToken);
	} else if (nToken == TOKEN_BAR) {
		currentToken = nextToken(sourceFilePtr, NULL);
		return grammerArray(sourceFilePtr, outFilePtr, currentToken, 0, 0);
	} else if (nToken == TOKEN_VAR && n2Token == TOKEN_BAR) {
		return grammerFunc(sourceFilePtr, outFilePtr, currentToken);
	} else {
		char* varVal = smalloc(1);
		varVal[0] = '\0';

		currentToken = nextToken(sourceFilePtr, &varVal);
		if (varVal == NULL) genericError(sourceFilePtr, 901, "Failed to parse var value!");

		switch (currentToken) {
			case TOKEN_NUM:
				varVal = sstrpre(varVal, "(ES3Var) { .type = 1, .valNum = ");
				varVal = sstrcat(varVal, " }");
				break;
			case TOKEN_STR:
				varVal = sstrpre(varVal, "(ES3Var) { .type = 2, .valString = ");
				varVal = sstrcat(varVal, " }");
				break;
			case TOKEN_VAR:
				varVal = sstrcat(varVal, "__raw");
				break;
			case TOKEN_TRU:
				varVal = sstrcat(varVal, "(ES3Var) { .type = 3, .valBool = 1 }");
				break;
			case TOKEN_FLS:
				varVal = sstrcat(varVal, "(ES3Var) { .type = 3, .valBool = 0 }");
				break;
		
			default:
				genericError(sourceFilePtr, 902, "Failed to parse var type!");
				break;
		}

		return varVal;
		grammerDepth--;
	}
}

/**
 * Gets the next unary
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the unary
 * @return the first TOKEN_... enum after the unary
 */
static char* grammerUnary(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER UNARY CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* outExpr = (char*) smalloc(1);
	int isUnary = 0;

	if (peekToken(sourceFilePtr, NULL, 1) == TOKEN_SUB) {
		nextToken(sourceFilePtr, NULL);
		outExpr = sstrpre(outExpr, "esvUnary(");
		isUnary = 1;
	}

	outExpr[0] = '\0';
	char* inExp = grammerPrimary(sourceFilePtr, outFilePtr, currentToken);
	outExpr = sstrcat(outExpr, inExp);
	free(inExp);

	if (isUnary) outExpr = sstrcat(outExpr, ")");

	grammerDepth--;
	return outExpr;
}

/**
 * Gets the next expression
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the expression
 * @return A string with the transpiled source code for the expression
 */
static char* grammerExponentiation(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER EXPONENTIATION CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* outExpr = (char*) smalloc(1);
	outExpr[0] = '\0';
	char* inExp = grammerUnary(sourceFilePtr, outFilePtr, currentToken);
	outExpr = sstrcat(outExpr, inExp);

	int pToken = peekToken(sourceFilePtr, NULL, 1);
	while ((pToken & (TOKEN_EXP)) > 0) {
		if (pToken == TOKEN_EXP) outExpr = sstrcat(outExpr, ", 1, ");

		free(inExp);
		inExp = grammerUnary(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));

		outExpr = sstrpre(outExpr, "esvExpo(");
		outExpr = sstrcat(outExpr, inExp);
		outExpr = sstrcat(outExpr, ")");

		pToken = peekToken(sourceFilePtr, NULL, 1);
	}

	free(inExp);

	grammerDepth--;
	return outExpr;
}

/**
 * Gets the next term
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the term
 * @return the first TOKEN_... enum after the term
 */
static char* grammerTerm(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER TERM CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* outExpr = (char*) smalloc(1);
	outExpr[0] = '\0';
	char* inExp = grammerExponentiation(sourceFilePtr, outFilePtr, currentToken);
	outExpr = sstrcat(outExpr, inExp);

	int pToken = peekToken(sourceFilePtr, NULL, 1);
	while ((pToken & (TOKEN_MUL | TOKEN_DIV)) > 0) {
		if (pToken == TOKEN_MUL) outExpr = sstrcat(outExpr, ", 1, "); else
		if (pToken == TOKEN_DIV) outExpr = sstrcat(outExpr, ", 2, ");

		free(inExp);
		inExp = grammerExponentiation(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));

		outExpr = sstrpre(outExpr, "esvTerm(");
		outExpr = sstrcat(outExpr, inExp);
		outExpr = sstrcat(outExpr, ")");
		
		pToken = peekToken(sourceFilePtr, NULL, 1);
	}

	free(inExp);

	grammerDepth--;
	return outExpr;
}

/**
 * Gets the next expression
 * @param sourceFilePtr - file buffer of the source code
 * @param currentToken - the value of the first TOKEN_... enum in the expression
 * @return A string with the transpiled source code for the expression
 */
static char* grammerExpression(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER EXPRESSION CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* outExpr = (char*) smalloc(1);
	outExpr[0] = '\0';
	char* inExp = grammerTerm(sourceFilePtr, outFilePtr, currentToken);
	outExpr = sstrcat(outExpr, inExp);

	int pToken = peekToken(sourceFilePtr, NULL, 1);
	while ((pToken & (TOKEN_ADD | TOKEN_SUB)) > 0) {
		if (pToken == TOKEN_ADD) outExpr = sstrcat(outExpr, ", 1, "); else
		if (pToken == TOKEN_SUB) outExpr = sstrcat(outExpr, ", 2, ");

		free(inExp);
		inExp = grammerTerm(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));

		outExpr = sstrpre(outExpr, "esvExpr(");
		outExpr = sstrcat(outExpr, inExp);
		outExpr = sstrcat(outExpr, ")");

		pToken = peekToken(sourceFilePtr, NULL, 1);
	}

	free(inExp);

	grammerDepth--;
	return outExpr;
}

static char* grammerComparison(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER COMPARISON CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	char* outExpr = (char*) smalloc(1);
	outExpr[0] = '\0';
	char* inExp = grammerExpression(sourceFilePtr, outFilePtr, currentToken);
	outExpr = sstrcat(outExpr, inExp);

	int pToken = peekToken(sourceFilePtr, NULL, 1);
	while ((pToken & (TOKEN_DEQ | TOKEN_GTT | TOKEN_GTE | TOKEN_LST | TOKEN_LSE)) > 0) {
		if (pToken == TOKEN_DEQ) outExpr = sstrcat(outExpr, ", 1, "); else
		if (pToken == TOKEN_GTT) outExpr = sstrcat(outExpr, ", 2, "); else
		if (pToken == TOKEN_GTE) outExpr = sstrcat(outExpr, ", 3, "); else
		if (pToken == TOKEN_LST) outExpr = sstrcat(outExpr, ", 4, "); else
		if (pToken == TOKEN_LSE) outExpr = sstrcat(outExpr, ", 5, ");

		free(inExp);
		inExp = grammerExpression(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));
		
		outExpr = sstrpre(outExpr, "esvComp(");
		outExpr = sstrcat(outExpr, inExp);
		outExpr = sstrcat(outExpr, ")");

		pToken = peekToken(sourceFilePtr, NULL, 1);
	}

	free(inExp);

	grammerDepth--;
	return outExpr;
}

static int grammerStatement(FILE* sourceFilePtr, FILE* outFilePtr, int currentToken, int funcDefMode) {
	if (DEBUGLEVEL > 1) printf("\x1b[32m%sGRAMMER STATEMENT CALL: %s\n\x1b[0m", str_repeat("| ", grammerDepth), getTokenNameFromValue(currentToken));
	grammerDepth++;

	if (currentToken == TOKEN_EOF) return 2;

	char* iVarVal = NULL;
	currentToken = peekToken(sourceFilePtr, &iVarVal, 1);

	if (currentToken == TOKEN_EOF) return 2;

	if (currentToken == TOKEN_EOF) return 0;

	grammerCheck(sourceFilePtr, currentToken, TOKEN_DEF | TOKEN_VAR | TOKEN_CON | TOKEN_RET | TOKEN_LOP);

	// Define var / function
	if (currentToken == TOKEN_DEF) {
		nextToken(sourceFilePtr, NULL);

		char* potVarName = NULL;
		int varToken = nextToken(sourceFilePtr, &potVarName);
		if (potVarName == NULL) genericError(sourceFilePtr, 901, "Failed to parse function/var name");

		grammerCheck(sourceFilePtr, varToken, TOKEN_VAR);
		currentToken = grammerMatch(sourceFilePtr, TOKEN_EQL | TOKEN_BAR);
		
		// Def function
		if (currentToken == TOKEN_BAR) {
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mDEFINE FUNCTION\x1b[0m\n");

			if (!funcDefMode) genericError(sourceFilePtr, 903, "Function defined not at top of file!");
			fputs("static ES3Var ", outFilePtr);
			fputs(potVarName, outFilePtr);
			fputs("__raw", outFilePtr);

			free(potVarName);

			char* inArr = grammerArray(sourceFilePtr, outFilePtr, currentToken, 1, 1);
			fputs(inArr, outFilePtr);
			free(inArr);
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
			
			fputs("ES3Var ", outFilePtr);
			fputs(potVarName, outFilePtr);
			fputs("__raw = ", outFilePtr);

			char* inComp = grammerComparison(sourceFilePtr, outFilePtr, currentToken);
			fputs(inComp, outFilePtr);
			free(inComp);

			fputs(";\n", outFilePtr);

			free(potVarName);

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

		grammerCheck(sourceFilePtr, pToken, TOKEN_EQL | TOKEN_BAR);

		// Redefine var
		if (pToken == TOKEN_EQL) {
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mREDEFINE VAR\x1b[0m\n");
			grammerMatch(sourceFilePtr, TOKEN_VAR); // a | = 12;
			grammerMatch(sourceFilePtr, TOKEN_EQL); // a = | 12;
			fputs(iVarVal, outFilePtr);
			fputs("__raw", outFilePtr);
			fputs(" = ", outFilePtr);
			char* inComp = grammerComparison(sourceFilePtr, outFilePtr, TOKEN_EQL);
			fputs(inComp, outFilePtr);
			fputs(";\n", outFilePtr);
			grammerMatch(sourceFilePtr, TOKEN_EDL);
		} 
		// Call function
		else { 
			if (DEBUGLEVEL > 0) printf("\x1b[1;36mCALL FUNCTION\x1b[0m\n");
			char* funcIn = grammerFunc(sourceFilePtr, outFilePtr, currentToken);
			fputs(funcIn, outFilePtr);
			free(funcIn);
			grammerMatch(sourceFilePtr, TOKEN_EDL);
			fputs(";\n", outFilePtr);
		}

		grammerDepth--;
		return 0;
	}

	// If statement
	if (currentToken == TOKEN_CON) {
		if (DEBUGLEVEL > 0) printf("\x1b[1;36mIF STATEMENT\x1b[0m\n");

		fputs("if (esvTruthy", outFilePtr);

		// | if (a > b) { ... };
		nextToken(sourceFilePtr, NULL);
		// if | (a > b) { ... };
		char* inPar = grammerParenthasis(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));
		fputs(inPar, outFilePtr);
		free(inPar);
		fputs(") ", outFilePtr);
		currentToken = nextToken(sourceFilePtr, NULL);
		// if (a > b) | { ... };
		grammerCodeBlock(sourceFilePtr, outFilePtr, currentToken);
		// if (a > b) { ... | };
		grammerMatch(sourceFilePtr, TOKEN_ECB);
		grammerMatch(sourceFilePtr, TOKEN_EDL);

		grammerDepth--;
		return 0;
	}

	// While statement
	if (currentToken == TOKEN_LOP) {
		if (DEBUGLEVEL > 0) printf("\x1b[1;36mLOOP STATEMENT\x1b[0m\n");

		fputs("while (esvTruthy", outFilePtr);

		// | if (a > b) { ... };
		nextToken(sourceFilePtr, NULL);
		// if | (a > b) { ... };
		char* inPar = grammerParenthasis(sourceFilePtr, outFilePtr, nextToken(sourceFilePtr, NULL));
		fputs(inPar, outFilePtr);
		free(inPar);
		fputs(") ", outFilePtr);
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

		char* inComp = grammerComparison(sourceFilePtr, outFilePtr, currentToken);
		fputs(inComp, outFilePtr);
		free(inComp);
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

		int curFuncMode = grammerStatement(sourceFilePtr, outFilePtr, NULL, funcDefMode);
		if (curFuncMode == 2) return funcDefMode;

		if (curFuncMode == 0) {
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

int main(int argc, char** argv) {
	argc = 2;
	argv = (char*[2]) {"es3.exe", "test.es3"};

	FILE* test = NULL;

	if (argc < 2) genericError(NULL, 100, "Too few arguments! Usage: es3 fileIn.es3 [fileOut]");
	if (argc > 3) genericError(NULL, 100, "Too many arguments! Usage: es3 fileIn.es3 [fileOut]");

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

	fputs("#include <stdio.h>\n#include \"std.c\"\n\n", outFilePtr);

	// Read file
	if (!grammerProgram(sourceFilePtr, outFilePtr)) {
		fputs("return 0;\n}", outFilePtr);
	} else {
		fputs("int main() { }", outFilePtr);
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