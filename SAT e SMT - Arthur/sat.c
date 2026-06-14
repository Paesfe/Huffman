#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// nó para um literal dentro de uma cláusula
typedef struct Literal{
    int id, negated; // id e se é negativo
    struct Literal *next;
}Literal;

// nó para uma cláusula contendo vários literais
typedef struct Clause{
    Literal *literals; // ponteiro para os literais
    struct Clause *next; // ponteiro para próxima cláusula
}Clause;

// estrutura para a fórmula geral
typedef struct Formula{
    int nVars, nClauses; // número de literais e cláusulas
    Clause *clauses; // ponteiro para a próxima cláusula
}Formula;

// estrutura da árvore de deisões
typedef struct Tree{
    int varID, value; // id da cláusula e seu valor (1 se verdadeiro, 0 se falso e -1 se indefinido)
    struct Tree *left, *right, *parent; // adicionamos o pai para não precisar de outra struct para guardar os valores das literais, fazendo por recursão
}Tree;

Formula* read(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Não foi possível abrir o arquivo.\n");
        return NULL;
    }

    Formula *formula = (Formula*) malloc(sizeof(Formula));
    formula->clauses = NULL;

    char type;
    while (fscanf(file, " %c", &type) == 1) {
        if (type == 'c') {
            char buffer[1000];
            fgets(buffer, 1000, file);
        } else if (type == 'p') {
            char format[10]; // criamos para ler "cnf" entre "p" e o número de variáveis e cláusulas
            fscanf(file, "%s %d %d", format, &formula->nVars, &formula->nClauses);
            break;
        }
    }

    Clause *last = NULL;
    Clause *curr = NULL;
    int lit;

    while (fscanf(file, "%d", &lit) == 1) {
        if (curr == NULL) { // iniciando uma nova cláusula (current vazia)
            curr = (Clause*) malloc(sizeof(Clause));
            curr->literals = NULL;
            curr->next = NULL;

            // conectando a lista de cláusulas
            if (formula->clauses == NULL) formula->clauses = curr;
            else last->next = curr;
            last = curr;
        }

        if (lit == 0) curr = NULL; // fechamento de cláusula
        else {
            Literal *newLit = (Literal*) malloc(sizeof(Literal));
            newLit->id = abs(lit); // pegamos o valor absoluto (marcador da variável)
            newLit->negated = (lit < 0) ? 1 : 0; // marca se é negativo

            newLit->next = last->literals;
            last->literals = newLit; // inserção no início pela facilidade (ordem não importa)
        }
    }

    fclose(file);
    return formula;
}

void printFormula(Formula *formula) {
    if (formula == NULL) return;
    printf("Formula com %d variaveis e %d clausulas:\n", formula->nVars, formula->nClauses);

    Clause *clause = formula->clauses;
    int counter = 1;
    while (clause != NULL) {
        printf("Clausula %d: ( ", counter);
        counter++;
        Literal *lit = clause->literals;
        while (lit != NULL) {
            if (lit->negated == 1) printf("~");
            printf("x%d ", lit->id);
            if (lit->next != NULL) printf("V "); // OR
            lit = lit->next;
        }
        printf(")\n");
        clause = clause->next;
    }
}

int getVarValue(Tree *node, int id) {
    while (node != NULL) { // retornamos o valor de certa variável na cláusula (1 se verdadeiro, 0 se falso e -1 se indefinido)
        if (node->varID == id) return node->value;
        else node = node->parent; // retornando mais um passo até achar a literal correta
    }
    return 0;
}

bool formulaSat(Formula *formula, Tree *leaf) {
    Clause *clause = formula->clauses;
    while (clause != NULL) {
        Literal *literals = clause->literals;
        bool clauseTrue = false;
        while (literals != NULL) {
            int value = getVarValue(leaf, literals->id);
            int litValue = literals->negated ? !value : value; // corrigindo o valor caso seja negativo

            if (litValue == 1) {
                clauseTrue = true; // precisamos apenas de uma literal verdadeira para a cláusula ser verdadeira
                break;
            }
            literals = literals->next;
        }

        if (!clauseTrue) return false; // se apenas uma cláusula for falsa, podemos retornar
        clause = clause->next;
    }
    return true;
}

Tree* solveSat(Formula *formula, int currVar, Tree *parent, bool *sat) {
    // caso base: já analisamos todas as variáveis, então checamos a fórmula
    if (currVar > formula->nVars) {
        if (formulaSat(formula, parent)) *sat = true;
        return NULL;
    }

    Tree *node = (Tree*) malloc(sizeof(Tree));
    node->varID = currVar;
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;

    node->value = 1; // tentamos a esquerda, assumindo true
    node->left = solveSat(formula, currVar + 1, node, sat);
    if (*sat) return node; // achamos a solução, podemos retornar

    node->value = 0;
    node->right = solveSat(formula, currVar + 1, node, sat);
    if (*sat) return node;

    return node;
}

int main() {
    printf("Bem vindo ao SAT Solver. Digite o nome do seu arquivo .cnf para análise: ");
    char filename[256];
    fgets(filename, 256, stdin);
    filename[strcspn(filename, "\n")] = '\0';
    Formula *formula = read(filename);
    printFormula(formula); // no print, possivelmente teremos a ordem de literais invertidas para cada cláusula pela inserção no início
                           // como o OR é comutativo, não nos afeta
    if (formula == NULL) return 1;

    printf("Resolvendo com arvore de decisao...\n");
    bool sat = false;

    Tree *tree = solveSat(formula, 1, NULL, &sat); // iniciamos na variável 1, com pai NULL

    if (sat) {
        printf("\n>>> RESULTADO: SAT! <<<\n");
        printf("Solucao encontrada:\n");
        
        Tree *curr = tree;
        while (curr != NULL) {
            printf("Variavel x%d = %s\n", curr->varID, (curr->value == 1) ? "TRUE" : "FALSE");
            if (curr->value == 1) curr = curr->left;
            else curr = curr->right;
        }
    } else {
        printf("\n>>> RESULTADO: UNSAT! <<<\n");
    }

    return 0;
}