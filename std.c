#include <stdlib.h>
#include <math.h>

#include "esvutil.h"

#define PI__raw (ES3Var) { .type = 1, .valNum = 3.14159265358979323846 }
#define E__raw (ES3Var) { .type = 1, .valNum = 2.71828182845904523536 }
#define RAD__raw (ES3Var) { .type = 1, .valNum = 0.01745329238474369049072265625 }
#define DEG__raw (ES3Var) { .type = 1, .valNum = 57.295780181884765625 }

ES3Var sqrt__raw(ES3Var a) {
    if (a.type != 1) return (ES3Var) { .type = 0 };
    return (ES3Var) { .type = 1, .valNum = sqrt(a.valNum) };
}

ES3Var sin__raw(ES3Var a) {
    if (a.type != 1) return (ES3Var) { .type = 0 };
    return (ES3Var) { .type = 1, .valNum = sin(a.valNum) };
}

ES3Var cos__raw(ES3Var a) {
    if (a.type != 1) return (ES3Var) { .type = 0 };
    return (ES3Var) { .type = 1, .valNum = cos(a.valNum) };
}

ES3Var tan__raw(ES3Var a) {
    if (a.type != 1) return (ES3Var) { .type = 0 };
    return (ES3Var) { .type = 1, .valNum = tan(a.valNum) };
}

ES3Var log__raw(ES3Var a, ES3Var b) {
    if (a.type != 1) return (ES3Var) { .type = 0 };
    if (b.type != 1) return (ES3Var) { .type = 0 };
    return (ES3Var) { .type = 1, .valNum = log(a.valNum)/log(b.valNum) };
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
    
    char *pStr = smalloc(len_max);
    current_size = len_max;

    puts(esvToString(a));

    int c = 0;
    unsigned int i =0;
    while (( c = getchar() ) != '\n') {
        pStr[i++]=(char)c;

        if (i == current_size) {
            current_size = i+len_max;
            pStr = srealloc(pStr, current_size);
        }
    }

    pStr[i] = '\0';

    return (ES3Var) { .type = 2, .valString = pStr };
}