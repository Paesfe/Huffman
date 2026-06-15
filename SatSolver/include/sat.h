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

// Estruturaa do nó da árvore de decisão
typedef struct DecisionNode {
    bool isSAT;                     // Indica o resultado do caminho (SAT ou UNSAT)
    int decisionAtomID;             // ID da Variável Booleana
    short polarity;                 // Valor Booleano Atribuído ao nó (TRUE, FALSE ou UNDEFINED)
    struct DecisionNode *left;      // Ramo Positivo (TRUE)
    struct DecisionNode *right;     // Ramo Negativo (FALSE)
    struct DecisionNode *parent;    // Ramo de volta ao pai — torna a árvore o estado
} DecisionNode;

// Protótipos SAT
Formula* initializeFormula();
Clause* addClause(Formula *f);
Literal* addLiteral(Literal *l, int variable, bool isNegative);
bool readClauses(Formula *f, FILE *file);
short getVarValue(DecisionNode *leaf, int atomID);
int evaluateFormula(Formula *f, DecisionNode *leaf);
DecisionNode* createDecisionNode(int atomID, DecisionNode *parent);
DecisionNode* solveSAT(Formula *f, int currentVar, DecisionNode *parent);
void printBooleanFormulaCNF(const Formula *f);
void freeFormula(Formula *f);
void freeTree(DecisionNode *node);

#endif //SAT_H