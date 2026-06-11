#ifndef SAT_H
#define SAT_H

#include <stdbool.h>
#include <stdio.h>

#define UNDEFINED -1

// Lista encadeada para armazenar os literais de uma cláusula
typedef struct Literal {
    struct Literal *next;
    int atomID;             
    bool isNegative;        
} Literal;

// Lista encadeada para armazenar as cláusulas da fórmula
typedef struct Clause {
    struct Clause *next;   
    Literal *literalHead;   
    int literalCount;
} Clause;

// Estrutura principal da Fórmula
typedef struct Formula{
    Clause *clauseHead;         
    int clauseCount;        
    int atomCount;
} Formula;

// Estrutura que guarda a interpretação atual
// Undefined = -1; False = 0; True = 1
typedef struct PartialInterp{
    short *truthValue; 
} PartialInterp;

// Estrutura da arvore de decisão
typedef struct DecisionNode{
    int decisionAtomID;                    // Qual variável este nó está testando
    short value;                           // (0 ou 1)
    struct DecisionNode *left;             // Ramo positivo (True)
    struct DecisionNode *right;            // Ramo negativo (False)
    bool isSAT;                            // Se o caminho resultou em SAT ou UNSAT
} DecisionNode;

// Protótipos das funções do módulo SAT
Formula *initializeFormula();
PartialInterp initializePartialInterp(Formula *f);
Clause *addClause(Formula *f);
Literal *addLiteral(Literal *l, int variable, bool isNegative);
bool readClauses(Formula *f, FILE *file);
int evaluateFormula(Formula *f, PartialInterp *pi);
DecisionNode *createDecisionNode(int atomID);
DecisionNode *solveSAT(Formula *f, PartialInterp *pi, int currentVar);
void printBooleanFormulaCNF(const Formula *f);
void freeFormula(Formula *f);
void freeTree(DecisionNode *node);

#endif //SAT_H