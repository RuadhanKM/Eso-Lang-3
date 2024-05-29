#pragma once

typedef struct ES3Var_ {
    int type;

    double valNum;
    char* valString;
    int valBool;

    struct ES3Var_* valArrCur;
    struct ES3Var_* valArrNext;
} ES3Var;

/**
 * Errors with message and error code
 * @param message - message in error
 * @param code - error code
 */
void genericError(FILE* sourceFilePtr, int code, const char* const message, ...);

/**
 * Resizes destination to fit both strings and prepends source to destination
 * @param destination - original string
 * @param source - string to add
 * @return The new string, must be freed
 */
char* sstrpre(char* destination, const char* source);

/**
 * Wrapper for strcat that resizes the string to fit before calling
 * @param destination - original string
 * @param source - string to add
 * @return The new string, must be freed
 */
char* sstrcat(char* destination, const char* source);

/**
 * Allocates a new string that is the str repeated by times
 * @param str - char string to be repeated
 * @param times - the number of times to repeat str
 * @return The new string, must be freed
 */
char* str_repeat(char* str, int times);

/**
 * Wrapper for realloc that exits and errors if realloc fails
 * @param size - ammount of memory to be allocted, passed to malloc
 * @return Pointer to the memory, returned from malloc
 */
void* srealloc(void* _Block, size_t size);

/**
 * Wrapper for malloc that exits and errors if malloc fails
 * @param size - ammount of memory to be allocted, passed to malloc
 * @return Pointer to the memory, returned from malloc
 */
void* smalloc(size_t size);

char* esvToString(ES3Var a);

ES3Var esvComp(ES3Var a, int op, ES3Var b);
ES3Var esvTerm(ES3Var a, int op, ES3Var b);
ES3Var esvExpr(ES3Var a, int op, ES3Var b);
ES3Var esvExpo(ES3Var a, int op, ES3Var b);

int esvTruthy(ES3Var a);