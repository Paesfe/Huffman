#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define UNDEFINED -1

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

// Estrutura do nó da árvore de decisão
typedef struct DecisionNode {
    bool isSAT;                     // Indica o resultado do caminho (SAT ou UNSAT)
    int decisionAtomID;             // ID da Variável Booleana
    short polarity;                 // Valor Booleano Atribuído ao nó (TRUE, FALSE ou UNDEFINED)
    struct DecisionNode *left;      // Ramo Positivo (TRUE)
    struct DecisionNode *right;     // Ramo Negativo (FALSE)
    struct DecisionNode *parent;    // Ramo de volta ao pai — torna a árvore o estado
} DecisionNode;


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

// Faz a leitura das Clausulas que compoem a Formula
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
    return node;
}

// Árvore de Decisão Recursiva, resolve o SAT usando Backtracking
DecisionNode* solveSAT(Formula *f, int currentVar, DecisionNode *parent) {
    
    // Verifica o estado da formula com as atribuições atuais
    int result = evaluateFormula(f, parent);
    if (result == true)  { 
        DecisionNode *leaf = createDecisionNode(currentVar, parent);
        if (leaf) leaf->isSAT = true;
        return leaf;
    }
    if (result == false)  { 
        DecisionNode *leaf = createDecisionNode(currentVar, parent);
        if (leaf) leaf->isSAT = false;
        return leaf;
    }

    // Ramificação (Somente se booleanEvaluationResult == Indefinida)
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


FILE *openFile() {
    char filename[256];
    char filepath[512];

    printf("Digite o nome do arquivo a ser lido: ");
    scanf("%255s", filename);
    
    // 1ª Tentativa: Abre o caminho exato que você digitou
    FILE *file = fopen(filename, "r");
    if (file != NULL) { return file; } 
    
    // 2ª Tentativa: Se você estiver rodando de dentro da pasta 'src',
    // ele volta uma pasta e entra em 'Test Cases/'
    snprintf(filepath, sizeof(filepath), "../Test Cases/%s", filename);
    file = fopen(filepath, "r"); 
    if (file != NULL) { return file; }

    // 3ª Tentativa: Caso o terminal esteja aberto na raiz do projeto 'Sat Solver',
    // ele tenta buscar direto a partir dali
    snprintf(filepath, sizeof(filepath), "Test Cases/%s", filename);
    file = fopen(filepath, "r"); 
    if (file != NULL) { return file; }

    // Se chegou até aqui, é porque falhou nas três tentativas
    printf("Erro ao abrir o arquivo: '%s'\n", filename);
    
    return NULL;
}

// Faz a leitura da Fórmula a partir do arquivo fornecido (somente seções CNF)
Formula* readFile(FILE *file) {
    char comment[128];
    char format[8];
    char command; 

    Formula *inputFormula = initializeFormula();
    if (inputFormula == NULL) return NULL;

    while(fscanf(file, " %c", &command) != EOF) {
        switch (command){
            case 'c': 
                fgets(comment, sizeof(comment), file); 
                break;

            case 'p': 
                fscanf(file, "%s %d %d", format, &inputFormula->atomCount, &inputFormula->clauseCount);
                
                if (strcmp(format, "cnf") != 0){
                    printf("ERROR: Formato nao suportado. Esperado 'cnf'.\n");
                    freeFormula(inputFormula);
                    return NULL;
                }

                // Captura falha na leitura das clausulas
                if (!readClauses(inputFormula, file)) {
                    freeFormula(inputFormula);
                    return NULL;
                }

                break;

            default:
                printf("ERROR: Comando desconhecido '%c'.\n", command); 
                freeFormula(inputFormula);
                return NULL;
        }
    }

    return inputFormula;
}

void printSolution(DecisionNode *root, Formula *f) {
    if (root == NULL) return;

    if (!root->isSAT) {
        printf("\nResultado final: UNSAT\n");
        printf("Nenhuma combinacao torna a formula verdadeira.\n");
        return;
    } 
    
    printf("\nResultado final: SAT\n");
    printf("Atribuicao Logica:\n");

    DecisionNode *curr = root;
    while (curr != NULL) {
        if (curr->decisionAtomID <= f->atomCount) {
            printf("  Var %d = %d\n", curr->decisionAtomID, curr->polarity);
        }
        // Desce pelo ramo que levou ao SAT
        if (curr->polarity == 1 && curr->left  != NULL) curr = curr->left;
        else if (curr->right != NULL) curr = curr->right;
        else break;
    }
}


int main() {
    FILE *file = openFile();
    if (file == NULL) { return 1; }

    Formula *f = readFile(file);
    fclose(file);

    if (f == NULL) { return 1; }

    printf("[MODO SAT]\n");
    printf("\nFormula Booleana lida (CNF):\n");
    printBooleanFormulaCNF(f);

    // solveSAT(f, 1, NULL) — começa na variável 1, sem pai (raiz)
    DecisionNode *root = solveSAT(f, 1, NULL);

    printf("\nProcessando a Arvore de Decisao...\n");
    printSolution(root, f);

    freeTree(root);
    freeFormula(f);

    return 0;
}