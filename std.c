#include <stdlib.h>
#include <math.h>
#include "transUtil.c"

static ES3Var sqrt__raw(ES3Var a) {
    return (ES3Var) { .type = 1, .valNum = sqrt(a.valNum) };
}

static char* esvToString(ES3Var a) {
    // Number
    if (a.type == 1) { 
        char* buffer = malloc(sizeof(char) * 1080);
        snprintf(buffer, 1080, "%g", a.valNum);
        return buffer;
    }
    // String
    else if (a.type == 2) {
        char* buffer = malloc(sizeof(char) * (strlen(a.valString) + 3));
        buffer[0] = '"';
        buffer[1] = '\0';
        buffer = strcat(buffer, a.valString);
        buffer = strcat(buffer, "\"");
        return buffer;
    }
    // Array
    else if (a.type == 4) {
        char *buffer = malloc(sizeof(char) * 2);
        if (buffer == NULL) exit(800);
        buffer[0] = '[';
        buffer[1] = '\0';

        while (a.valArrCur) {
            char* strVal = esvToString(*a.valArrCur);
            int size = strlen(buffer) + strlen(strVal) + 1;
            buffer = realloc(buffer, size+2);
            buffer = strcat(buffer, strVal);

            if (a.valArrNext) buffer = strcat(buffer, ", ");
            if (a.valArrCur->type == 1 || a.valArrCur->type == 2 || a.valArrCur->type == 4) free(strVal);
            if (a.valArrNext != NULL) a = *a.valArrNext; else break;
        }
        
        buffer = realloc(buffer, strlen(buffer)+2);
        buffer = strcat(buffer, "]");
        
        return buffer;
    }

    switch (a.type) {
        case 0:
            return "Null";
        case 3:
            return a.valBool ? "true" : "false";
        default:
            return "Undefined";
    }
}

static void print__raw(ES3Var a) {
    char* out = esvToString(a);
    printf(out);
    if (a.type == 1 || a.type == 2 || a.type == 4) free(out);
}

static void println__raw(ES3Var a) {
    print__raw(a);
    printf("\n");
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