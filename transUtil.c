#include <string.h>

typedef struct ES3Var_ {
    int type;

    double valNum;
    char* valString;
    int valBool;
} ES3Var;

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

static ES3Var esvComp(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
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
    }
}

static ES3Var esvTerm(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 1, .valNum = a.valNum * b.valNum };
                case 2:
                    return (ES3Var) { .type = 1, .valNum = a.valNum / b.valNum };
            }
    }
}

static ES3Var esvExpr(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 1, .valNum = a.valNum + b.valNum };
                case 2:
                    return (ES3Var) { .type = 1, .valNum = a.valNum - b.valNum };
            }
    }
}

static int esvTruthy(ES3Var a) {
    switch (a.type){
        case 1:
            return a.valNum != 0;
        case 3:
            return a.valBool;
    }
}