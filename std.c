#include <stdlib.h>
#include <math.h>

#include "esvutil.h"

ES3Var sqrt__raw(ES3Var a) {
    if (a.type != 1)
    return (ES3Var) { .type = 1, .valNum = sqrt(a.valNum) };
}

void print__raw(ES3Var a) {
    char* out = esvToString(a);
    printf(out);
    if (a.type == 1 || a.type == 2 || a.type == 4) free(out);
}

void println__raw(ES3Var a) {
    print__raw(a);
    printf("\n");
}

ES3Var input__raw(ES3Var a) {
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