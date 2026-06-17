#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Estrutura do nó da Árvore de Huffman
typedef struct Node {
    unsigned char byte;         //Carcatere original do arquivo 
    unsigned long long freq;    //frequencia do caractere no arquivo
    struct Node *left, *right;  //Ponteiros para os filhos da esquerda (0) e direita (1)
}Node;

// Estrutura do Nó da Fila de Prioridade
typedef struct NodeQ{
    void *content;              //Ponteiro genérico (vai apontar para um Node da árvore)
    struct NodeQ *next;         //Ponteiro para o próximo elemento da fila
}NodeQ;

// Cria um nó para *árvore*
void* createNode(unsigned char byte, unsigned long long freq) {
    Node *node = (Node*) malloc(sizeof(Node));
    if (node == NULL) {
        printf("Falha na alocacao.\n");
        return NULL;
    }
    node->byte = byte;
    node->freq = freq;
    node->left = NULL;
    node->right = NULL;

    return (void*)node;
}

// Função que compara a frequência de dois nós, retorna 1, se a for maior que b, e 0, caso contrário
int compareNode(void* node1, void* node2) {
    Node *a = (Node*) node1;
    Node *b = (Node*) node2;

    if (a->freq > b->freq) return 1;
    else return 0;
}

// Cria o nó para *fila de prioridade*
void* createQueueElement(void *content) {
    NodeQ *node = (NodeQ*) malloc(sizeof(NodeQ));
    if (node == NULL) {
        printf("Falha na alocacao.\n");
        return NULL;
    }
    node->content = content;
    node->next = NULL;
    return (void*) node;
}

// Insere um novo nó na Fila mantendo a ordem crescente de frequência
// Recebe um ponteiro duplo (**qHead) para poder alterar a verdadeira cabeça da fila
void insertQueue(void *head, void *newNode) {
    NodeQ **qHead = (NodeQ**) head;
    NodeQ *node = (NodeQ*) createQueueElement(newNode);
    if (node == NULL) return;

    // Caso 1: A fila está vazia OU o novo nó é menor que a atual cabeça da fila
    if (*qHead == NULL || compareNode((*qHead)->content, node->content) == 1) {
        node->next = *qHead;
        *qHead = node;
    } else {
        // Caso 2: O nó deve ser inserido no meio ou no fim da fila
        NodeQ *aux = *qHead;
        //Anda pela fila enquanto houver próximo e a frequencia do novo for maior que a do próximo nó
        while (aux->next != NULL && compareNode(node->content, aux->next->content) == 1) aux = aux->next;
        node->next = aux->next;
        aux->next = node;
    }
    return;
}

// Lê o arquivo original e conta a frequência de cada byte
void countFreq(char *filename, unsigned long long *freqArr) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Erro ao abrir arquivo \"%s\"", filename);
        return;
    }

    int currByte;
    // O valor numérico do byte atua como o próprio índice no array freqArr
    while ((currByte = fgetc(file)) != EOF) freqArr[currByte]++;
    fclose(file);
    return;
}

// Transforma o array de frequências em uma Fila de Prioridade
void* fillQueue(unsigned long long *freqArr) {
    void *qHead = NULL;

    for (int i = 0; i < 256; i++) {
        if (freqArr[i] > 0) {
            // Cria um nó de árvore isolado para cada byte que existe no texto e joga na fila
            void *node = createNode((unsigned char) i, freqArr[i]);
            insertQueue(&qHead, node);
        }
    }

    return qHead;
}

// Remove e retorna o primeiro elemento da fila (o que tem a menor frequência)
void* popQ(void* head) {
    NodeQ **qHead = (NodeQ**) head;
    if (*qHead == NULL) return NULL;

    NodeQ *aux = *qHead;
    void* content = aux->content; //Guarda a árvore/nó
    *qHead = (*qHead)->next;      //Avança a cabeça da fila  
    free(aux);
    return content;
}

// Monta a Árvore de Huffman consumindo a Fila de Prioridade
void* buildTree(void *head) {
    NodeQ **qHead = (NodeQ**) head;
    // Roda até sobrar apenas a raiz da árvore na fila
    while(*qHead != NULL && (*qHead)->next != NULL) {
        Node *left = (Node*) popQ(qHead);   //Retira o menor
        Node *right = (Node*) popQ(qHead);  //Retira o 2º menor

        // Cria um nó pai (*) somando as frequências dos dois filhos
        Node *parent = (Node*) createNode('*', left->freq + right->freq);
        parent->left = left;
        parent->right = right;

        //Devolve o pai para fila de for ordenada
        insertQueue(qHead, parent);
    }
    return popQ(qHead);     //Retorna a raiz da árvore montada
}

// Calcula quantos bytes a árvore ocupará no cabeçalho do arquivo
int treeSize(void *treeRoot) {
    Node *root = (Node*) treeRoot;
    if (root == NULL) return 0;

    // Se chegou numa folha
    if (root->left == NULL && root->right == NULL) {
        // Se a folha for um '*' ou '\' real do texto, precisamos colocar um '\' de escape (gasta 2 bytes)
        if (root->byte == '*' || root->byte == '\\') return 2;
        else return 1;
    }
    // Tamanho do pai + tamanho do lado esquerdo + tamanho do lado direito
    return 1 + treeSize(root->left) + treeSize(root->right);
}

// Navega pela árvore e cria o dicionário de caminhos (0s e 1s) para cada byte
void codes(void *treeRoot, char *currCode, int depth, char **dictionary) {
    Node *root = (Node*) treeRoot;
    if (root == NULL) return;

    // Achou uma folha: Salva o caminho gerado no dicionário
    if (root->left == NULL && root->right == NULL) {
        currCode[depth] = '\0'; // Finaliza a string
        dictionary[root->byte] = strdup(currCode); // Clona a string pro dicionário
        return;
    }

    // Navega para a esquerda adicionando '0'
    currCode[depth] = '0';
    codes(root->left, currCode, depth+1, dictionary);

    // Navega para a direita adicionando '1'
    currCode[depth] = '1';
    codes(root->right, currCode, depth+1, dictionary);
}

// Grava a árvore no arquivo usando Pré-Ordem (Raiz, Esquerda, Direita)
void writeTree(void *treeRoot, FILE *file) {
    Node *root = (Node*) treeRoot;
    if (root == NULL) return;

    if (root->left == NULL && root->right == NULL) {
        // Aplica o escape caso a letra seja igual a um nó interno (*) ou ao próprio escape (\)
        if (root->byte == '*' || root->byte == '\\') fputc('\\', file);
        fputc(root->byte, file);
    } else {
        fputc('*', file); // Marca nó interno
        writeTree(root->left, file);
        writeTree(root->right, file);
    }
}

// Calcula quantos bits ficarão vazios (Lixo) no último byte do arquivo
int trash(unsigned long long *freqArr, char **dictionary) {
    unsigned long long totalBits = 0;
    for (int i = 0; i < 256; i++) {
        // Soma os bits totais que cada caractere vai consumir após ser compactado
        if (freqArr[i] > 0) totalBits += freqArr[i] * strlen(dictionary[i]);
    }
    int resto = totalBits % 8;
    if (resto == 0) return 0;  // Preencheu completamente o ultimo byte
    else return 8 - resto;     // O que falta pra fechar o byte é o lixo
}

// Escreve os 2 bytes do cabeçalho (Lixo + Tamanho da Árvore)
void writeHeader(FILE *output, int trash, int treeSize, void *treeRoot) {
    // byte1: junta os 3 bits do lixo deslocados pro topo (<< 5) com os 5 maiores bits da arvore (>> 8)
    unsigned char byte1 = (trash << 5) | (treeSize >> 8);
    // byte2: corta apenas os 8 menores bits do tamanho da árvore com a mascara (11111111)
    unsigned char byte2 = treeSize & 0xFF;
    
    fputc(byte1, output);
    fputc(byte2, output);
    writeTree(treeRoot, output); // Grava a estrutura da árvore em seguida
}

// Realiza a compactação: Lendo o arquivo original e gravando o novo com os bits do dicionário
void writeCompressed(char *filename, FILE *output, char **dictionary) {
    FILE *input = fopen(filename, "rb");
    if (input == NULL) {
        printf("Erro ao abrir arquivo %s\n", filename);
        return;
    }

    unsigned char buffer = 0; // acumula bits até fechar 1 byte
    int bitCounter = 0;
    int currByte;
    
    while ((currByte = fgetc(input)) != EOF) {
        char *code = dictionary[currByte];
        for (int i = 0; code[i] != '\0'; i++) {
            buffer = buffer << 1; // Empurra tudo pra esquerda para abrir espaço pro novo bit
            if (code[i] == '1') buffer = buffer | 1; // "Liga" o bit se for '1'
            bitCounter++;

            // Se acumulou 8 bits escreve no arquivo e zera pra próxima rodada
            if (bitCounter == 8) {
                fputc(buffer, output);
                buffer = 0;
                bitCounter = 0;
            }
        }
    }
    // Despeja o resto dos bits que não fecharam 8. 
    // Empurra pro começo (esquerda) formando o lixo no final (direita), caso houver
    if (bitCounter > 0) {
        buffer = buffer << (8 - bitCounter);
        fputc(buffer, output);
    }
    fclose(input);
}

// Reconstrói a árvore de Huffman lendo a Pré-ordem do arquivo compactado
void* rebuildTree(int treeSize, int *index, FILE *input) {
    // Previne que leia além do tamanho da árvore
    if ((*index) >= treeSize) return NULL;
    
    int currByte = fgetc(input);
    (*index)++;

    if (currByte == '\\') {
        // Encontrou um escape. O próximo byte é um caractere real!
        int currByte = fgetc(input);
        (*index)++;
        return createNode((unsigned char)currByte, 0);
    } else if (currByte == '*') {
        // É um nó interno. Chama a recursão pra criar a perna esquerda e depois a direita
        Node *node = (Node*) createNode('*', 0);
        node->left = (Node*) rebuildTree(treeSize, index, input);
        node->right = (Node*) rebuildTree(treeSize, index, input);
        return (void*) node;
    } else {
        // Nó folha normal (caractere real)
        return createNode((unsigned char)currByte, 0);
    }
}

// Decodifica os bits lendo da esquerda para a direita e andando na árvore
void writeDescompressed(FILE *input, FILE *output, void *treeRoot, int trash) {
    Node *root = (Node*) treeRoot;
    Node *currNode = root;

    int currByte = fgetc(input);
    int nextByte;

    while (currByte != EOF) {
        // A "leitura antecipada" prevê se estamos chegando no fim do arquivo
        nextByte = fgetc(input);

        // Se o próximo byte for EOF, significa que o atual é o último. Logo, descontamos o lixo dele!
        int bits = (nextByte == EOF) ? (8 - trash) : 8;

        // Extrai cada bit varrendo o byte do bit 7 ao 0
        for (int i = 7; i >= 8 - bits; i--) {
            // Empurra o bit que queremos até o final e isola ele com a máscara '& 1'
            int currBit = (currByte >> i) & 1;
            
            // Navega na árvore: 1 para direita, 0 para esquerda
            if (currBit) currNode = currNode->right;
            else currNode = currNode->left;

            // Bateu em uma folha? Achou a letra!
            if (currNode->left == NULL && currNode->right == NULL) {
                fputc(currNode->byte, output);
                currNode = root; // Reseta a busca pra raiz da árvore para decodificar o prox. bit
            }
        }

        currByte = nextByte; // Avança no arquivo para o próximo byte
    }
}

int main() {
    int option = 0;
    do {
        printf("Bem vindo ao Algoritmo de Huffman.\nSelecione sua opcao:\n1 - Compactar\n2 - Descompactar\n");
        scanf("%d", &option);
        
        if (option == 1) { // COMPACTAÇÃO
            char filename[256];
            printf("Digite o nome do arquivo a ser compactado (com sua extensao). ");
            scanf("%s", filename);
            getchar(); // Limpa o buffer do enter

            FILE *checkFile = fopen(filename, "rb");
            if (checkFile == NULL) {
                printf("\nErro: O arquivo '%s' nao foi encontrado!\n", filename);
                return 1; // Interrompe o programa para evitar Segmentantion Fault
            }
            fclose(checkFile);

            // Manipulação de strings: Tira a extensão velha e coloca .huff
            char output[256];
            strcpy(output, filename);
            char *dot = strrchr(output, '.');
            if (dot != NULL) *dot = '\0';
            strcat(output, ".huff");

            // Passo 1: Conta as frequências
            unsigned long long freqArr[256] = {0};
            countFreq(filename, freqArr);

            // Passo 2: Joga tudo ordenado numa Fila
            void *queue = fillQueue(freqArr);

            // Passo 3: Constrói a Árvore
            void *huffman = buildTree(&queue);

            // Passo 4: Gera o Dicionário de caminhos (0s e 1s)
            char *dictionary[256] = {NULL};
            char currCode[256];
            codes(huffman, currCode, 0, dictionary);

            // Passo 5: Calcula os valores do Cabeçalho
            int sizeTree = treeSize(huffman);
            int trashBits = trash(freqArr, dictionary);

            FILE *outputFile = fopen(output, "wb");
            if (outputFile == NULL) {
                printf("Erro ao criar arquivo para compactacao.\n");
                return 1;
            }
            
            // Passo 6: Grava no arquivo!
            writeHeader(outputFile, trashBits, sizeTree, huffman);
            writeCompressed(filename, outputFile, dictionary);

            printf("Arquivo compactado em %s\n", output);
            fclose(outputFile);

        } else if (option == 2) { // DESCOMPACTAÇÃO
            char filename[256];
            printf("Digite o nome do arquivo a ser descompactado (com sua extensao). ");
            scanf("%s", filename);
            getchar();

            char ext[50];
            printf("Digite a extensao para o qual deseja descompactar (ex: pdf, png, txt...). ");
            scanf("%s", ext);
            getchar();

            // Montagem do nome do arquivo restaurado
            char output[256];
            strcpy(output, filename);
            char *dot = strrchr(output, '.');
            if(dot != NULL) *dot = '\0';
            strcat(output, ".");
            strcat(output, ext);

            FILE *input = fopen(filename, "rb");

            if (input == NULL) {
                printf("\nErro: O arquivo '%s' nao foi encontrado!\n", filename);
                return 1; // Interrompe o programa para evitar que o fgetc quebre tudo na próxima linha
            }
            
            // Passo 1: Lê o cabeçalho (Lixo e Tamanho da Árvore)
            int byte1 = fgetc(input);
            int byte2 = fgetc(input);

            // Isola o lixo tirando os 5 últimos bits da frente (>> 5)
            int trashSize = byte1 >> 5;
            
            // Isola a árvore do byte 1 cortando o lixo com a máscara (00011111) e abre espaço (<< 8) para o byte 2
            int treeSize = ((byte1 & 0x1F) << 8) | byte2;
            
            int index = 0;
            
            // Passo 2: Remonta a árvore original
            void *huffmanTree = rebuildTree(treeSize, &index, input);
            
            FILE *outputFile = fopen(output, "wb");
            if (outputFile == NULL) {
                printf("Erro ao criar arquivo para descompactar.\n");
                fclose(input);
                return 1;
            }

            // Passo 3: Traduz e escreve o novo arquivo decodificado
            writeDescompressed(input, outputFile, huffmanTree, trashSize);
            printf("Arquivo descompactado em %s\n", output);
            
            fclose(input);
            fclose(outputFile);
        }
    } while (option != 1 && option != 2);
    
    return 0;
}
