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

bool readEquations(Formula *f, LIATheory* t, FILE *file) {
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

        // Rejeita coeficiente zero (causaria divisão por zero em calculateLIAInterval)
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

// Arredonda para cima 
int floorDiv(int dividend, int divisor) {
    return dividend / divisor - (((dividend ^ divisor) < 0) && (dividend % divisor != 0));
}

// Arredonda para baixo
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


Interval calculateLIAInterval(int totalVars, PartialInterp *pi, LIATheory *theory) {
    Interval validRange = { INT_MIN, INT_MAX };

    LIAConstraint *currentConstraint = theory->constraintListHead;
    while (currentConstraint != NULL) {
        short val = pi->truthValue[currentConstraint->atomID];
        if (val != UNDEFINED) {
            int adjustedConstant = currentConstraint->constantValue;
            char op[3];
            strcpy(op, currentConstraint->operatorSymbol);

            if (currentConstraint->innerSign == '+') { adjustedConstant -= currentConstraint->innerOffset; } 
            else if (currentConstraint->innerSign == '-') { adjustedConstant += currentConstraint->innerOffset; }

            int a = currentConstraint->coefficient;
            

            //Tratamento caso a clausula booleana seja negativa 
            if (val == 0) { negateOperator(op); } 

            // Se 'a' for negativo, multiplicamos a inequação por -1, invertendo a inequação. ( '<' vira '>')
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
        currentConstraint = currentConstraint->next;
    }
    return validRange;
}

bool evaluateMathematicalConsistency(Formula *f, PartialInterp *pi, LIATheory *theory) {
    Interval validRange = calculateLIAInterval(f->atomCount, pi, theory);
    return (validRange.minimumValue <= validRange.maximumValue);
}

DecisionNode* solveSMT(Formula *f, PartialInterp *pi, int currentVar, LIATheory *theory) {
    DecisionNode *node = createDecisionNode(currentVar);
    if (node == NULL) return NULL;
    
    int booleanEvaluationResult = evaluateFormula(f, pi);

    if (booleanEvaluationResult == true) { 
        // Fórmula booleana satisfeita: verifica consistência matemática
        node->isSAT = evaluateMathematicalConsistency(f, pi, theory);
        return node; 
    }
 
    if (booleanEvaluationResult == false) { 
        node->isSAT = false;
        return node;
    }

    // Backtracking
    // Ramo Verdadeiro (1)
    pi->truthValue[currentVar] = true; 
    node->value = true;
    
    // Só desce na recursão se o estado atual for matematicamente consistente
    if (evaluateMathematicalConsistency(f, pi, theory)) {
        node->left = solveSMT(f, pi, currentVar + 1, theory);
        if (node->left != NULL && node->left->isSAT) {
            node->isSAT = true;
            return node;
        }
    }

    // Ramo FALSO (0)
    pi->truthValue[currentVar] = 0; 
    node->value = 0;
    
    // Só desce na recursão se o estado atual for matematicamente consistente
    if (evaluateMathematicalConsistency(f, pi, theory)) {
        node->right = solveSMT(f, pi, currentVar + 1, theory);
        if (node->right != NULL && node->right->isSAT) {
            node->isSAT = true;
            return node;
        }
    }

    // Se chegou aqui, ambos falharam: desfaz a atribuição e sinaliza UNSAT
    pi->truthValue[currentVar] = UNDEFINED;
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