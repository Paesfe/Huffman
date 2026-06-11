#ifndef SMT_H
#define SMT_H

#include "../include/sat.h"

// Representa uma única equação da Teoria LIA expandida (ex: 2x + 3 >= 9 ou 3x - 1 < 10)
typedef struct LIAConstraint {
    int atomID;                 // ID da variável Booleana que ativa esta equação
    int coefficient;            // Multiplicador da variável matemática 'x' (ex: o '2' em 2x)
    char mathVar;               // Letra da variável
    char innerSign;             // '+' ou '-' se houver deslocamento interno, ou '0' se não houver
    int innerOffset;            // Valor do deslocamento interno (ex: o '3' em 2x + 3)
    char operatorSymbol[3];     // Armazena "<=", ">=", "<", ">"
    int constantValue;          // O valor após o operador (ex: o '9' em >= 9)
    struct LIAConstraint *next; // Ponteiro para a próxima equação da lista encadeada
} LIAConstraint;

// Guarda a lista de todas as equações matemáticas lidas do arquivo
typedef struct LIATheory {
    LIAConstraint *constraintListHead; // Ponteiro para o início da lista de equações
    int totalConstraints;              // Quantidade total de equações lidas
} LIATheory;

// Representa o intervalo de respostas matematicamente válidas para 'x'
typedef struct Interval {
    int minimumValue; // Limite inferior do intervalo (Piso)
    int maximumValue; // Limite superior do intervalo (Teto)
} Interval;


LIATheory *initializeLIATheory();
bool readEquations(Formula *f, LIATheory* t, FILE *file);
int floorDiv(int dividend, int divisor);
int ceilDiv(int dividend, int divisor);
Interval calculateLIAInterval(int totalVars, PartialInterp *pi, LIATheory *theory);
bool evaluateMathematicalConsistency(Formula *f, PartialInterp *pi, LIATheory *theory);
DecisionNode* solveSMT(Formula *f, PartialInterp *pi, int currentVar, LIATheory *theory);
void printLIATheoryConstraints(const LIATheory *theory);
void freeLIATheory(LIATheory *theory);

#endif //SMT_H