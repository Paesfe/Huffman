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

int main() {
    char filename[256];
    printf("Digite o nome do arquivo a ser compactado (com sua extensao). ");
    scanf("%s", filename);
    getchar();

    char output[256];
    strcpy(output, filename);
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

    printf("Arquivo compactado em %s.huff\n", output);
    fclose(outputFile);
    return 0;
}