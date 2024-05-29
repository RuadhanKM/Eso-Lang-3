#include <string.h>
#include <math.h>

typedef struct ES3Var_ {
    int type;

    double valNum;
    char* valString;
    int valBool;

    struct ES3Var_* valArrCur;
    struct ES3Var_* valArrNext;
} ES3Var;

static ES3Var esvArrayAccess(ES3Var a, int index) {
    int i = 0;
    ES3Var out = a;
    while (i < index) {
        if (out.valArrNext == NULL) return (ES3Var) { .type = 0 };
        out = *out.valArrNext; 
        i++;
    }
    return *out.valArrCur;
}

static void esvArraySet(ES3Var* a, int index, ES3Var* val) {
    int i = 0;
    while (i < index) {
        if (!a->valArrNext) return;
        a = a->valArrNext; 
        i++;
    }
    a->valArrCur = val;
}

static ES3Var esvComp(ES3Var a, int op, ES3Var b) {
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
    }
}

static ES3Var esvTerm(ES3Var a, int op, ES3Var b) {
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
    }
}

static ES3Var esvExpr(ES3Var a, int op, ES3Var b) {
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
    }
}

static ES3Var esvExpo(ES3Var a, int op, ES3Var b) {
    if (op == 0) return a;

    switch (a.type) {
        case 1:
            if (b.type != 1) return (ES3Var) { .type = 0 };
            switch (op) {
                case 1:
                    return (ES3Var) { .type = 1, .valNum = pow(a.valNum, b.valNum) };
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