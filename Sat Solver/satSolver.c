#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

// Undefined = -1; False = 0; True = 1
#define UNDEFINED -1 

// Representa uma única equação da Teoria LIA (ex: 2x <= 9)
typedef struct LIAConstraint {
    int atomID;                 // ID da variável Booleana que ativa esta equação
    int coefficient;            // Multiplicador da variável matemática 'x' (ex: o '2' em 2x)
    char mathVar;               // Letra da variável
    int constantValue;          // O valor após o operador (ex: o '9' em <= 9)
    bool isUpperBound;          // Verdadeiro se for um limite de teto (<=). Falso se for limite de piso (>=).
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

FILE *openFile();
formula *initializeFormula();
formula* createFormulaCNF(FILE *file, LIATheory *theory);
int formulaReader(formula *f, FILE *file);
clause *addClause(formula *f);
literal *addLiteral(literal *l, int variable, bool isNegative);
int clauseReader(clause *c, formula *f, FILE *file);
void printClauses(formula *f);
partialInt initializePartialInterp(formula *f);
int evaluateFormula(formula *f, partialInt *pi);
DecisionNode* solveSAT(formula *f, partialInt *pi, int currentVar);
void printSolution(DecisionNode *root, int totalVars, partialInt *pi, LIATheory *theory);
void freeFormula(formula *f);
void freeTree(DecisionNode *node);

bool evaluateMathematicalConsistency(formula *f, partialInt *pi, LIATheory *theory);
DecisionNode* solveSMT(formula *f, partialInt *pi, int currentVar, LIATheory *theory);
void freeLIATheory(LIATheory *theory);
LIATheory *initializeLIATheory();


int main() {
    FILE *file = openFile();
    if (file == NULL) { return 1; }

    // 1. Inicializamos a Teoria AQUI no main
    LIATheory *t = initializeLIATheory();
    
    // 2. Chamamos o leitor unificado. Ele preenche o 't' e nos devolve o 'f'
    formula *f = createFormulaCNF(file, t);
    fclose(file);

    if (f == NULL) { 
        freeLIATheory(t);
        return 1; 
    }
    
    partialInt pi = initializePartialInterp(f);
    DecisionNode *root = NULL;
 
    if (t->totalConstraints == 0) {
        printf("[MODO SAT DETECTADO]\n");

        printf("\nFormula lida (CNF)\n");
        printClauses(f);

        root = solveSAT(f, &pi, 1);
    } else {
        printf("[MODO SMT LIA DETECTADO] - %d equacoes carregadas.\n", t->totalConstraints);
        root = solveSMT(f, &pi, 1, t);
    }

    printf("\nProcessando a Arvore de Decisao...\n");
    printSolution(root, f->atomCount, &pi, t);

    freeTree(root);
    freeFormula(f);
    freeLIATheory(t);
    free(pi.truthValue);

    return 0;
}

FILE *openFile() {
    char filename[256];

    printf("Digite o nome do arquivo a ser lido: ");
    scanf("%255s", filename);
    
    // Tenta abrir o arquivo diretamente (caso o terminal esteja na pasta certa)
    FILE *file = fopen(filename, "r");
    if (file != NULL) { return file; } 
    
    // Se falhou, tenta buscar na pasta de trás
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "../%s", filename);
    
    FILE *fallbackFile = fopen(filepath, "r"); 
    if (fallbackFile != NULL) { return fallbackFile; }

    // Se chegou até aqui, é porque falhou nas duas tentativas
    printf("Erro ao abrir o arquivo: '%s'\n", filename);
    
    return NULL;
}

formula *initializeFormula(){
    formula *newFormula = (formula *) malloc(sizeof(formula));
    newFormula->clauseHead = NULL;
    newFormula->clauseCount = 0;
    newFormula->atomCount = 0;
    return newFormula;
}

LIATheory *initializeLIATheory() {
    LIATheory *theory = (LIATheory *) malloc(sizeof(LIATheory));
    theory->constraintListHead = NULL;
    theory->totalConstraints = 0;
    return theory;
}

// Faz a leitura da Fórmula a partir da entrada padrão
// Leitor Unificado (Máquina de Estados)
formula* createFormulaCNF(FILE *file, LIATheory *theory) {
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
                
                if (strcmp(format, "cnf") != 0){
                    printf("ERROR: Formato nao suportado. Esperado 'cnf'.\n");
                    freeFormula(inputFormula);
                    return NULL;
                }

                // Lê as cláusulas lógicas
                if (formulaReader(inputFormula, file) == UNDEFINED){
                    printf("ERROR: Erro na leitura das clausulas.\n");
                    freeFormula(inputFormula);
                    return NULL;
                }
                break;

            case 't':
                fscanf(file, "%s %d", format, &theory->totalConstraints);
                
                if (strcmp(format, "lia") != 0){
                    printf("ERROR: Formato nao suportado. Esperado 'lia'.\n");
                    freeFormula(inputFormula);
                    return NULL;
                }

                // Lê as equações matemáticas (Substituindo o uso errado do formulaReader)
                for (int i = 0; i < theory->totalConstraints; i++) {
                    int atomID, coefficient, constantValue;
                    char operatorSymbol[4]; 
                    char mathVar;
                    
                    // Ele aceita tanto "f1 2x <= 10" quanto "f1 2 x <= 10" com espaço!
                    int lidos = fscanf(file, " f%d %d %c %3s %d", &atomID, &coefficient, &mathVar, operatorSymbol, &constantValue);

                    if (lidos != 5) {
                        printf("ERROR: Erro de formatacao na equacao SMT.\n");
                        freeFormula(inputFormula);
                        return NULL;
                    }

                    LIAConstraint *newConstraint = (LIAConstraint *) malloc(sizeof(LIAConstraint));
                    newConstraint->atomID = atomID;
                    newConstraint->coefficient = coefficient;
                    newConstraint->mathVar = mathVar;
                    newConstraint->constantValue = constantValue;
                    newConstraint->isUpperBound = (strcmp(operatorSymbol, "<=") == 0);
                    
                    newConstraint->next = theory->constraintListHead;
                    theory->constraintListHead = newConstraint;
                }
                break;

            default:
                printf("ERROR: Comando desconhecido '%c'.\n", command); 
                freeFormula(inputFormula);
                return NULL;
        }
    }
    
    // Retorna a fórmula preenchida corretamente no final do laço!
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
void printClauses(formula *f){
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

// Verifica se as equações ativadas pelo SAT Solver fazem sentido matematicamente
bool evaluateMathematicalConsistency(formula *f, partialInt *pi, LIATheory *theory) {
    Interval validRange;
    validRange.minimumValue = INT_MIN; 
    validRange.maximumValue = INT_MAX; 

    LIAConstraint *currentConstraint = theory->constraintListHead;
    
    while (currentConstraint != NULL) {
        if (currentConstraint->atomID <= f->atomCount && 
            pi->truthValue[currentConstraint->atomID] == 1) {
            
            if (currentConstraint->isUpperBound) {
                int calculatedLimit = currentConstraint->constantValue / currentConstraint->coefficient; 
                if (calculatedLimit < validRange.maximumValue) { 
                    validRange.maximumValue = calculatedLimit; 
                }
            } else {
                int calculatedLimit = (currentConstraint->constantValue + currentConstraint->coefficient - 1) / currentConstraint->coefficient; 
                if (calculatedLimit > validRange.minimumValue) { 
                    validRange.minimumValue = calculatedLimit; 
                }
            }
        }
        currentConstraint = currentConstraint->next;
    }

    if (validRange.minimumValue > validRange.maximumValue) {
        return false; 
    }
    return true; 
}

// Árvore de Decisão SMT (DPLL-T)
DecisionNode* solveSMT(formula *f, partialInt *pi, int currentVar, LIATheory *theory) {
    DecisionNode *node = malloc(sizeof(DecisionNode));
    node->decisionAtomID = currentVar;
    node->left = NULL;
    node->right = NULL;

    int booleanEvaluationResult = evaluateFormula(f, pi);

    if (booleanEvaluationResult == 1) { 
        bool isMathematicallyValid = evaluateMathematicalConsistency(f, pi, theory);
        node->isSAT = isMathematicallyValid; 
        return node; 
    }
 
    if (booleanEvaluationResult == 0) { 
        node->isSAT = false;
        return node;
    }

    if (currentVar > f->atomCount) {
        node->isSAT = false;
        return node;
    }

    pi->truthValue[currentVar] = 1; 
    node->value = 1;
    node->left = solveSMT(f, pi, currentVar + 1, theory);
    
    if (node->left->isSAT) {
        node->isSAT = true;
        return node; 
    }

    pi->truthValue[currentVar] = 0; 
    node->value = 0;
    node->right = solveSMT(f, pi, currentVar + 1, theory); 
    
    if (node->right->isSAT) {
        node->isSAT = true;
        return node;
    }

    pi->truthValue[currentVar] = UNDEFINED; 
    node->isSAT = false;
    return node;
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

void printSolution(DecisionNode *root, int totalVars, partialInt *pi, LIATheory *theory) {
    if (root == NULL) return;

    if (!root->isSAT) {
        printf("\nResultado final: UNSAT\n");
        printf("Nenhuma combinacao torna a formula verdadeira ou ha conflito matematico.\n");
    } else {
        printf("\nResultado final: SAT\n");

        // Verifica se é modo SMT (tem equações carregadas)
        if (theory != NULL && theory->totalConstraints > 0) {
            printf("Equacoes LIA ativadas (Verdadeiras):\n");
            
            // Percorre a lista de equações e imprime as que são verdadeiras
            LIAConstraint *current = theory->constraintListHead;
            while (current != NULL) {
                // Se a variável associada à equação for Verdadeira (1), imprime a equação
                if (current->atomID <= totalVars && pi->truthValue[current->atomID] == 1) {
                    printf("  %d%c %s %d\n", 
                           current->coefficient, 
                           current->mathVar, 
                           current->isUpperBound ? "<=" : ">=", 
                           current->constantValue);
                }
                current = current->next;
            }

            // Calcula e imprime o intervalo final da variável
            Interval iv = { INT_MIN, INT_MAX };
            LIAConstraint *c = theory->constraintListHead;
            char v = (c != NULL) ? c->mathVar : 'x'; 
            
            while (c != NULL) {
                if (c->atomID <= totalVars && pi->truthValue[c->atomID] == 1) {
                    if (c->isUpperBound) {
                        int lim = c->constantValue / c->coefficient;
                        if (lim < iv.maximumValue) { iv.maximumValue = lim; }
                    } else {
                        int lim = (c->constantValue + c->coefficient - 1) / c->coefficient;
                        if (lim > iv.minimumValue) { iv.minimumValue = lim; }
                    }
                }
                c = c->next;
            }

            printf("\nSolucoes inteiras para '%c':\n", v);
            if (iv.minimumValue == INT_MIN && iv.maximumValue == INT_MAX) {
                printf("  '%c' pode ser qualquer numero inteiro (sem restricoes ativas).\n", v);
            } else if (iv.minimumValue == INT_MIN) {
                printf("  %c <= %d\n", v, iv.maximumValue);
            } else if (iv.maximumValue == INT_MAX) {
                printf("  %c >= %d\n", v, iv.minimumValue);
            } else {
                printf("  %d <= %c <= %d\n", iv.minimumValue, v, iv.maximumValue);
            }

        } else {
            // Se for MODO SAT PURO, mantém o print tradicional
            printf("Atribuicao Logica:\n");
            for (int i = 1; i <= totalVars; i++) {
                printf("  Var %d = %d\n", i, pi->truthValue[i]);
            }
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

void freeLIATheory(LIATheory *theory) {
    if (theory == NULL) return;
    LIAConstraint *current = theory->constraintListHead;
    while (current != NULL) {
        LIAConstraint *next = current->next;
        free(current);
        current = next;
    }
    free(theory);
}

void freeTree(DecisionNode *node) {
    if (node == NULL) return;
    
    freeTree(node->left);
    freeTree(node->right);
    
    free(node);
}