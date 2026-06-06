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
    int atomID;             
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
    int atomCount;     
} formula;

// Estrutura que guarda a interpretação atual
// Undefined = -1; False = 0; True = 1
typedef struct partialInt{
    short *truthValue; 
} partialInt;

// Estrutura da arvore de decisão
typedef struct DecisionNode{
    int decisionAtomID;                    // Qual variável este nó está testando
    short value;                           // (0 ou 1)
    struct DecisionNode *left;             // Ramo positivo (True)
    struct DecisionNode *right;            // Ramo negativo (False)
    bool isSAT;                            // Se o caminho resultou em SAT ou UNSAT
} DecisionNode;


formula *initializeFormula();
formula* createFormulaCNF(FILE *file);
int formulaReader(formula *f, FILE *file);
clause *addClause(formula *f);
literal *addLiteral(literal *l, int variable, bool isNegative);
int clauseReader(clause *c, formula *f, FILE *file);
void printFormula(formula *f);
partialInt initializePartialInterp(formula *f);
int evaluateFormula(formula *f, partialInt *pi);
DecisionNode* solveSAT(formula *f, partialInt *pi, int currentVar);
void printSolution(DecisionNode *node, int totalVars, partialInt *pi);
void freeFormula(formula *f);
void freeTree(DecisionNode *node);

int main() {
    char filename[256];
    char filepath[512];

    printf("Digite o nome do arquivo a ser lido: ");
    scanf("%255s", filename);
    snprintf(filepath, sizeof(filepath), "../%s", filename);

    // Tenta abrir o arquivo com o nome digitado
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo: '%s'\n", filename);
        printf("Verifique se ele esta na mesma pasta do programa.\n");
        return 1;
    }

    formula *f = createFormulaCNF(file);
    fclose(file);

    if (f == NULL) { return 1; }
    
    printf("\nFormula lida (CNF)\n");
    printFormula(f);
    
    partialInt pi = initializePartialInterp(f);

    printf("\nProcessando a Arvore de Decisao...\n");
    DecisionNode *root = solveSAT(f, &pi, 1);

    printSolution(root, f->atomCount, &pi);

    freeTree(root);
    freeFormula(f);
    free(pi.truthValue);

    return 0;
}

formula *initializeFormula(){
    formula *newFormula = (formula *) malloc(sizeof(formula));
    newFormula->clauseHead = NULL;
    newFormula->clauseCount = 0;
    newFormula->atomCount = 0;
    return newFormula;
}

// Faz a leitura da Fórmula a partir da entrada padrão
formula* createFormulaCNF(FILE *file) {
    int maxCommentSize = 128;
    char comment[maxCommentSize];
    char format[8];
    char command; 

    formula *inputFormula = initializeFormula();
    
    while(fscanf(file, " %c", &command) != EOF) {
        switch (command){
            case 'c': 
                fgets(comment, maxCommentSize, file); 
                break;

            case 'p': 
                fscanf(file, "%s %d %d", format, &inputFormula->atomCount, &inputFormula->clauseCount);
                fgetc(file); // Limpa o '\n' do buffer

                if (strcmp(format, "cnf") != 0){
                    printf("ERROR: Formato não suportado. Esperado 'cnf'.\n");
                    free(inputFormula);
                    return NULL;
                }

                // Passe o arquivo para a próxima função
                if (formulaReader(inputFormula, file) == UNDEFINED){
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
int formulaReader(formula *f, FILE *file) {
    for (int i = 0; i < f->clauseCount; i++){
        clause *newClause = addClause(f);
        if ( clauseReader(newClause, f, file) == UNDEFINED){ return UNDEFINED; }
    }
    return 0;
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
    newLiteral->atomID = variable;
    newLiteral->isNegative = isNegative;
    newLiteral->next = l;
    return newLiteral;
}

// Lê individualmente cada literal dentro de uma determinada clausula, até encontrar 0
int clauseReader(clause *c, formula *f, FILE *file){
    while(1){
        int temp;
        fscanf(file, "%d", &temp);

        // Verifica se não excede o limite definido pelo cabeçalho
        if (abs(temp) > f->atomCount){ return UNDEFINED; }

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
            if (currentLiteral->isNegative) { printf("~%d", currentLiteral->atomID); }
            else { printf("%d", currentLiteral->atomID); }

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
    pi.truthValue = malloc(sizeof(short) * (f->atomCount + 1)); 
    for (int i = 0; i < f->atomCount + 1; i++) { pi.truthValue[i] = UNDEFINED; }

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
            short val = pi->truthValue[currentLiteral->atomID];
            
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

// Função recursiva que monta a Árvore de Decisão usando Backtracking
DecisionNode* solveSAT(formula *f, partialInt *pi, int currentVar) {
    //Cria a raiz da árvore de decisão
    DecisionNode *node = malloc(sizeof(DecisionNode));
    node->decisionAtomID = currentVar;
    node->left = NULL;
    node->right = NULL;

    //Avalia o estado da fórmula com os chutes atuais
    int evaluationResult = evaluateFormula(f, pi);

    if (evaluationResult == 1) { 
        node->isSAT = true;
        return node;
    }
 
    if (evaluationResult == 0) { 
        node->isSAT = false;
        return node;
    }

    // Proteção: Se passamos do limite de variáveis e não deu SAT, o caminho falhou.
    if (currentVar > f->atomCount) {
        node->isSAT = false;
        return node;
    }

    //Ramificação(BACKTRACKING))
    // 1. Caminho assumindo que a variável é VERDADEIRA (1)
    pi->truthValue[currentVar] = 1; // MODIFICA O ARRAY GLOBAL DIRETAMENTE
    node->value = 1;
    node->left = solveSAT(f, pi, currentVar + 1);
    
    if (node->left->isSAT) {
        node->isSAT = true;
        return node; // Achou SAT, sobe sem desfazer a modificação.
    }

    // 2. Se o caminho 1 falhou, tentamos o Caminho assumindo FALSA (0)
    pi->truthValue[currentVar] = 0; // SOBRESCREVE A MODIFICAÇÃO ANTERIOR
    node->value = 0;
    node->right = solveSAT(f, pi, currentVar + 1); 
    
    if (node->right->isSAT) {
        node->isSAT = true;
        return node;
    }

    // 3. Se AMBOS falharam
    // Desfaz a alteração feita, para que o nó pai receba receba o array no estado original
    pi->truthValue[currentVar] = UNDEFINED; 

    node->isSAT = false;
    return node;
}

// Como usamos um array único, 'pi' já volta da árvore com a solução gravada nele.
void printSolution(DecisionNode *root, int totalVars, partialInt *pi) {
    if (root == NULL) return;

    if (!root->isSAT) {
        printf("\nResultado final: UNSAT\n");
        printf("Nenhuma combinacao torna a formula verdadeira.\n");
    } else {
        printf("\nResultado final: SAT\n");
        printf("Combinacao de Sucesso:\n");
        for (int i = 1; i <= totalVars; i++) {
            printf("Var x%d = %d\n", i, pi->truthValue[i]);
        }
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

void freeTree(DecisionNode *node) {
    if (node == NULL) return;
    
    freeTree(node->left);
    freeTree(node->right);
    
    free(node);
}