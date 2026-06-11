#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../include/smt.h" // Automaticamente inclui sat.h



FILE *openFile();
Formula* readFile(FILE *file, LIATheory *theory);
void printSolution(DecisionNode *root, int totalVars, PartialInterp *pi, LIATheory *theory);



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
    
    PartialInterp pi = initializePartialInterp(f);
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

void printSolution(DecisionNode *root, int totalVars, PartialInterp *pi, LIATheory *theory) {
    if (root == NULL) return;

    if (!root->isSAT) {
        printf("\nResultado final: UNSAT\n");
        printf("Nenhuma combinacao torna a formula verdadeira ou ha conflito matematico.\n");
        return;
    } 
    
    printf("\nResultado final: SAT\n");

    // Checagem para o tipo de saida, é SMT ou é SAT?
    if (theory != NULL && theory->totalConstraints > 0) {
        // Invoca a função centralizada corrigida para obter o intervalo final
        Interval iv = calculateLIAInterval(totalVars, pi, theory);
        char v = (theory->constraintListHead != NULL) ? theory->constraintListHead->mathVar : 'x';

        
        printf("\nSolucoes inteiras para '%c':\n", v);

        if (iv.minimumValue == INT_MIN && iv.maximumValue == INT_MAX) { printf("'%c' pode ser qualquer numero inteiro.\n", v); }
        else if (iv.minimumValue == INT_MIN) { printf("%c <= %d\n", v, iv.maximumValue); }
        else if (iv.maximumValue == INT_MAX) { printf("%c >= %d\n", v, iv.minimumValue); }
        else { printf("%d <= %c <= %d\n", iv.minimumValue, v, iv.maximumValue); } 

    } else {
        printf("Atribuicao Logica:\n");
        for (int i = 1; i <= totalVars; i++) {
            printf("  Var %d = %d\n", i, pi->truthValue[i]);
        }
    } 
}

