#include "../include/smt.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

// Faz a leitura da equações informadas no input
// Ler equações tanto na forma 'a*x >= k' quanto 'a*x +- b <= k' 
bool readEquations(Formula *f, LIATheory* t, FILE *file) {
    for (int i = 0; i < t->totalConstraints; i++) {
        int atomID, coefficient, constantValue;
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
        // Verifica a alcação do malloc
        if (newConstraint == NULL) {
            fprintf(stderr, "ERRO: Falha ao alocar LIAConstraint.\n");
            return false;
        }
        
        // Preenche os equação com os dados lidos
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

// Divisão de números inteiros, arredondando para baixo 
int floorDiv(int dividend, int divisor) {
    return dividend / divisor - (((dividend ^ divisor) < 0) && (dividend % divisor != 0));
}

// Divisão de números inteiros, arredondando para cima
int ceilDiv(int dividend, int divisor) {
    return dividend / divisor + (((dividend ^ divisor) > 0) && (dividend % divisor != 0));
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
    
    //Percorre toda a lista de equações matemáticas 
    while (currentConstraint != NULL) {
        // Acessa o valor Booleano da Variavel/ID associado a uma equação
        short val = getVarValue(leaf, currentConstraint->atomID);

        // Só processa a equação caso a varivel tenha um valor booleano definnido (0 ou 1)
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
                int limit = floorDiv(adjustedConstant, a);
                if (limit < validRange.maximumValue) validRange.maximumValue = limit;
            } 
            // Caso 2: ax < b
            else if (strcmp(op, "<") == 0) {
                int limit = floorDiv(adjustedConstant - 1, a);
                if (limit < validRange.maximumValue) validRange.maximumValue = limit;
            } 
            // Caso 3: ax >= b
            else if (strcmp(op, ">=") == 0) {
                int limit = ceilDiv(adjustedConstant, a);
                if (limit > validRange.minimumValue) validRange.minimumValue = limit;
            } 
            // Caso 4: ax > b
            else if (strcmp(op, ">") == 0) {
                int limit = ceilDiv(adjustedConstant + 1, a);
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
    
   
    //Antes de descer para o próximo nível, verificamos se a atribuição atual quebrou a consistência matemática (ex: x >= 5 E x <= 2).
    //Se quebrou, não perde tempo processando o ramo esquerdo (node->left).
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
    //Se quebrou, não perde tempo processando o ramo direito (node->right).
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
    printf("\nRestricoes Matematicas Carregadas (LIA):\n");
    
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