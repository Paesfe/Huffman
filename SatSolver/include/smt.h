#ifndef SMT_H
#define SMT_H

#include "../include/sat.h"

// Estrutura de uma única equação da teoria LIA
typedef struct LIAConstraint {
    int atomID;                     //ID da variável Booleana realcionada
    int coefficient;                //Coeficiente angular da reta: 'a' em 'ax + b = k' 
    char mathVar;                   //A letra da variavel
    char innerSign;                 //'+' ou '-' se houver deslocamento interno, ou '0' se não houver
    int innerOffset;                //Coeficiente linear da reta: 'b' em 'ax + b = k'
    char operatorSymbol[3];         //Armazena "<=", ">=", "<", ">" e "=="
    int constantValue;              //O valor após o operador: 'k' em 'ax + b = k' 
    struct LIAConstraint *next;     //Ponteiro para a próxima equação na lista
} LIAConstraint;

// Lista encadeada de todas as equações 
typedef struct LIATheory {
    LIAConstraint *constraintListHead;
    int totalConstraints;
} LIATheory;

// Estrutura que armazena o intervalo válido para 'x'
typedef struct Interval {
    int minimumValue;
    int maximumValue;
} Interval;

// Protótipos SMT
LIATheory *initializeLIATheory();
bool readEquations(Formula *f, LIATheory* t, FILE *file);
int floorDiv(int dividend, int divisor);
int ceilDiv(int dividend, int divisor);
Interval calculateLIAInterval(DecisionNode *leaf, LIATheory *theory);
bool evaluateMathematicalConsistency(DecisionNode *leaf, LIATheory *theory);
DecisionNode *solveSMT(Formula *f, int currentVar, DecisionNode *parent, LIATheory *theory);
void printLIATheoryConstraints(const LIATheory *theory);
void freeLIATheory(LIATheory *theory);

#endif //SMT_H