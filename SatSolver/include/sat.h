#ifndef SAT_H
#define SAT_H

#include <stdbool.h>
#include <stdio.h>

#define UNDEFINED -1

// Estrutura SAT
// Lista de Literais de uma Clausula 
typedef struct Literal {
    struct Literal *next;
    int atomID;             
    bool isNegative;        
} Literal;

// Lista de Clausulas de uma Formula
typedef struct Clause {
    struct Clause *next;   
    Literal *literalHead;   
    int literalCount;
} Clause;

// Estrutura da Fórmula
typedef struct Formula {
    Clause *clauseHead;         
    int clauseCount;        
    int atomCount;
} Formula;

// Estrutura que guarda a interpretação atual de uma variavel
// Undefined = -1; False = 0; True = 1
typedef struct PartialInterp {
    short *truthValue; 
} PartialInterp;

// Estruturaa do nó da árvore de decisão
typedef struct DecisionNode {
    bool isSAT;                     //Indica o resultado do caminho (SAT ou UNSAT)
    int decisionAtomID;             //ID da Variável Booleana
    short polarity;                 //Valor Booleano Atribuido ao nó (TRUE ou FALSE)
    struct DecisionNode *left;      //Ramo Posittivo (TRUE)
    struct DecisionNode *right;     //Ramo Negativo (FALSE)
} DecisionNode;

// Protótipos SAT
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