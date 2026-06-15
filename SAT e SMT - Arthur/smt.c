#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h> // importante para as interseções

//struct para a equação a*x + b (op) c
typedef struct Equation{
    int a, b, c;
    int op; // 0 para <= / 1 para >=
}Equation;

// estrutura da árvore de decisão do SAT
typedef struct SatTree {
    int level; // 0 = Raiz, 1 = E1 decidida, 2 = E1 e E2 decididas (Folha)
    bool val_E1;
    bool val_E2;
    struct SatTree *left, *right; 
} SatTree;

// LIA: FUNÇÃO DE NEGAÇÃO (inversão lógica)
Equation negateEquation(Equation eq) {
    Equation neg = eq;
    if (eq.op == 0) { 
        neg.op = 1; 
        neg.c = eq.c + 1; // <= vira > (que em LIA é >= + 1)
    } else { 
        neg.op = 0; 
        neg.c = eq.c - 1; // >= vira < (que em LIA é <= - 1)
    }
    return neg;
}

// T-SOLVER (motor matemático)
bool checkLIA(Equation e1, Equation e2, int *outMin, int *outMax) {
    int minInterv = INT_MIN;
    int maxInterv = INT_MAX;

    // avaliando E1
    int limit1 = (e1.c - e1.b) / e1.a;
    if (e1.op == 0) { 
        if (limit1 < maxInterv) maxInterv = limit1; 
    } else { 
        if (limit1 > minInterv) minInterv = limit1; 
    }

    // avaliando E2
    int limit2 = (e2.c - e2.b) / e2.a;
    if (e2.op == 0) { 
        if (limit2 < maxInterv) maxInterv = limit2; 
    } else { 
        if (limit2 > minInterv) minInterv = limit2; 
    }

    *outMin = minInterv;
    *outMax = maxInterv;

    // se o piso for menor ou igual ao teto, SAT!
    return minInterv <= maxInterv; 
}

// construção da árvore (motor SAT)
SatTree* buildTree(int level, bool e1_val, bool e2_val) {
    SatTree *node = (SatTree*) malloc(sizeof(SatTree));
    node->level = level;
    node->val_E1 = e1_val;
    node->val_E2 = e2_val;
    node->left = NULL;
    node->right = NULL;

    // se não for folha (nível 2), continua construindo os ramos de verdadeiro (esquerda) e falso (direita)
    if (level == 0) {
        node->left = buildTree(1, true, false);  // decide E1 como true
        node->right = buildTree(1, false, false); // decide E1 como false
    } else if (level == 1) {
        node->left = buildTree(2, e1_val, true);  // decide E2 como true
        node->right = buildTree(2, e1_val, false); // decide E2 como false
    }

    return node;
}

void evaluateTree(SatTree *node, Equation e1, Equation e2, bool *globalSAT, int *scenarioCount) {
    if (node == NULL) return;

    // se for nó intermediário, desce na árvore (DFS)
    if (node->level < 2) {
        evaluateTree(node->left, e1, e2, globalSAT, scenarioCount);
        evaluateTree(node->right, e1, e2, globalSAT, scenarioCount);
        return;
    }

    // chegamos numa folha (nível 2): temos um cenário lógico completo para testar
    (*scenarioCount)++;
    bool v1 = node->val_E1;
    bool v2 = node->val_E2;

    // abstração booleana (filtro lógico: E1 OR E2)
    if (!(v1 || v2)) {
        printf("[SAT] Cenario %d (E1=%s, E2=%s): Rejeitado pela Logica Booleana.\n", *scenarioCount, v1?"V":"F", v2?"V":"F");
        return; // poda a folha e volta
    }

    printf("[SAT] Cenario %d (E1=%s, E2=%s): Logica Valida! Enviando para T-Solver...\n", *scenarioCount, v1?"V":"F", v2?"V":"F");

    // prepara as equações matematicamente (inverte se for false)
    Equation test_e1 = v1 ? e1 : negateEquation(e1);
    Equation test_e2 = v2 ? e2 : negateEquation(e2);

    // validação matemática (LIA)
    int minResult, maxResult;
    bool isMathPossible = checkLIA(test_e1, test_e2, &minResult, &maxResult);

    if (isMathPossible) {
        printf("  -> [T-SOLVER] SAT! Intersecao matematica valida em: %d <= x <= %d\n\n", minResult, maxResult);
        *globalSAT = true;
    } else {
        printf("  -> [T-SOLVER] UNSAT! Conflito matematico. Dominio impossivel.\n\n");
    }
}

// leitura de arquivo
bool readEquations(char *filename, Equation *e1, Equation *e2) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Nao foi possivel abrir o arquivo '%s'.\n", filename);
        return false;
    }

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), file) && count < 2) {
        if (line[0] == 'c' && line[1] == ' ') continue; // linhas de comentário
        if (strlen(line) <= 1) continue; // linhas vazias
        char sign; char opStr[3]; int a, b, c;

        if (sscanf(line, "%dx %c %d %s %d", &a, &sign, &b, opStr, &c) == 5) {
            Equation *currentEq = (count == 0) ? e1 : e2;
            currentEq->a = a;
            currentEq->b = (sign == '-') ? -b : b;
            currentEq->c = c;
            currentEq->op = (strcmp(opStr, ">=") == 0) ? 1 : 0;
            count++;
        }
    }
    fclose(file);
    return count == 2; // true se conseguiu ler as duas, false se não
}

int main() {
    printf("Digite o nome do seu arquivo .txt com as equacoes: ");
    char filename[256];
    fgets(filename, 256, stdin);
    filename[strcspn(filename, "\n")] = '\0'; 

    Equation e1, e2;
    if (!readEquations(filename, &e1, &e2)) {
        printf("Erro: O arquivo precisa conter exatamente duas equacoes validas.\n");
        return 1;
    }
    
    printf("\nEquacoes lidas com sucesso!\n");
    printf("E1: %dx + %d %s %d\n", e1.a, e1.b, e1.op == 0 ? "<=" : ">=", e1.c);
    printf("E2: %dx + %d %s %d\n", e2.a, e2.b, e2.op == 0 ? "<=" : ">=", e2.c);

    printf(" MOTOR SAT (ARVORE DE DECISAO) INICIADO\n");
    printf(" Tentando resolver a formula: (E1 OR E2)\n");

    // Constroi a Árvore de Decisão Booleana (Nível 0, sem valores definidos ainda)
    SatTree *root = buildTree(0, false, false);

    bool globalSAT = false;
    int scenarioCount = 0;

    // Avalia a árvore usando Busca em Profundidade (DFS)
    evaluateTree(root, e1, e2, &globalSAT, &scenarioCount);

    if (globalSAT) printf("\n RESULTADO FINAL DO SMT: SATISFIABLE (SAT)\n");
    else printf("\n RESULTADO FINAL DO SMT: UNSATISFIABLE (UNSAT)\n");
    
    return 0;
}