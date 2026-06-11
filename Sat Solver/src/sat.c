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

// Cria um array que armazena a interpretação parcial de cada variável única da formula, inicializando todas como UNDEFINED
PartialInterp initializePartialInterp(Formula *f){
    PartialInterp pi;
    pi.truthValue = malloc(sizeof(short) * (f->atomCount + 1)); 
    // Verifica a alocação do malloc
    if (pi.truthValue == NULL) {
        fprintf(stderr, "ERRO: Falha ao alocar PartialInterp.\n");
        return pi;
    }
    
    for (int i = 0; i < f->atomCount + 1; i++) { pi.truthValue[i] = UNDEFINED; }

    return pi;
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

            if (temp != 0) { 
                newClause->literalHead = addLiteral(newClause->literalHead, abs(temp), (temp < 0));
                newClause->literalCount++;
            } else { break; }

            newClause->literalCount++;
        }
    }
    return true;
}

// Verifica se a fórmula inteira é VERDADEIRA dada a interpretação atual
// Retorna 1 (SAT), 0 (UNSAT) ou -1 (UNDEFINED)
int evaluateFormula(Formula *f, PartialInterp *pi) {
    bool allClausesTrue = true; // Verdade até que se prove o contrário

    Clause *currentClause = f->clauseHead;
    while (currentClause != NULL) {
        bool clauseIsTrue = false;
        bool clauseIsUndefined = false;

        // Percorre os literais dentro de uma cláusula específica
        Literal *currentLiteral = currentClause->literalHead;
        while (currentLiteral != NULL) {
            short val = pi->truthValue[currentLiteral->atomID];

            if (val == UNDEFINED) {
                clauseIsUndefined = true;
            } else {
                // Literal verdadeiro: (val==1 e não negado) OU (val==0 e negado)
                if ((val == 1  && !currentLiteral->isNegative) ||
                    (val == 0 &&  currentLiteral->isNegative)) {
                    clauseIsTrue = true;
                    break;
                }
            }
            currentLiteral = currentLiteral->next;
        }

        // Se a clause ainda não é verdadeira e possui literais indefinidos, é inclonclusivo
        if (!clauseIsTrue && !clauseIsUndefined) { return 0; }
        
        // Se a clause é falsa e não possui literais indefinidos, a fórmula é definitivamente falsa
        if (!clauseIsTrue) { allClausesTrue = false; }
        
        currentClause = currentClause->next;
    }

    return allClausesTrue ? true : UNDEFINED;
}

DecisionNode *createDecisionNode(int atomID) {
    DecisionNode *node = (DecisionNode *) malloc(sizeof(DecisionNode));
    if (node == NULL) {
        fprintf(stderr, "ERRO: Falha ao alocar DecisionNode.\n");
        return NULL;
    }
    node->decisionAtomID = atomID;
    node->value          = 0;
    node->left           = NULL;
    node->right          = NULL;
    node->isSAT          = false;
    return node;
}

// Árvore de Decisão Recurrsiva, resolve o SAT usando Backtracking
DecisionNode* solveSAT(Formula *f, PartialInterp *pi, int currentVar) {
    // Cria o nó raiz da Arvore de decisão
    DecisionNode *node = createDecisionNode(currentVar);
    if (node == NULL) return NULL;

    if (currentVar > f->atomCount) {
        node->isSAT = false;
        return node;
    }
 
    int result = evaluateFormula(f, pi);
    if (result == 1)  { node->isSAT = true;  return node; }
    if (result == 0)  { node->isSAT = false; return node; }

    //Ramificação(BACKTRACKING))
    // Ramo com variável VERDADEIRA (1)
    pi->truthValue[currentVar] = true; // Modifica o array global diretamente
    node->value = true;
    node->left = solveSAT(f, pi, currentVar + 1);
    
    if (node->left != NULL && node->left->isSAT) {
        node->isSAT = true;
        return node; // Achou SAT, sobe sem desfazer a modificação.
    }

    // Se o caminho 1 falhou, tentamos o ramo com a variável FALSA (0)
    pi->truthValue[currentVar] = false;
    node->value = false;
    node->right = solveSAT(f, pi, currentVar + 1);

    if (node->right != NULL && node->right->isSAT) {
        node->isSAT = true;
        return node; // Achou SAT, sobe sem desfazer a modificação.
    }

    // Se chegou aqui, ambos falharam: desfaz a atribuição e sinaliza UNSAT
    pi->truthValue[currentVar] = UNDEFINED;
    node->isSAT = false;
    return node;
}

// Imprime a fórmula lógica no formato clássico de Conjunção de Disjunções (CNF)
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