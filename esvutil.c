#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "esvutil.h"

void genericError(FILE* sourceFilePtr, int code, const char* const message, ...) {
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

void* smalloc(size_t size) {
	void* m = malloc(size);
	if (m == NULL) {
		genericError(NULL, 102, "Out of memory!");
	}
	return m;
}

void* srealloc(void* _Block, size_t size) {
	void* m = realloc(_Block, size);
	if (m == NULL) {
		free(m);
		genericError(NULL, 102, "Out of memory!");
	}
	_Block = m;
	return m;
}

char* str_repeat(char* str, int times) {
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

char* sstrcat(char* destination, const char* source) {
	int size = strlen(destination) + strlen(source) + 1;
	destination = srealloc(destination, size);
	strcat(destination, source);
	return destination;
}

char* sstrpre(char* destination, const char* source) {
	size_t len = strlen(source);
	destination = srealloc(destination, strlen(destination) + len + 1);
    memmove(destination + len, destination, strlen(destination) + 1);
	destination = memcpy(destination, source, len);
	return destination;
}

char* esvToString(ES3Var a) {
    // Number
    if (a.type == 1) { 
        char* buffer = smalloc(sizeof(char) * 1080);
        snprintf(buffer, 1080, "%g", a.valNum);
        return buffer;
    }
    // String
    else if (a.type == 2) {
        char* buffer = smalloc(sizeof(char) * (strlen(a.valString) + 3));
        buffer[0] = '"';
        buffer[1] = '\0';
        buffer = sstrcat(buffer, a.valString);
        buffer = sstrcat(buffer, "\"");
        return buffer;
    }
    // Array
    else if (a.type == 4) {
        char *buffer = smalloc(sizeof(char) * 2);
        buffer[0] = '[';
        buffer[1] = '\0';

        while (a.valArrCur) {
            char* strVal = esvToString(*a.valArrCur);
            int size = strlen(buffer) + strlen(strVal) + 1;
            buffer = srealloc(buffer, size+2);
            buffer = sstrcat(buffer, strVal);

            if (a.valArrNext) buffer = sstrcat(buffer, ", ");
            if (a.valArrCur->type == 1 || a.valArrCur->type == 2 || a.valArrCur->type == 4) free(strVal);
            if (a.valArrNext != NULL) a = *a.valArrNext; else break;
        }
        
        buffer = srealloc(buffer, strlen(buffer)+2);
        buffer = sstrcat(buffer, "]");
        
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

ES3Var esvComp(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
            if (b.type != 1) return (ES3Var) { .type = 0 };
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 3, .valBool = a.valNum == b.valNum };
                case 2:
                    return (ES3Var) { .type = 3, .valBool = a.valNum >  b.valNum };
                case 3:
                    return (ES3Var) { .type = 3, .valBool = a.valNum >= b.valNum };
                case 4:
                    return (ES3Var) { .type = 3, .valBool = a.valNum <  b.valNum };
                case 5:
                    return (ES3Var) { .type = 3, .valBool = a.valNum <= b.valNum };
            }
        case 2:
            if (b.type != 2) return (ES3Var) { .type = 0 };
            switch(op) {
                case 1:
                    return (ES3Var) { .type = 3, .valBool = strcmp(a.valString, b.valString) == 0};
                case 2:
                    return (ES3Var) { .type = 3, .valBool = strcmp(a.valString, b.valString) >  0};
                case 3:
                    return (ES3Var) { .type = 3, .valBool = strcmp(a.valString, b.valString) >= 0};
                case 4:
                    return (ES3Var) { .type = 3, .valBool = strcmp(a.valString, b.valString) <  0};
                case 5:
                    return (ES3Var) { .type = 3, .valBool = strcmp(a.valString, b.valString) <= 0};
            }
        case 3:
            if (b.type != 3) return (ES3Var) { .type = 0 };
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 3, .valBool = a.valBool == b.valBool };
                case 2:
                    return (ES3Var) { .type = 3, .valBool = a.valBool >  b.valBool };
                case 3:
                    return (ES3Var) { .type = 3, .valBool = a.valBool >= b.valBool };
                case 4:
                    return (ES3Var) { .type = 3, .valBool = a.valBool <  b.valBool };
                case 5:
                    return (ES3Var) { .type = 3, .valBool = a.valBool <= b.valBool };
            }
        default:
            return (ES3Var) { .type = 0 };
    }
}

ES3Var esvTerm(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
            if (b.type != 1) return (ES3Var) { .type = 0 };
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 1, .valNum = a.valNum * b.valNum };
                case 2:
                    return (ES3Var) { .type = 1, .valNum = a.valNum / b.valNum };
            }
        default:
            return (ES3Var) { .type = 0 };
    }
}

ES3Var esvExpr(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
            if (b.type != 1) return (ES3Var) { .type = 0 };
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 1, .valNum = a.valNum + b.valNum };
                case 2:
                    return (ES3Var) { .type = 1, .valNum = a.valNum - b.valNum };
            }
        default:
            return (ES3Var) { .type = 0 };
    }
}

ES3Var esvExpo(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
            if (b.type != 1) return (ES3Var) { .type = 0 };
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 1, .valNum = pow(a.valNum, b.valNum) };
            }
        default:
            return (ES3Var) { .type = 0 };
    }
}

int esvTruthy(ES3Var a) {
    switch (a.type){
        case 1:
            return a.valNum != 0;
        case 3:
            return a.valBool;
        default:
            return 0;
    }
}