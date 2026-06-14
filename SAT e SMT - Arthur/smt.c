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

// estrutura da árvore de deisões
typedef struct Tree{
    int leaf; // 1 se for folha, 0 se nó intermediário
    int value; // apenas se for folha
    int minDomain; // limite inferior do domínio neste nó
    int maxDomain; // limite superior do domínio neste nó
    struct Tree *left, *right; 
}Tree;

Tree* createNode(int minDomain, int maxDomain) {
    Tree *node = (Tree*) malloc(sizeof(Tree));
    node->minDomain = minDomain; node->maxDomain = maxDomain;
    node->left = NULL; node->right = NULL;

    if (minDomain == maxDomain) { // se o domínio converge para o mesmo número, é um nó folha
        node->leaf = 1;
        node->value = minDomain;
    } else {
        node->leaf = 0;
        node->value = -1; // indefinido para nós não-folha
    }
    return node;
}

void intersect(Equation e1, Equation e2, int *minInterv, int *maxInterv) {
     *minInterv = INT_MIN;
     *maxInterv = INT_MAX;

     printf("Normalizacao:\n");
     int limit1 = (e1.c - e1.b) / e1.a;
     if (e1.op == 0) { // se for <= mudamos o teto
        if (limit1 < *maxInterv) *maxInterv = limit1;
        printf("%dx + %d <= %d \t x <= %d\n", e1.a, e1.b, e1.c, limit1);
     } else { // se for >= mudamos o piso
        if (limit1 > *minInterv) *minInterv = limit1;
        printf("%dx + %d >= %d \t x >= %d\n", e1.a, e1.b, e1.c, limit1);
     }

     int limit2 = (e2.c - e2.b) / e2.a; // fazendo os cálculos para a segunda equação
     if (e2.op == 0) {
        if (limit2 < *maxInterv) *maxInterv = limit2;
        printf("%dx + %d <= %d \t x <= %d\n", e2.a, e2.b, e2.c, limit2);
     } else {
        if (limit2 > *minInterv) *minInterv = limit2;
        printf("%dx + %d >= %d \t x >= %d\n", e2.a, e2.b, e2.c, limit2);
     }

     printf("\nIntersecao:\n");
     if (*minInterv > *maxInterv) printf("Dominio impossivel! x >= %d e x <= %d\n\n", *minInterv, *maxInterv);
     else {
        printf("%d <= x <= %d \t x in {", *minInterv, *maxInterv);
        for (int i = *minInterv; i <= *maxInterv; i++) {
            printf("%d%s", i, (i == *maxInterv) ? "}\n\n" : ",");
        }
     }
}

Tree* buildTree(int minInterv, int maxInterv) {
    if (minInterv > maxInterv) return NULL; // domínio inválido (UNSAT na raíz)

    Tree *node = createNode(minInterv, maxInterv);

    // se não for folha, divide o domínio em dois e cria as ramificações
    if (!node->leaf) {
        int mid = minInterv + (maxInterv - minInterv) / 2;
        node->left = buildTree(minInterv, mid);
        node->right = buildTree(mid + 1, maxInterv);
    }

    return node;
}

// percorre a árvore (DFS) e avalia apenas as folhas
void evaluateTree(Tree *node, Equation e1, Equation e2, int *checkNum, int solutions[], int *solCount, bool *overallSat) {
    if (node == NULL) return;

    // se for nó interno, continua descendo na árvore
    if (!node->leaf) {
        evaluateTree(node->left, e1, e2, checkNum, solutions, solCount, overallSat);
        evaluateTree(node->right, e1, e2, checkNum, solutions, solCount, overallSat);
        return;
    }

    // agora temos um nó folha
    printf("Checando soluçao %d:\n", *checkNum);
    (*checkNum)++;
    printf("x = %d\n", node->value);
    
    int x = node->value;
    bool e1Sat = false, e2Sat = false;
    
    int res1 = e1.a * x + e1.b;
    if (e1.op == 0) { // <=
        e1Sat = (res1 <= e1.c); // atribuição automática pela comparação
        printf("%d*(%d) + %d <= %d \t %d <= %d \t %s\n", e1.a, x, e1.b, e1.c, res1, e1.c, e1Sat ? "True!" : "False!");
    } else { // >=
        e1Sat = (res1 >= e1.c); 
        printf("%d*(%d) + %d >= %d \t %d >= %d \t %s\n", e1.a, x, e1.b, e1.c, res1, e1.c, e1Sat ? "True!" : "False!");
    }
    
    int res2 = e2.a * x + e2.b;
    if (e2.op == 0) { // <=
        e2Sat = (res2 <= e2.c);
        printf("%d*(%d) + %d <= %d \t %d <= %d \t %s\n", e2.a, x, e2.b, e2.c, res2, e2.c, e2Sat ? "True!" : "False!");
    } else { // >=
        e2Sat = (res2 >= e2.c);
        printf("%d*(%d) + %d >= %d \t %d >= %d \t %s\n", e2.a, x, e2.b, e2.c, res2, e2.c, e2Sat ? "True!" : "False!");
    }
    
    printf("\n");
    
    if (e1Sat && e2Sat) {
        *overallSat = true;
        solutions[(*solCount)] = x;
        (*solCount)++;
    }
}

bool readEquations(char *filename, Equation *e1, Equation *e2) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Nao foi possivel abrir o arquivo '%s'.\n", filename);
        return false;
    }

    char line[256];
    int count = 0; // vai contar 0 (para e1) e 1 (para e2)

    // lê as linhas até o fim do arquivo ou até ter lido 2 equações
    while (fgets(line, sizeof(line), file) && count < 2) {
        if (strlen(line) <= 1) continue; // ignora linhas vazias

        char sign;
        char opStr[3];
        int a, b, c;

        // tenta extrair o padrão: 2x + 7 <= 8
        if (sscanf(line, "%dx %c %d %s %d", &a, &sign, &b, opStr, &c) == 5) {
            
            // se count for 0, aponta para e1, mas se for 1, aponta para e2
            Equation *currentEq = (count == 0) ? e1 : e2;

            currentEq->a = a;
            currentEq->b = (sign == '-') ? -b : b; // ajusta o sinal
            currentEq->c = c;
            
            // se a string for ">=", op = 1. senão, assumimos que é "<=" (op = 0).
            currentEq->op = (strcmp(opStr, ">=") == 0) ? 1 : 0;
            
            count++;
        }
    }

    fclose(file);
    return count == 2; // retorna true apenas se leu exatamente 2 equações válidas
}

int main() {
    printf("Digite o nome do seu arquivo .txt com as equacoes: ");
    char filename[256];
    fgets(filename, 256, stdin);
    filename[strcspn(filename, "\n")] = '\0'; 

    Equation e1, e2;
    
    // Passa o endereço de e1 e e2 para a função preencher
    if (!readEquations(filename, &e1, &e2)) {
        printf("Erro: O arquivo precisa conter exatamente duas equacoes validas.\n");
        return 1;
    }
    
    printf("\nEquacoes lidas com sucesso!\n\n");
    
    int minVal = 0, maxVal = 0;
    intersect(e1, e2, &minVal, &maxVal);

    // constrói a árvore binária
    Tree *tree = buildTree(minVal, maxVal);

    // prepara variáveis para a travessia
    int checkNum = 1;
    int solutions[100];
    int solCount = 0;
    bool overallSat = false;
    
    // avalia a árvore
    evaluateTree(tree, e1, e2, &checkNum, solutions, &solCount, &overallSat);
    
    // 4. Resultado Final
    if (overallSat) {
        printf("SAT! Com soluçoes: ");
        for (int i = 0; i < solCount; i++) {
            printf("x = %d", solutions[i]);
            if (i < solCount - 1) printf(" ou ");
        }
        printf("\n");
    } else {
        printf("UNSAT!\n");
    }
    
    return 0;
}