#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../include/smt.h" 

FILE *openFile();
Formula* readFile(FILE *file, LIATheory *theory);
void printSolution(DecisionNode *root, Formula *f, LIATheory *theory);


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
        printBooleanFormulaCNF(f);

        // vsolveSAT(f, 1, NULL) — começa na variável 1, sem pai (raiz)
        root = solveSAT(f, 1, NULL);
    } else {
        printf("[MODO SMT LIA DETECTADO] - %d equacoes carregadas.\n", t->totalConstraints);
        printBooleanFormulaCNF(f);
        printLIATheoryConstraints(t);
        
        //solveSMT(f, 1, NULL, t) — começa na variável 1, sem pai (raiz)
        root = solveSMT(f, 1, NULL, t);
    }

    printf("\nProcessando a Arvore de Decisao...\n");
    printSolution(root, f, t);

    freeTree(root);
    freeFormula(f);
    freeLIATheory(t);

    return 0;
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

    // Se chegou até aqui, é porque falhou nas duas tentativas
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
            if      (curr->polarity == 1 && curr->left  != NULL) curr = curr->left;
            else if (curr->right != NULL)                         curr = curr->right;
            else break;
        }
    } 
}

