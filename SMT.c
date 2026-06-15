#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

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

// Estrutura de uma única equação da teoria LIA
typedef struct LIAConstraint {
    int atomID;                     // ID da variável Booleana relacionada
    int coefficient;                // Coeficiente angular da reta: 'a' em 'ax + b = k' 
    char mathVar;                   // A letra da variavel
    char innerSign;                 // '+' ou '-' se houver deslocamento interno, ou '0' se não houver
    int innerOffset;                // Coeficiente linear da reta: 'b' em 'ax + b = k'
    char operatorSymbol[3];         // Armazena "<=", ">=", "<", ">" e "=="
    int constantValue;              // O valor após o operador: 'k' em 'ax + b = k' 
    struct LIAConstraint *next;     // Ponteiro para a próxima equação na lista
} LIAConstraint;

// Lista encadeada de todas as equações 
typedef struct LIATheory {
    LIAConstraint *constraintListHead;
    int totalConstraints;
} LIATheory;

// Estrutura que armazena o intervalo válido para 'x'
typedef struct Interval {
    int minimumValue;
    int maximumValue;
} Interval;



// ==========================================
// PROTÓTIPOS DAS FUNÇÕES (ASSINATURAS)
// ==========================================
Formula *initializeFormula();
void freeFormula(Formula *f);
void freeTree(DecisionNode *node);
bool readClauses(Formula *f, FILE *file);

LIATheory *initializeLIATheory();
void freeLIATheory(LIATheory *theory);
bool readEquations(Formula *f, LIATheory* t, FILE *file);
Interval calculateLIAInterval(DecisionNode *leaf, LIATheory *theory);

DecisionNode* solveSAT(Formula *f, int currentVar, DecisionNode *parent);
DecisionNode* solveSMT(Formula *f, int currentVar, DecisionNode *parent, LIATheory *theory);

void printBooleanFormulaCNF(const Formula *f);
void printLIATheoryConstraints(const LIATheory *theory);
short getVarValue(DecisionNode *leaf, int atomID);
// ==========================================



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

// Faz a leitura da Fórmula a partir do arquivo fornecido 
Formula* readFile(FILE *file, LIATheory *theory) {
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

    return inputFormula;
}

void printSolution(DecisionNode *root, Formula *f, LIATheory *theory) {
    if (root == NULL) return;

    if (!root->isSAT) {
        printf("\nResultado final: UNSAT\n");
        printf("Nenhuma combinacao torna a formula verdadeira ou ha conflito matematico.\n");
        return;
    } 
    
    printf("\nResultado final: SAT\n");

    // Checagem para o tipo de saida, é SMT ou é SAT?
    if (theory != NULL && theory->totalConstraints > 0) {
        // Desce pelo caminho vencedor até a folha SAT
        DecisionNode *curr = root;
        while (curr != NULL && (curr->left != NULL || curr->right != NULL)) {
            if (curr->polarity == 1 && curr->left  != NULL) curr = curr->left;
            else if (curr->right != NULL)                   curr = curr->right;
            else break;
        }

        Interval iv = calculateLIAInterval(curr, theory);
        char v = (theory->constraintListHead != NULL) ? theory->constraintListHead->mathVar : 'x';

        printf("\nSolucoes inteiras para '%c':\n", v);

        if (iv.minimumValue == INT_MIN && iv.maximumValue == INT_MAX) { printf("'%c' pode ser qualquer numero inteiro.\n", v); }
        else if (iv.minimumValue == INT_MIN) { printf("%c <= %d\n", v, iv.maximumValue); }
        else if (iv.maximumValue == INT_MAX) { printf("%c >= %d\n", v, iv.minimumValue); }
        else { printf("%d <= %c <= %d\n", iv.minimumValue, v, iv.maximumValue); } 

    } else {

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
}



// FUNÇÕES SAT


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


// FUNÇÕES SMT (LIA)


LIATheory *initializeLIATheory() {
    LIATheory *newTheory = (LIATheory *) malloc(sizeof(LIATheory));
    // Verifica a alocação do malloc
    if (newTheory == NULL) {
        fprintf(stderr, "ERRO: Falha ao alocar Formula.\n");
        return NULL;
    }
    newTheory->constraintListHead = NULL;
    newTheory->totalConstraints = 0;
    return newTheory;
}

// Faz a leitura das equações informadas no input
// Ler equações tanto na forma 'a*x >= k' quanto 'a*x +- b <= k' 
bool readEquations(Formula *f, LIATheory* t, FILE *file) {
    (void)f; // Silencia o warning do compilador
    
    for (int i = 0; i < t->totalConstraints; i++) {
        int atomID, coefficient;
        char mathVar;
        char buffer[128];

        // Lê o cabeçalho do literal mapeado "fX", ID da variavel booleana Relacionada
        if (fscanf(file, " f%d", &atomID) != 1) return false;

        // Lê o restante da linha até o fim para fazer a análise manual (parsing)
        if (fscanf(file, " %[^\n]", buffer) != 1) return false;

        int offset = 0;
        char sign = '0';
        char op[4] = "";
        int target = 0;

        // Tenta ler o formato completo com operador secundário: "a x + b >= k" ou "ax + b >= k"
        int lidos = sscanf(buffer, "%d %c %c %d %2s %d", &coefficient, &mathVar, &sign, &offset, op, &target);

        // Se falhou, tenta ler o formato simples sem deslocamento: "a*x <= k"
        if (lidos != 6) {
            sign = '0';
            offset = 0;
            lidos = sscanf(buffer, "%d %c %2s %d", &coefficient, &mathVar, op, &target);
    
            if (lidos != 4) {
                printf("ERROR: Erro de formatacao na equacao SMT: '%s'\n", buffer);
                return false;
            }
        }

        // Rejeita coeficiente zero
        if (coefficient == 0) {
            fprintf(stderr, "ERRO: Coeficiente zero na equacao: '%s'\n", buffer);
            return false;
        }

        LIAConstraint *newConstraint = (LIAConstraint *) malloc(sizeof(LIAConstraint));
        // Verifica a alocação do malloc
        if (newConstraint == NULL) {
            fprintf(stderr, "ERRO: Falha ao alocar LIAConstraint.\n");
            return false;
        }
        
        // Preenche a equação com os dados lidos
        newConstraint->atomID = atomID;
        newConstraint->coefficient = coefficient;
        newConstraint->mathVar = mathVar;
        newConstraint->innerSign = sign;
        newConstraint->innerOffset = offset;
        newConstraint->constantValue = target;

        strcpy(newConstraint->operatorSymbol, op);

        newConstraint->next = t->constraintListHead;
        t->constraintListHead = newConstraint;
    }
    return true;
}

// Inverte o operador relacional (negação booleana: ¬(ax OP b))
static void negateOperator(char op[3]) {
    if      (strcmp(op, "<=") == 0) strcpy(op, ">");
    else if (strcmp(op, "<")  == 0) strcpy(op, ">=");
    else if (strcmp(op, ">=") == 0) strcpy(op, "<");
    else if (strcmp(op, ">")  == 0) strcpy(op, "<=");
}

// Inverte o sentido do operador (multiplicação por -1 dos dois lados)
static void invertOperator(char op[3]) {
    if      (strcmp(op, "<=") == 0) strcpy(op, ">=");
    else if (strcmp(op, "<")  == 0) strcpy(op, ">");
    else if (strcmp(op, ">=") == 0) strcpy(op, "<=");
    else if (strcmp(op, ">")  == 0) strcpy(op, "<");
}

// Calcula o intervalo válido para a variavel, considerando as clausulas e equações informadas
Interval calculateLIAInterval(DecisionNode *leaf, LIATheory *theory) {
    Interval validRange = { INT_MIN, INT_MAX };

    LIAConstraint *currentConstraint = theory->constraintListHead;
    
    // Percorre toda a lista de equações matemáticas 
    while (currentConstraint != NULL) {
        // Acessa o valor Booleano da Variavel/ID associado a uma equação
        short val = getVarValue(leaf, currentConstraint->atomID);

        // Só processa a equação caso a variavel tenha um valor booleano definido (0 ou 1)
        if (val != UNDEFINED) {
            int adjustedConstant = currentConstraint->constantValue;
            char op[3];
            strcpy(op, currentConstraint->operatorSymbol);

            // Remove o coeficiente linear 'b'
            if (currentConstraint->innerSign == '+') { adjustedConstant -= currentConstraint->innerOffset; } 
            else if (currentConstraint->innerSign == '-') { adjustedConstant += currentConstraint->innerOffset; }

            // Acessa o valor do coeficiente Angular 'a'
            int a = currentConstraint->coefficient;
            
            // Tratamento de clausula booleana negativa 
            // ~(ax >= k) == (ax < k)
            if (val == 0) { negateOperator(op); } 

            // Tratamento do coeficiente angular negativo, multiplica a equação por (-1): ( '<' vira '>')
            if (a < 0) {
                a = -a;
                adjustedConstant = -adjustedConstant;
                invertOperator(op);
            }

            // Adaptação dos operadores estritos (< e >) para seus equivalentes inclusivos (<= e >=).
            // Usamos floorDiv e ceilDiv para garantir o arredondamento correto.

            // Caso 1: ax <= b
            if (strcmp(op, "<=") == 0) {
                // Converte para double, divide e arredonda para baixo
                int limit = (int)floor((double)adjustedConstant / a);
                if (limit < validRange.maximumValue) validRange.maximumValue = limit;
            } 
            // Caso 2: ax < b
            else if (strcmp(op, "<") == 0) {
                // Aplica a subtração antes da divisão, converte e arredonda para baixo
                int limit = (int)floor((double)(adjustedConstant - 1) / a);
                if (limit < validRange.maximumValue) validRange.maximumValue = limit;
            } 
            // Caso 3: ax >= b
            else if (strcmp(op, ">=") == 0) {
                // Converte para double, divide e arredonda para cima
                int limit = (int)ceil((double)adjustedConstant / a);
                if (limit > validRange.minimumValue) validRange.minimumValue = limit;
            } 
            // Caso 4: ax > b
            else if (strcmp(op, ">") == 0) {
                // Aplica a adição antes da divisão, converte e arredonda para cima
                int limit = (int)ceil((double)(adjustedConstant + 1) / a);
                if (limit > validRange.minimumValue) validRange.minimumValue = limit;
            }
        }
        // Avança para a próxima equação da lista
        currentConstraint = currentConstraint->next;
    }

    // Retorna o intervalo final estreitado. Se min > max, a teoria será declarada INCONSISTENTE!
    return validRange;
}

// Verifica se o intervalo atual é válido
bool evaluateMathematicalConsistency(DecisionNode *leaf, LIATheory *theory) {
    Interval validRange = calculateLIAInterval(leaf, theory);
    return (validRange.minimumValue <= validRange.maximumValue);
}

// Árvore de Decisão Recursiva com Backtracking.
// Combina a busca booleana (SAT) com a validação da teoria matemática (LIA).
DecisionNode* solveSMT(Formula *f, int currentVar, DecisionNode *parent, LIATheory *theory) {
    
    // Verifica o estado da formula com as atribuições atuais
    int booleanEvaluationResult = evaluateFormula(f, parent);

    if (booleanEvaluationResult == true) { 
        // Para um SMT ser SAT, uma Interpretação Parcial tem que gerar um intervalo matemáticamente consistente
        DecisionNode *leaf = createDecisionNode(currentVar, parent);
        if (leaf == NULL) return NULL;
        leaf->isSAT = evaluateMathematicalConsistency(parent, theory);
        return leaf; 
    }
    if (booleanEvaluationResult == false) { 
        DecisionNode *leaf = createDecisionNode(currentVar, parent);
        if (leaf) leaf->isSAT = false;
        return leaf;
    }

    // Ramificação (Somente se booleanEvaluationResult == Indefinida)
    DecisionNode *node = createDecisionNode(currentVar, parent);
    if (node == NULL) return NULL;

    // Ramo Verdadeiro (1)
    node->polarity = true;
    
    // Antes de descer para o próximo nível, verificamos se a atribuição atual quebrou a consistência matemática (ex: x >= 5 E x <= 2).
    // Se quebrou, não perde tempo processando o ramo esquerdo (node->left).
    if (evaluateMathematicalConsistency(node, theory)) {
        node->left = solveSMT(f, currentVar + 1, node, theory);
        if (node->left != NULL && node->left->isSAT) {
            node->isSAT = true;
            return node;
        }
    }

    // Ramo FALSO (0)
    node->polarity = false;
    
    // Repete novamente a verificação da consistencia matemática
    // Se quebrou, não perde tempo processando o ramo direito (node->right).
    if (evaluateMathematicalConsistency(node, theory)) {
        node->right = solveSMT(f, currentVar + 1, node, theory);
        if (node->right != NULL && node->right->isSAT) {
            node->isSAT = true;
            return node;
        }
    }

    // Se chegou aqui, ambos falharam matematicamente e/ou booleanamente
    // Reseta a atribuição para UNDEFINED e sinaliza o nó como UNSAT
    node->polarity = UNDEFINED;
    node->isSAT = false;
    return node;
}

// Imprime a lista de equações lineares inteiras (Teoria LIA) carregadas do arquivo
void printLIATheoryConstraints(const LIATheory *theory) {
    LIAConstraint *current = theory->constraintListHead;
    if (current == NULL) {
        printf("Nenhuma equacao matematica carregada.\n");
        return;
    }

    // Percorre a lista de equações 
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

// Gerenciador de memória
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


int main() {
    FILE *file = openFile();
    if (file == NULL) { return 1; }

    LIATheory *t = initializeLIATheory();
    
    // Chamada do leitor unificado. Ele preenche o 't' e devolve o 'f'
    Formula *f = readFile(file, t);
    fclose(file);

    if (f == NULL) { 
        freeLIATheory(t);
        return 1; 
    }
    
    DecisionNode *root = NULL;
    
    // Identifica o modo execução. (Num de equações == 0) ?  Modo SAT : Modo SMT
    if (t->totalConstraints == 0) {
        printf("[MODO SAT DETECTADO]\n");
        printf("\nFormula Booleana lida (CNF):\n");
        printBooleanFormulaCNF(f);

        // solveSAT(f, 1, NULL) — começa na variável 1, sem pai (raiz)
        root = solveSAT(f, 1, NULL);
    } else {
        printf("[MODO SMT LIA DETECTADO] - %d equacoes carregadas.\n", t->totalConstraints);
        printf("\nFormula Booleana lida (CNF mapeada):\n");
        printBooleanFormulaCNF(f);
        printf("\nRestricoes Matematicas Carregadas (LIA):\n");
        printLIATheoryConstraints(t);
        
        // solveSMT(f, 1, NULL, t) — começa na variável 1, sem pai (raiz)
        root = solveSMT(f, 1, NULL, t);
    }

    printf("\nProcessando a Arvore de Decisao...\n");
    printSolution(root, f, t);

    freeTree(root);
    freeFormula(f);
    freeLIATheory(t);

    return 0;
}