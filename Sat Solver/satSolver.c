#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

// Undefined = -1; False = 0; True = 1
#define UNDEFINED -1 

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
    LIAConstraint **lookupTable;
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
formula* readFile(FILE *file, LIATheory *theory);

formula *initializeFormula();
LIATheory *initializeLIATheory();
partialInt initializePartialInterp(formula *f);

clause *addClause(formula *f);
literal *addLiteral(literal *l, int variable, bool isNegative);

bool readClauses(formula *f, FILE *file);
bool readEquations(formula *f, LIATheory* t, FILE *file);
bool evaluateMathematicalConsistency(formula *f, partialInt *pi, LIATheory *theory);
int evaluateFormula(formula *f, partialInt *pi);

DecisionNode* solveSAT(formula *f, partialInt *pi, int currentVar);
DecisionNode* solveSMT(formula *f, partialInt *pi, int currentVar, LIATheory *theory);

void printBooleanFormulaCNF(const formula *f);
void printLIATheoryConstraints(const LIATheory *theory);
void printSolution(DecisionNode *root, int totalVars, partialInt *pi, LIATheory *theory);

void freeFormula(formula *f);
void freeTree(DecisionNode *node);
void freeLIATheory(LIATheory *theory);



int main() {
    FILE *file = openFile();
    if (file == NULL) { return 1; }

    LIATheory *t = initializeLIATheory();
    
    // Chamada do leitor unificado. Ele preenche o 't' e devolve o 'f'
    formula *f = readFile(file, t);
    fclose(file);

    if (f == NULL) { 
        freeLIATheory(t);
        return 1; 
    }
    
    partialInt pi = initializePartialInterp(f);
    DecisionNode *root = NULL;
 
    if (t->totalConstraints == 0) {
        printf("[MODO SAT DETECTADO]\n");

        printf("\nFormula Booleana lida (CNF):\n");
        printBooleanFormulaCNF(f);

        root = solveSAT(f, &pi, 1);
    } else {
        printf("[MODO SMT LIA DETECTADO] - %d equacoes carregadas.\n", t->totalConstraints);
        
        printf("\nFormula Booleana lida (CNF mapeada):\n");
        printBooleanFormulaCNF(f);

        printf("\nRestricoes Matematicas Carregadas (LIA):\n");
        printLIATheoryConstraints(t);
        
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

// Cria um array que armazena a interpretação parcial de cada variável única da formula, inicializando todas como UNDEFINED
partialInt initializePartialInterp(formula *f){
    partialInt pi;
    pi.truthValue = malloc(sizeof(short) * (f->atomCount + 1)); 
    for (int i = 0; i < f->atomCount + 1; i++) { pi.truthValue[i] = UNDEFINED; }

    return pi;
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

// Faz a leitura da Fórmula a partir do arquivo fornecido 
formula* readFile(FILE *file, LIATheory *theory) {
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

                // Captura falha na leitura das clausulas
                if (!readClauses(inputFormula, file)) {
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

                // Captura falha na leitura das equações
                if (!readEquations(inputFormula, theory, file)) {
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
    
    // Aloca a tabela de busca baseada no número de átomos/variáveis
    if (inputFormula != NULL && theory != NULL) {
        theory->lookupTable = (LIAConstraint **) calloc(inputFormula->atomCount + 1, sizeof(LIAConstraint *));
        
        // Popula a tabela mapeando atomID -> Ponteiro da Restrição
        LIAConstraint *curr = theory->constraintListHead;
        while (curr != NULL) {
            if (curr->atomID <= inputFormula->atomCount) {
                theory->lookupTable[curr->atomID] = curr;
            }
            curr = curr->next;
        }
    }

    return inputFormula; 
}

bool readClauses(formula *f, FILE *file) {
    for (int i = 0; i < f->clauseCount; i++){
        clause *newClause = addClause(f);
        while(1){
            int temp;
            if (fscanf(file, "%d", &temp) != 1 || abs(temp) > f->atomCount) { 
                printf("ERROR: Erro na leitura das clausulas.\n");
                return false; // Informa o erro para quem chamou
            }

            if (temp != 0) { newClause->literalHead = addLiteral(newClause->literalHead, abs(temp), (temp<0)); }
            else { break; }

            newClause->literalCount++;
        }
    }
    return true; // Sucesso
}

bool readEquations(formula *f, LIATheory* t, FILE *file) {
    for (int i = 0; i < t->totalConstraints; i++) {
        int atomID, coefficient, constantValue;
        char mathVar;
        char buffer[128];

        // Lê o cabeçalho do literal mapeado "fX"
        if (fscanf(file, " f%d", &atomID) != 1) return false;

        // Lê o restante da linha até o fim para fazer a análise manual (parsing)
        if (fscanf(file, " %[^\n]", buffer) != 1) return false;

        int offset = 0;
        char sign = '0';
        char op[4] = "";
        int target = 0;

        // Tenta ler o formato completo com operador secundário: "2 x + 3 >= 9" ou "2x + 3 >= 9"
        int lidos = sscanf(buffer, "%d %c %c %d %2s %d", &coefficient, &mathVar, &sign, &offset, op, &target);

        // Se falhou, tenta ler o formato simples sem deslocamento: "2 x <= 9"
        if (lidos != 6) {
            sign = '0';
            offset = 0;
            lidos = sscanf(buffer, "%d %c %2s %d", &coefficient, &mathVar, op, &target);
    
            if (lidos != 4) {
                printf("ERROR: Erro de formatacao na equacao SMT: '%s'\n", buffer);
                return false;
            }
        }

        LIAConstraint *newConstraint = (LIAConstraint *) malloc(sizeof(LIAConstraint));
        if (newConstraint == NULL) return false;
        
        newConstraint->atomID = atomID;
        newConstraint->coefficient = coefficient;
        newConstraint->mathVar = mathVar;
        newConstraint->innerSign = sign;
        newConstraint->innerOffset = offset;
        strcpy(newConstraint->operatorSymbol, op);
        newConstraint->constantValue = target;
        
        newConstraint->next = t->constraintListHead;
        t->constraintListHead = newConstraint;
    }
    return true;
}
// Cria uma clause vazia, e adiciona no início da lista encadeada de cláusulas
clause *addClause(formula *f){
    clause *newClause = malloc(sizeof(clause));
    newClause->literalCount = 0;
    newClause->literalHead = NULL;
    newClause->next = f->clauseHead;
    f->clauseHead = newClause;
    return newClause;
}

// Cria e preenche literal, colocando-o na lista encadeada de uma cláusula específica
literal *addLiteral(literal *l, int variable, bool isNegative){
    literal *newLiteral = (literal *)malloc(sizeof(literal));
    newLiteral->atomID = variable;
    newLiteral->isNegative = isNegative;
    newLiteral->next = l;
    return newLiteral;
}


// Imprime a fórmula lógica no formato clássico de Conjunção de Disjunções (CNF)
void printBooleanFormulaCNF(const formula *f) {
    clause *currentClause = f->clauseHead;
    while (currentClause != NULL) {
        printf("(");
        literal *currentLiteral = currentClause->literalHead;
        while (currentLiteral != NULL) {
            if (currentLiteral->isNegative) { 
                printf("~%d", currentLiteral->atomID); 
            } else { 
                printf("%d", currentLiteral->atomID); 
            }

            if (currentLiteral->next != NULL) { printf(" V "); }
            currentLiteral = currentLiteral->next;
        }
        printf(")");
        if (currentClause->next != NULL) { printf(" ^ "); }
        currentClause = currentClause->next;
    }
    printf("\n");
}

// Imprime a lista de equações lineares inteiras (Teoria LIA) carregadas do arquivo
void printLIATheoryConstraints(const LIATheory *theory) {
    LIAConstraint *current = theory->constraintListHead;
    if (current == NULL) {
        printf("Nenhuma equacao matematica carregada.\n");
        return;
    }

    while (current != NULL) {
        // Exemplo de saída esperada: "f1: 2x + 3 >= 9" ou "f2: 3x <= 10"
        printf("  f%d: %dx", current->atomID, current->coefficient);
        
        // Se houver deslocamento interno (ex: + 3 ou - 1), imprime-o
        if (current->innerSign == '+' || current->innerSign == '-') {
            printf(" %c %d", current->innerSign, current->innerOffset);
        }
        
        printf(" %s %d\n", current->operatorSymbol, current->constantValue);
        current = current->next;
    }
}


// operação de arredondamento pra baixo para inteiros
int floor_div(int b, int a) {
    int res = b / a;
    int rem = b % a;
    if (rem != 0 && b < 0) res--;
    return res;
}

// operação de arredondamento pra cima para inteiros
int ceil_div(int b, int a) {
    int res = b / a;
    int rem = b % a;
    if (rem != 0 && b > 0) res++;
    return res;
}

Interval calculateLIAInterval(int totalVars, partialInt *pi, LIATheory *theory) {
    Interval validRange;
    validRange.minimumValue = INT_MIN; 
    validRange.maximumValue = INT_MAX; 

    LIAConstraint *currentConstraint = theory->constraintListHead;
    while (currentConstraint != NULL) {
        int val = pi->truthValue[currentConstraint->atomID];

        // Se a variável for UNDEFINED (-1), ignoramos pois o SAT ainda não tomou uma decisão sobre ela
        if (val == 1 || val == 0) {
            
            int adjustedConstant = currentConstraint->constantValue;
            if (currentConstraint->innerSign == '+') { adjustedConstant -= currentConstraint->innerOffset; } 
            else if (currentConstraint->innerSign == '-') { adjustedConstant += currentConstraint->innerOffset; }

            int a = currentConstraint->coefficient;
            char op[3];
            strcpy(op, currentConstraint->operatorSymbol);

            //Tratamento caso a clausula booleana seja negativa 
            if (val == 0) {
                if (strcmp(op, "<=") == 0)      strcpy(op, ">");   // Negação de <= é >
                else if (strcmp(op, "<") == 0)  strcpy(op, ">=");  // Negação de < é >=
                else if (strcmp(op, ">=") == 0) strcpy(op, "<");   // Negação de >= é <
                else if (strcmp(op, ">") == 0)  strcpy(op, "<=");  // Negação de > é <=
            }

            // Se 'a' for negativo, multiplicamos a inequação por -1, invertendo a inequação. ( '<' vira '>')
            if (a < 0) {
                a = -a;
                adjustedConstant = -adjustedConstant;
                if (strcmp(op, "<=") == 0)      strcpy(op, ">=");
                else if (strcmp(op, "<") == 0)  strcpy(op, ">");
                else if (strcmp(op, ">=") == 0) strcpy(op, "<=");
                else if (strcmp(op, ">") == 0)  strcpy(op, "<");
            }

            // Adaptação dos operadores estritos (< e >) para seus equivalentes inclusivos (<= e >=).
            // Usamos floor_div e ceil_div para garantir o arredondamento correto.

            // Caso 1: ax <= b
            if (strcmp(op, "<=") == 0) {
                int limit = floor_div(adjustedConstant, a);
                if (limit < validRange.maximumValue) validRange.maximumValue = limit;
            } 
            // Caso 2: ax < b
            else if (strcmp(op, "<") == 0) {
                int limit = floor_div(adjustedConstant - 1, a);
                if (limit < validRange.maximumValue) validRange.maximumValue = limit;
            } 
            // Caso 3: ax >= b
            else if (strcmp(op, ">=") == 0) {
                int limit = ceil_div(adjustedConstant, a);
                if (limit > validRange.minimumValue) validRange.minimumValue = limit;
            } 
            // Caso 4: ax > b
            else if (strcmp(op, ">") == 0) {
                int limit = ceil_div(adjustedConstant + 1, a);
                if (limit > validRange.minimumValue) validRange.minimumValue = limit;
            }
        }
        currentConstraint = currentConstraint->next;
    }
    return validRange;
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

bool evaluateMathematicalConsistency(formula *f, partialInt *pi, LIATheory *theory) {
    Interval validRange = calculateLIAInterval(f->atomCount, pi, theory);
    
    if (validRange.minimumValue > validRange.maximumValue) {
        return false; // Conflito matemático detectado
    }
    return true; 
}


// Árvore de Decisão SMT Modificada para Consistência Incremental (Early Pruning)
// Verifica se a atribuição lógica atual é matematicamente viável e não possui contradições.
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

    // 1. Tenta ramo VERDADEIRO (1)
    pi->truthValue[currentVar] = 1; 
    node->value = 1;
    
    // Só desce na recursão se o estado atual for matematicamente consistente
    if (evaluateMathematicalConsistency(f, pi, theory)) {
        node->left = solveSMT(f, pi, currentVar + 1, theory);
        if (node->left->isSAT) {
            node->isSAT = true;
            return node; 
        }
    }

    // Tenta ramo FALSO (0)
    pi->truthValue[currentVar] = 0; 
    node->value = 0;
    
    // Só desce na recursão se o estado atual for matematicamente consistente
    if (evaluateMathematicalConsistency(f, pi, theory)) {
        node->right = solveSMT(f, pi, currentVar + 1, theory); 
        if (node->right->isSAT) {
            node->isSAT = true;
            return node;
        }
    }

    // Se chegou aqui, ambos os lados falharem (ou forem inconsistentes), limpa e retorna UNSAT para este nó
    pi->truthValue[currentVar] = UNDEFINED; 
    node->isSAT = false;
    return node;
}

// Árvore de Decisão Recurrsiva, resolve o SAT usando Backtracking
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

    //Ramificação(BACKTRACKING))
    // Ramo com variável VERDADEIRA (1)
    pi->truthValue[currentVar] = 1; // Modifica o array global diretamente
    node->value = 1;
    node->left = solveSAT(f, pi, currentVar + 1);
    
    if (node->left->isSAT) {
        node->isSAT = true;
        return node; // Achou SAT, sobe sem desfazer a modificação.
    }

    // Se o caminho 1 falhou, tentamos o ramo com a variável FALSA (0)
    pi->truthValue[currentVar] = 0; // SOBRESCREVE A MODIFICAÇÃO ANTERIOR
    node->value = 0;
    node->right = solveSAT(f, pi, currentVar + 1); 
    
    if (node->right->isSAT) {
        node->isSAT = true;
        return node;
    }

    // Se chegou aqui, AMBOS falharam
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

        if (theory != NULL && theory->totalConstraints > 0) {

            // Invoca a função centralizada corrigida para obter o intervalo final
            Interval iv = calculateLIAInterval(totalVars, pi, theory);
            char v = (theory->constraintListHead != NULL) ? theory->constraintListHead->mathVar : 'x'; 
            
            printf("\nSolucoes inteiras para '%c':\n", v);
            if (iv.minimumValue == INT_MIN && iv.maximumValue == INT_MAX) {
                printf("'%c' pode ser qualquer numero inteiro (sem restricoes ativas).\n", v);
            } else if (iv.minimumValue == INT_MIN) {
                printf("%c <= %d\n", v, iv.maximumValue);
            } else if (iv.maximumValue == INT_MAX) {
                printf("%c >= %d\n", v, iv.minimumValue);
            } else {
                printf("%d <= %c <= %d\n", iv.minimumValue, v, iv.maximumValue);
            }

        } else {
            printf("Atribuicao Logica:\n");
            for (int i = 1; i <= totalVars; i++) {
                printf("  Var %d = %d\n", i, pi->truthValue[i]);
            }
        }
    }
}


// Gestores de armazenamento
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
    if (theory->lookupTable != NULL) { free(theory->lookupTable); }
    free(theory);
}

void freeTree(DecisionNode *node) {
    if (node == NULL) return;
    
    freeTree(node->left);
    freeTree(node->right);
    
    free(node);
}