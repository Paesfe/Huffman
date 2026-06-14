#include "../include/sat.h"
#include <stdlib.h>
#include <string.h>

Formula *initializeFormula(){
    Formula *newFormula = (Formula *) malloc(sizeof(Formula));
    // Verifica a alocação do malloc
    if (newFormula == NULL) {
        fprintf(stderr, "ERRO: Falha ao alocar Formula.\n");
        return NULL;
    }
    newFormula->clauseHead = NULL;
    newFormula->clauseCount = 0;
    newFormula->atomCount = 0;
    return newFormula;
}

// Cria uma clause vazia, e adiciona no início da lista encadeada de cláusulas
Clause *addClause(Formula *f) {
    Clause *newClause = (Clause *) malloc(sizeof(Clause));
    // Verifica a alocação do malloc
    if (newClause == NULL) {
        fprintf(stderr, "ERRO: Falha ao alocar Clause.\n");
        return NULL;
    }
    newClause->literalCount = 0;
    newClause->literalHead  = NULL;
    newClause->next         = f->clauseHead;
    f->clauseHead           = newClause;
    return newClause;
}

// Cria e preenche literal, colocando-o na lista encadeada de uma cláusula específica
Literal *addLiteral(Literal *l, int variable, bool isNegative){
    Literal *newLiteral = (Literal *)malloc(sizeof(Literal));
    // Verifica a alocação do malloc
    if (newLiteral == NULL) {
        fprintf(stderr, "ERRO: Falha ao alocar Literal.\n");
        return l; // retorna a lista anterior intacta
    }

    newLiteral->atomID     = variable;
    newLiteral->isNegative = isNegative;
    newLiteral->next       = l;
    return newLiteral;
}

// Faz a leitura da Clausulas que compoem a Formula
bool readClauses(Formula *f, FILE *file) {
    for (int i = 0; i < f->clauseCount; i++){
        Clause *newClause = addClause(f);
        if (newClause == NULL) return false;
        
        while(1){
            int temp;
            if (fscanf(file, "%d", &temp) != 1 || abs(temp) > f->atomCount) { 
                printf("ERROR: Erro na leitura das clausulas.\n");
                return false; // Informa o erro para quem chamou
            }

            // '0' é indicador do fim da Clausula
            if (temp != 0) { newClause->literalHead = addLiteral(newClause->literalHead, abs(temp), (temp < 0)); }
            else { break; }
            
            newClause->literalCount++;
        }
    }
    return true;
}

// A Função sobe de nó filho para pai, até retornar ao nó que fez a chamada de função 
short getVarValue(DecisionNode *leaf, int atomID) {
    DecisionNode *node = leaf;
    while (node != NULL) {
        if (node->decisionAtomID == atomID) return node->polarity;
        // Sobe um nível em direção à raiz da árvore
        node = node->parent;
    }
    // Se chegou na raiz e não encontrou, a variável ainda não foi decidida neste ramo
    return UNDEFINED;
}

// Verifica se a fórmula inteira é VERDADEIRA dada a interpretação atual
// Retorna 1 (SAT), 0 (UNSAT) ou -1 (UNDEFINED)
int evaluateFormula(Formula *f, DecisionNode *leaf) {
    bool hasUndefinedClauses = false; // Checa se o resultado da formula depende de uma variável que ainda não teve valor definido

    Clause *currentClause = f->clauseHead;
    while (currentClause != NULL) {
        bool clauseIsTrue = false;
        bool clauseIsUndefined = false;

        // Percorre os literais dentro de uma cláusula específica
        Literal *currentLiteral = currentClause->literalHead;
        while (currentLiteral != NULL) {
            short val = getVarValue(leaf, currentLiteral->atomID);

            if (val == UNDEFINED) { clauseIsUndefined = true; }
            else {
                // Literal verdadeiro: (val==1 e não negado) OU (val==0 e negado)
                if ((val == 1  && !currentLiteral->isNegative) || (val == 0 &&  currentLiteral->isNegative)) {
                    clauseIsTrue = true;
                    break;
                }
            }

            currentLiteral = currentLiteral->next;
        }

        // Se a cláusula NÃO é verdadeira...
        if (!clauseIsTrue) {
            // ...e NÃO possui literais indefinidos, ela é 100% FALSA
            // Se uma cláusula é falsa, a fórmula inteira é falsa
            if (!clauseIsUndefined) { return false; } 
            // ...mas possui literais indefinidos, ela está aberta/inconclusiva por enquanto
            else { hasUndefinedClauses = true; }
        }
        
        currentClause = currentClause->next;
    }

    // Se nenhuma Clausula for completamente falsa e existir alguma que ficou indefinida, a formula inteira está UNDEFINED
    if (hasUndefinedClauses) { return UNDEFINED; }

    // Se chegou aqui, todas as Clausulas são verdadeiras
    return true;
}

DecisionNode *createDecisionNode(int atomID, DecisionNode *parent) {
    DecisionNode *node = (DecisionNode *) malloc(sizeof(DecisionNode));
    if (node == NULL) {
        fprintf(stderr, "ERRO: Falha ao alocar DecisionNode.\n");
        return NULL;
    }
    node->decisionAtomID = atomID;
    node->polarity       = 0;
    node->left           = NULL;
    node->right          = NULL;
    node->isSAT          = false;
    node->parent         = parent;
    return node;;
}

// Árvore de Decisão Recurrsiva, resolve o SAT usando Backtracking
DecisionNode* solveSAT(Formula *f, int currentVar, DecisionNode *parent) {
    
    int result = evaluateFormula(f, parent);
    if (result == 1)  { 
        DecisionNode *leaf = createDecisionNode(currentVar, parent);
        if (leaf) leaf->isSAT = true;
        return leaf;
    }
    if (result == 0)  { 
        DecisionNode *leaf = createDecisionNode(currentVar, parent);
        if (leaf) leaf->isSAT = false;
        return leaf;
    }

    //Ramificação
    DecisionNode *node = createDecisionNode(currentVar, parent);
    if (node == NULL) return NULL;
    
    // Testa o ramo VERDADEIRO (1)
    node->polarity = true;
    node->left = solveSAT(f, currentVar + 1, node); // Recursão
    
    if (node->left != NULL && node->left->isSAT) {
        node->isSAT = true;
        return node; // Achou SAT, sobe sem desfazer a modificação.
    }

    // Testa o ramo FALSO (0), somente se o ramo VERDADEIRO falhar
    node->polarity = false;
    node->right = solveSAT(f, currentVar + 1, node); // Recursão

    if (node->right != NULL && node->right->isSAT) {
        node->isSAT = true;
        return node; // Achou SAT, sobe sem desfazer a modificação.
    }

    // Se chegou aqui, ambos falharam: desfaz a atribuição e sinaliza UNSAT
    node->polarity = UNDEFINED;
    node->isSAT = false;
    return node;
}

// Imprime a fórmula lógica: (~1 v 2) ^ (1 v ~3)...
void printBooleanFormulaCNF(const Formula *f) {
    Clause *currentClause = f->clauseHead;
    while (currentClause != NULL) {
        printf("(");
        Literal *currentLiteral = currentClause->literalHead;
        while (currentLiteral != NULL) {
            if (currentLiteral->isNegative) { printf("~%d", currentLiteral->atomID); }
            else { printf("%d", currentLiteral->atomID); }

            if (currentLiteral->next != NULL) { printf(" V "); }
            currentLiteral = currentLiteral->next;
        }
        printf(")");
        if (currentClause->next != NULL) { printf(" ^ "); }
        currentClause = currentClause->next;
    }
    printf("\n");
}

// Gestores de armazenamento
void freeFormula(Formula *f) {
    if (f == NULL) return;
    Clause *currClause = f->clauseHead;
    while (currClause != NULL) {
        Clause *nextClause = currClause->next;
        Literal *currLiteral = currClause->literalHead;
        while (currLiteral != NULL) {
            Literal *nextLiteral = currLiteral->next;
            free(currLiteral);
            currLiteral = nextLiteral;
        }
        free(currClause);
        currClause = nextClause;
    }
    free(f);
}

void freeTree(DecisionNode *node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}