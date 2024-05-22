#include <stdlib.h>
#include "transUtil.c"

static void print__raw(ES3Var a) {
    switch (a.type) {
        case 1:
            printf("%g\n", a.valNum);
            break;
        case 2:
            printf("%s\n", a.valString);
            break;
        case 3:
            printf("%s\n", a.valBool ? "True" : "False");
            break;
    }
}

static ES3Var input__raw(ES3Var a) {
    unsigned int len_max = 128;
    unsigned int current_size = 0;
    
    char *pStr = malloc(len_max);
    if (pStr == NULL) exit(800);
    current_size = len_max;

    puts(a.valString);

    int c = 0;
    unsigned int i =0;
    while (( c = getchar() ) != '\n') {
        pStr[i++]=(char)c;

        if (i == current_size) {
            current_size = i+len_max;
            pStr = realloc(pStr, current_size);
        }
    }

    pStr[i] = '\0';

    return (ES3Var) { .type = 2, .valString = pStr };
}