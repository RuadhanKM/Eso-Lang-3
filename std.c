#include <stdlib.h>
#include <math.h>
#include "transUtil.c"

static ES3Var sqrt__raw(ES3Var a) {
    return (ES3Var) { .type = 1, .valNum = sqrt(a.valNum) };
}

static char* esvToString(ES3Var a) {
    char *buffer = malloc(1080);
    if (buffer == NULL) exit(800);
    switch (a.type) {
        case 0:
            return "Null";
        case 1:
            snprintf(buffer, 1080, "%g", a.valNum);
            return buffer;
        case 2:
            return a.valString;
        case 3:
            return a.valBool ? "true" : "false";
        default:
            return "Undefined";
    }
}

static void print__raw(ES3Var a) {
    char* out = esvToString(a);
    printf(out);
    if (a.type == 1) free(out);
}

static ES3Var input__raw(ES3Var a) {
    unsigned int len_max = 128;
    unsigned int current_size = 0;
    
    char *pStr = malloc(len_max);
    if (pStr == NULL) exit(800);
    current_size = len_max;

    puts(esvToString(a));

    int c = 0;
    unsigned int i =0;
    while (( c = getchar() ) != '\n') {
        pStr[i++]=(char)c;

        if (i == current_size) {
            current_size = i+len_max;
            char* tmp = realloc(pStr, current_size);
            if (tmp == NULL) exit(800);
            pStr = tmp;
        }
    }

    pStr[i] = '\0';

    return (ES3Var) { .type = 2, .valString = pStr };
}