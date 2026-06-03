#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

// Undefined = -1; False = 0; True = 1
#define UNDEFINED -1 


// Lista encadeada para armazenar os literais de uma cláusula
typedef struct literal {
    struct literal *next;
    int variable;             
    bool isNegative;        
} literal;

// Lista encadeada para armazenar as cláusulas da fórmula
typedef struct clause {
    struct clause *next;   
    literal *literalHead;   
    int literalCount;
} clause;

// Estrutura principal da Fórmula
typedef struct formula{
    clause *clauseHead;         
    int clauseCount;        
    int variableCount;     
} formula;

// Estrutura que guarda a interpretação atual
// Undefined = -1; False = 0; True = 1
typedef struct partialInt{
    short *valores;
} partialInt;

// Estrutura da arvore de decisão
typedef struct nodeBinaryTree{
    int literal;                           // Qual variável este nó está testando
    short value;                           // (0 ou 1)
    partialInt interpretacao;              // Estado da interpretação do node
    struct nodeBinaryTree *left;           // Ramo positivo (True)
    struct nodeBinaryTree *right;          // Ramo negativo (False)
    bool isSAT;                            // Se o caminho resultou em SAT ou UNSAT
} nodeBinaryTree;


formula *initializeFormula();
formula* createFormulaCNF();
int formulaReader(formula *f);
clause *addClause(formula *f);
literal *addLiteral(literal *l, int variable, bool isNegative);
int clauseReader(clause *c, formula *f);
void printFormula(formula *f);
partialInt initializePartialInterp(formula *f);
int evaluateFormula(formula *f, partialInt *pi);
partialInt cloneInterpretation(partialInt *oldPi, int totalVars, int currentVar, short guess);
nodeBinaryTree* solveSAT(formula *f, partialInt *pi, int currentVar);
void printSolution(nodeBinaryTree *node, int totalVars);
void freeFormula(formula *f);
void freeTree(nodeBinaryTree *node);

int main(){
    formula *f = createFormulaCNF();
    if (f == NULL) { return 1; }
    
    printf("\nFormula lida (CNF)\n");
    printFormula(f);
    
    partialInt pi = initializePartialInterp(f);

    printf("\nProcessando a Arvore de Decisao...\n");
    nodeBinaryTree *root = solveSAT(f, &pi, 1);

    if (root->isSAT) {
        printf("\nResultado final: SAT\n");
        printSolution(root, f->variableCount);
    } else {
        printf("\nResultado final: UNSAT\n");
        printf("Nenhuma combinacao torna a formula verdadeira.\n");
    }

    freeTree(root);
    freeFormula(f);
    return 0;
}

formula *initializeFormula(){
    formula *newFormula = (formula *) malloc(sizeof(formula));
    newFormula->clauseHead = NULL;
    newFormula->clauseCount = 0;
    newFormula->variableCount = 0;
    return newFormula;
}

// Faz a leitura da Fórmula a partir da entrada padrão
formula* createFormulaCNF(){
    int maxCommentSize = 128;
    char comment[maxCommentSize];
    char format[8];
    char command; 

    formula *inputFormula = initializeFormula();
    while(scanf(" %c", &command) != EOF){
        switch (command){
            case 'c': // Linha de comentário, ignora o restante da linha
                fgets(comment, maxCommentSize, stdin); // Lê e descarta o comentário
                break;

            case 'p': // Linha de configuração principal
                scanf("%s %d %d", format, &inputFormula->variableCount, &inputFormula->clauseCount);
                getchar();

                if (strcmp(format, "cnf") != 0){
                    printf("ERROR: Formato não suportado. Esperado 'cnf'.\n");
                    free(inputFormula);
                    return NULL;
                }

                // Leitura das cláusulas
                if (formulaReader(inputFormula) == UNDEFINED){
                    printf("ERROR: Erro na leitura das cláusulas. Verifique o formato.\n");
                    freeFormula(inputFormula);
                    return NULL;
                }

                return inputFormula;

            default:
                printf("ERROR: Comando desconhecido '%c'.\n", command); 
                free(inputFormula);
                return NULL;
        }
    }

    return inputFormula;
}

// Faz a leitura das clauses da formula, com base na informação do cabecalho
int formulaReader(formula *f){
    for (int i = 0; i < f->clauseCount; i++){
        clause *newClause = addClause(f);
        // Retorna UNDEFINED se houver erro de formato ou limite
        if ( clauseReader(newClause, f) == UNDEFINED){ return UNDEFINED; }
    }

    return 0; // Sucesso
}

// Cria uma clause vazia, e adiciona no início da lista encadeada de cláusulas da fórmula
clause *addClause(formula *f){
    clause *newClause = malloc(sizeof(clause));
    newClause->literalCount = 0;
    newClause->literalHead = NULL;
    newClause->next = f->clauseHead;
    f->clauseHead = newClause;
    return newClause;
}

// Insere um literal na lista encadeada de uma cláusula específica
literal *addLiteral(literal *l, int variable, bool isNegative){
    literal *newLiteral = (literal *)malloc(sizeof(literal));
    newLiteral->variable = variable;
    newLiteral->isNegative = isNegative;
    newLiteral->next = l;
    return newLiteral;
}

// Lê individualmente cada literal dentro de uma determinada clausula, até encontrar 0
int clauseReader(clause *c, formula *f){
    while(1){
        int temp;
        scanf("%d", &temp);

        // Verifica se não excede o limite definido pelo cabeçalho
        if (abs(temp) > f->variableCount){ return UNDEFINED; }

        // 0 == fim da cláusula, padrão DIMACS
        if (temp != 0) { c->literalHead = addLiteral(c->literalHead, abs(temp), (temp<0)); }
        else { return 0; }

        c->literalCount++;
    }
}



// Imprimi a formula lida: (1 V ~2) ^ (2 V 3) ^ ... 
void printFormula(formula *f){
    clause *currentClause = f->clauseHead;
    while(currentClause != NULL){
        printf("(");
        literal *currentLiteral = currentClause->literalHead;
        while(currentLiteral != NULL){
            if (currentLiteral->isNegative) { printf("~%d", currentLiteral->variable); }
            else { printf("%d", currentLiteral->variable); }

            if (currentLiteral->next != NULL){ printf(" V "); } // Imprime o 'OR' entre literais
            currentLiteral = currentLiteral->next;
        }
        printf(")");
        if (currentClause->next != NULL){ printf(" ^ "); } // Imprime o 'AND' entre cláusulas
        currentClause = currentClause->next;
    }

    printf("\n");
}


// Cria um array que armazena a interpretação parcial de cada variável única da  formula, inicializando todas como UNDEFINED
partialInt initializePartialInterp(formula *f){
    partialInt pi;
    pi.valores = malloc(sizeof(short) * (f->variableCount + 1)); 
    for (int i = 0; i < f->variableCount + 1; i++) { pi.valores[i] = UNDEFINED; }

    return pi;
}

// Verifica se a fórmula inteira é VERDADEIRA dada a interpretação atual
// Retorna 1 (SAT), 0 (UNSAT) ou -1 (UNDEFINED)
int evaluateFormula(formula *f, partialInt *pi) {
    bool allClausesTrue = true; // Verdade até que se prove o contrário

    clause *currentClause = f->clauseHead;
    while (currentClause != NULL) {
        bool clauseIsTrue = false;
        bool clauseIsUndefined = false;

        literal *currentLiteral = currentClause->literalHead;
        
        // Percorre os literais dentro de uma cláusula específica
        while (currentLiteral != NULL) {
            short val = pi->valores[currentLiteral->variable];
            
            if (val == UNDEFINED) { clauseIsUndefined = true; }
            else {
                // Checa (se a variável == 1 e NÃO for negada) OU (se a variável == 0 e FOR negada)
                if ((val == 1 && !currentLiteral->isNegative) || (val == 0 && currentLiteral->isNegative)) {
                    clauseIsTrue = true;
                    break; // Se achar 1 literal verdadeiro, a cláusula inteira já é verdadeira
                }
            }
            currentLiteral = currentLiteral->next;
        }

        // Se a clause é falsa e não possui literais indefinidos, a fórmula é definitivamente falsa
        if (!clauseIsTrue && !clauseIsUndefined) { return 0; }
        
        // Se a clause ainda não é verdadeira e possui literais indefinidos, é inclonclusivo
        if (!clauseIsTrue) { allClausesTrue = false; }
        
        currentClause = currentClause->next;
    }

    if (allClausesTrue) { return 1; }
    else { return UNDEFINED; } // Ainda precisa descer mais na árvore
}

// Cria uma cópia do interpretação atual, e adiciona o novo chute (0 ou 1) para a variável
partialInt cloneInterpretation(partialInt *oldPi, int totalVars, int currentVar, short guess) {
    partialInt newPi;
    newPi.valores = malloc(sizeof(short) * (totalVars + 1));
    for (int i = 1; i <= totalVars; i++) { newPi.valores[i] = oldPi->valores[i];}
    newPi.valores[currentVar] = guess;
    
    return newPi;
}

// Função recursiva que monta a Árvore de Decisão usando Backtracking
nodeBinaryTree* solveSAT(formula *f, partialInt *pi, int currentVar) {
    //Cria a raiz da árvore de decisão
    nodeBinaryTree *node = malloc(sizeof(nodeBinaryTree));
    node->literal = currentVar;
    node->interpretacao = *pi;
    node->left = NULL;
    node->right = NULL;

    //Avalia o estado da fórmula com os chutes atuais
    int status = evaluateFormula(f, pi);

    if (status == 1) { 
        node->isSAT = true;
        return node;
    }
 
    if (status == 0) { 
        node->isSAT = false;
        return node;
    }

    // Proteção: Se passamos do limite de variáveis e não deu SAT, o caminho falhou.
    if (currentVar > f->variableCount) {
        node->isSAT = false;
        return node;
    }

    //Ramificação(BACKTRACKING))
    //Ramo esquerdo: True
    partialInt piLeft = cloneInterpretation(pi, f->variableCount, currentVar, true);
    node->value = 1;
    node->left = solveSAT(f, &piLeft, currentVar + 1); // Desce na árvore (recursão)
    
    // Verifica se o caminho da esquerda deu SAT, com isso não precisa testar a direita
    if (node->left->isSAT) {
        node->isSAT = true;
        return node;
    }

    // Ramo direito: False
    partialInt piRight = cloneInterpretation(pi, f->variableCount, currentVar, false);
    node->value = 0;
    node->right = solveSAT(f, &piRight, currentVar + 1); // Desce na árvore pela direita
    
    // Verifica se pelo caminho da direita deu SAT
    if (node->right->isSAT) {
        node->isSAT = true;
        return node;
    }

    // Se ambos falharam, o ramo não possue solução.
    node->isSAT = false;
    return node;
}


void printSolution(nodeBinaryTree *node, int totalVars) {
    if (node == NULL) return;

    // Se chegamos no nó final (a folha que deu SAT)
    if (node->isSAT && node->left == NULL && node->right == NULL) {
        printf("\nCombinacao de Sucesso:\n");
        for (int i = 1; i <= totalVars; i++) {
            printf("Var x%d = %d\n", i, node->interpretacao.valores[i]);
        }
        return;
    }

    // Procura o caminho do sucesso pela árvore para imprimir
    if (node->left && node->left->isSAT) {
        printSolution(node->left, totalVars);
    } else if (node->right && node->right->isSAT) {
        printSolution(node->right, totalVars);
    }
}


void freeFormula(formula *f) {
    if (f == NULL) return;
    clause *currClause = f->clauseHead;
    while (currClause != NULL) {
        clause *nextClause = currClause->next;
        literal *currLiteral = currClause->literalHead;
        while (currLiteral != NULL) {
            literal *nextLiteral = currLiteral->next;
            free(currLiteral);
            currLiteral = nextLiteral;
        }
        free(currClause);
        currClause = nextClause;
    }
    free(f);
}

void freeTree(nodeBinaryTree *node) {
    if (node == NULL) return;
    
    freeTree(node->left);
    freeTree(node->right);
    
    // Libera o array de interpretação que este nó copiou
    if (node->interpretacao.valores != NULL) { free(node->interpretacao.valores); }
    
    free(node);
}