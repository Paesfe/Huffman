#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Node {
    unsigned char byte;
    unsigned long long freq;
    struct Node *left, *right;
}Node;

typedef struct NodeQ{
    void *content;
    struct NodeQ *next;
}NodeQ;

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

//FUNÇÕES PARA A COMPRESSÃO

int compareNode(void* node1, void* node2) {
    Node *a = (Node*) node1;
    Node *b = (Node*) node2;

    if (a->freq > b->freq) return 1;
    else return 0;
}

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

void insertQueue(void *head, void *newNode) {
    NodeQ **qHead = (NodeQ**) head;
    NodeQ *node = (NodeQ*) createQueueElement(newNode);
    if (node == NULL) return;

    if (*qHead == NULL || compareNode((*qHead)->content, node->content) == 1) {
        node->next = *qHead;
        *qHead = node;
    } else {
        NodeQ *aux = *qHead;
        while (aux->next != NULL && compareNode(node->content, aux->next->content) == 1) aux = aux->next;
        node->next = aux->next;
        aux->next = node;
    }
    return;
}

void countFreq(char *filename, unsigned long long *freqArr) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Erro ao abrir arquivo \"%s\"", filename);
        return;
    }

    int currByte;
    while ((currByte = fgetc(file)) != EOF) freqArr[currByte]++;
    fclose(file);
    return;
}

void* fillQueue(unsigned long long *freqArr) {
    void *qHead = NULL;

    for (int i = 0; i < 256; i++) {
        if (freqArr[i] > 0) {
            void *node = createNode((unsigned char) i, freqArr[i]);
            insertQueue(&qHead, node);
        }
    }

    return qHead;
}

void* popQ(void* head) {
    NodeQ **qHead = (NodeQ**) head;
    if (*qHead == NULL) return NULL;

    NodeQ *aux = *qHead;
    void* content = aux->content;
    *qHead = (*qHead)->next;
    free(aux);
    return content;
}

void* buildTree(void *head) {
    NodeQ **qHead = (NodeQ**) head;
    while(*qHead != NULL && (*qHead)->next != NULL) {
        Node *left = (Node*) popQ(qHead);
        Node *right = (Node*) popQ(qHead);
        Node *parent = (Node*) createNode('*', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        insertQueue(qHead, parent);
    }
    return popQ(qHead);
}

int treeSize(void *treeRoot) {
    Node *root = (Node*) treeRoot;
    if (root == NULL) return 0;

    if (root->left == NULL && root->right == NULL) {
        if (root->byte == '*' || root->byte == '\\') return 2;
        else return 1;
    }
    return 1 + treeSize(root->left) + treeSize(root->right);
}

void codes(void *treeRoot, char *currCode, int depth, char **dictionary) {
    Node *root = (Node*) treeRoot;
    if (root == NULL) return;

    if (root->left == NULL && root->right == NULL) {
        currCode[depth] = '\0';
        dictionary[root->byte] = strdup(currCode);
        return;
    }

    currCode[depth] = '0';
    codes(root->left, currCode, depth+1, dictionary);

    currCode[depth] = '1';
    codes(root->right, currCode, depth+1, dictionary);
}

void writeTree(void *treeRoot, FILE *file) {
    Node *root = (Node*) treeRoot;
    if (root == NULL) return;

    if (root->left == NULL && root->right == NULL) {
        if (root->byte == '*' || root->byte == '\\') fputc('\\', file);
        fputc(root->byte, file);
    } else {
        fputc('*', file);
        writeTree(root->left, file);
        writeTree(root->right, file);
    }
}

int trash(unsigned long long *freqArr, char **dictionary) {
    unsigned long long totalBits = 0;
    for (int i = 0; i < 256; i++) {
        if (freqArr[i] > 0) totalBits += freqArr[i] * strlen(dictionary[i]);
    }
    int resto = totalBits % 8;
    if (resto == 0) return 0;
    else return 8 - resto;
}

void writeHeader(FILE *output, int trash, int treeSize, void *treeRoot) {
    //faça na mão usando trash = 5 e treeSize = 1209
    unsigned char byte1 = (trash << 5) | (treeSize >> 8);
    unsigned char byte2 = treeSize & 0xFF;
    fputc(byte1, output);
    fputc(byte2, output);
    writeTree(treeRoot, output);
}

void writeCompressed(char *filename, FILE *output, char **dictionary) {
    FILE *input = fopen(filename, "rb");
    if (input == NULL) {
        printf("Erro ao abrir arquivo %s\n", filename);
        return;
    }

    unsigned char buffer = 0;
    int bitCounter = 0;
    int currByte;
    while ((currByte = fgetc(input)) != EOF) {
        char *code = dictionary[currByte];
        for (int i = 0; code[i] != '\0'; i++) {
            buffer = buffer << 1;
            if (code[i] == '1') buffer = buffer | 1;
            bitCounter++;

            if (bitCounter == 8) {
                fputc(buffer, output);
                buffer = 0;
                bitCounter = 0;
            }
        }
    }
    //colocando o lixo na frente do cabeçalho
    if (bitCounter > 0) {
        buffer = buffer << (8 - bitCounter);
        fputc(buffer, output);
    }
    fclose(input);
}

//FUNÇÔES PARA A DESCOMPRESSÃO
void* rebuildTree(int treeSize, int *index, FILE *input) {
    if ((*index) >= treeSize) return NULL;
    
    int currByte = fgetc(input);
    (*index)++;

    if (currByte == '\\') {
        //fgetc retorna um int, logo não podemos criar currByte como char direto
        int currByte = fgetc(input);
        (*index)++;
        return createNode((unsigned char)currByte, 0);
    } else if (currByte == '*') {
        Node *node = (Node*) createNode('*', 0);

        //pré-ordem
        node->left = (Node*) rebuildTree(treeSize, index, input);
        node->right = (Node*) rebuildTree(treeSize, index, input);

        return (void*) node;
    } else {
        //nó folha normal (caractere aleatório)
        return createNode((unsigned char)currByte, 0);
    }
}

void writeDescompressed(FILE *input, FILE *output, void *treeRoot, int trash) {
    Node *root = (Node*) treeRoot;
    Node *currNode = root;

    int currByte = fgetc(input);
    int nextByte;

    while (currByte != EOF) {
        nextByte = fgetc(input);

        //se o próximo byte for o EOF, descontamos o lixo do byte atual, pois chegamos no ultimo
        int bits = (nextByte == EOF) ? (8 - trash) : 8;

        //lemos da esquerda para a direita até o eventual primeiro bit do lixo
        for (int i = 7; i >= 8 - bits; i--) {
            //isolando o bit atual
            int currBit = (currByte >> i) & 1;
            if (currBit) currNode = currNode->right;
            else currNode = currNode->left;

            if (currNode->left == NULL && currNode->right == NULL) {
                //se chegar a um nó folha, recuperamos o byte e escrevemos no arquivo de saída
                fputc(currNode->byte, output);
                currNode = root; //voltando à raíz para o próximo byte
            }
        }

        currByte = nextByte; //avançamos o byte atual
    }
}

int main() {

    int option = 0;
    do {
        printf("Bem vindo ao Algoritmo de Huffman.\nSelecione sua opcao:\n1 - Compactar\n2 - Descompactar\n");
        scanf("%d", &option);
        if (option == 1) {
            char filename[256];
            printf("Digite o nome do arquivo a ser compactado (com sua extensao). ");
            scanf("%s", filename);
            getchar();

            char output[256];
            strcpy(output, filename);
            char *dot = strrchr(output, '.');
            if (dot != NULL) *dot = '\0';
            strcat(output, ".huff");

            unsigned long long freqArr[256] = {0};
            countFreq(filename, freqArr);

            void *queue = fillQueue(freqArr);

            void *huffman = buildTree(&queue);

            char *dictionary[256] = {NULL};
            char currCode[256];
            codes(huffman, currCode, 0, dictionary);

            int sizeTree = treeSize(huffman);
            int trashBits = trash(freqArr, dictionary);

            FILE *outputFile = fopen(output, "wb");
            if (outputFile == NULL) {
                printf("Erro ao criar arquivo para compactacao.\n");
                return 1;
            }
            writeHeader(outputFile, trashBits, sizeTree, huffman);
            writeCompressed(filename, outputFile, dictionary);

            printf("Arquivo compactado em %s\n", output);
            fclose(outputFile);
        } else if (option == 2) {
            char filename[256];
            printf("Digite o nome do arquivo a ser descompactado (com sua extensao). ");
            scanf("%s", filename);
            getchar();

            char ext[50];
            printf("Digite a extensao para o qual deseja descompactar (ex: pdf, png, jpeg...). ");
            scanf("%s", ext);
            getchar();

            char output[256];
            strcpy(output, filename);
            char *dot = strrchr(output, '.');
            if(dot != NULL) *dot = '\0';
            strcat(output, ".");
            strcat(output, ext);

            FILE *input = fopen(filename, "rb");
            int byte1 = fgetc(input);
            int byte2 = fgetc(input);

            //descobrindo o tamanho do lixo (3 primeiros bits)
            int trashSize = byte1 >> 5;
            //montamos o tamanho da arvore isolando os 5 bits restantes do primeiro byte com 0x1F (0001 1111) e deslocamos 8 posições
            //para abrir espaço para o segundo byte, juntando com um OR
            int treeSize = ((byte1 & 0x1F) << 8) | byte2;
            int index = 0;
            void *huffmanTree = rebuildTree(treeSize, &index, input);
            FILE *outputFile = fopen(output, "wb");
            if (outputFile == NULL) {
                printf("Erro ao criar arquivo para descompactar.\n");
                fclose(input);
                return 1;
            }

            writeDescompressed(input, outputFile, huffmanTree, trashSize);
            printf("Arquivo descompactado em %s\n", output);
            fclose(input);
            fclose(outputFile);
        }
    } while (option != 1 && option != 2);
    
    return 0;
}