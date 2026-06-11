#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/compresser.h"

// Binary tree node
typedef struct Node {
    void* data;
    struct Node* left;   
    struct Node* right;  
} Node;

// Priority Queue Node
typedef struct queueNode {
    void *data;                     
    struct queueNode *next;  
} queueNode;

typedef struct {
    unsigned char byte;
    long frequency;     
} HuffmanNode;

static int compare_HuffmanNode(void *a, void *b);
static Node* create_HuffmanNode(unsigned char byte, long frequency);
static Node* generateBinaryTree(queueNode *queue, int (*compareFunction)(void*, void*));
static void enqueue(queueNode **head, void *treeNode, int (*compare)(void*, void*));
static void* dequeue(queueNode **head);
static void printTree_preOrder(Node *root, FILE *out, int *tree_size);


void CompressFile(const char *filePath) {
    FILE *inputFile = fopen(filePath, "rb");
    if (inputFile == NULL) {
        printf("\nErro: O arquivo '%s' nao pôde ser aberto.\n", filePath);
        printf("Compressao abortada.\n");
        return;
    }

    long frequencies[256] = {0};
    int current_char;
    while ((current_char = fgetc(inputFile)) != EOF) {
        frequencies[(unsigned char)current_char]++;
    }

    queueNode *queue_head = NULL;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            Node *leaf = create_HuffmanNode((unsigned char)i, frequencies[i]);
            enqueue(&queue_head, leaf, compare_HuffmanNode);
        }
    }

    if (queue_head == NULL) {
        printf("\nAviso: O arquivo fornecido esta vazio.\n");
        fclose(inputFile);
        return;
    }

    if (queue_head->next == NULL) {
        Node *singleNode = (Node*)queue_head->data;
        HuffmanNode *singleData = (HuffmanNode*)singleNode->data;
        unsigned char dummy_byte = (singleData->byte == 0) ? 1 : 0;
        Node *dummy_leaf = create_HuffmanNode(dummy_byte, 0);
        enqueue(&queue_head, dummy_leaf, compare_HuffmanNode);
    }

    Node *root = generateBinaryTree(queue_head, compare_HuffmanNode);

    char dictionary[256][257] = {{0}};
    char current_path[257] = {0};
    generateDictionary(root, dictionary, current_path, 0);

    char outPath[256];
    sprintf(outPath, "%s.huff", filePath);
    FILE *outputFile = fopen(outPath, "wb");
    if (outputFile == NULL) {
        printf("\nErro: Nao foi possivel criar o arquivo compactado.\n");
        free_tree(root);
        fclose(inputFile);
        return;
    }

    fputc(0, outputFile); 
    fputc(0, outputFile);

    int tree_size = 0;
    printTree_preOrder(root, outputFile, &tree_size);

    rewind(inputFile); 
    
    unsigned char buffer_byte = 0;
    int bitCount = 0;

    while ((current_char = fgetc(inputFile)) != EOF) {
        char *code = dictionary[(unsigned char)current_char];
        for (int i = 0; code[i] != '\0'; i++) {
            if (code[i] == '1') {
                buffer_byte = buffer_byte | (1 << (7 - bitCount));
            }
            bitCount++;

            if (bitCount == 8) {
                fputc(buffer_byte, outputFile);
                buffer_byte = 0;
                bitCount = 0;
            }
        }
    }

    int trash_size = 0;
    if (bitCount > 0) {
        trash_size = 8 - bitCount; 
        fputc(buffer_byte, outputFile);
    }

    fseek(outputFile, 0, SEEK_SET); 

    unsigned char header_b1 = (trash_size << 5) | ((tree_size >> 8) & 0x1F);
    unsigned char header_b2 = tree_size & 0xFF;

    fputc(header_b1, outputFile);
    fputc(header_b2, outputFile);

    fclose(inputFile);
    fclose(outputFile);
    free_tree(root);

    printf("\nArquivo comprimido com sucesso!\n");
    printf("Salvo em: %s\n", outPath);
}


// Cria um nó da árvore encapsulando os dados de huffman
static Node* create_HuffmanNode(unsigned char byte, long frequency) {
    HuffmanNode* HuffmanData = (HuffmanNode*) malloc(sizeof(HuffmanNode));
    if (!HuffmanData) return NULL;
   
    HuffmanData->byte = byte;
    HuffmanData->frequency = frequency;

    Node *node = (Node*) malloc(sizeof(Node));
    if (!node) { free(HuffmanData); return NULL; }
    
    node->data = HuffmanData;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

// Ordenar a Fila
static int compare_HuffmanNode(void *a, void *b) {
    Node *na = (Node*)a;
    Node *nb = (Node*)b;
    HuffmanNode *da = (HuffmanNode*)na->data;
    HuffmanNode *db = (HuffmanNode*)nb->data;
    
    if (da->frequency < db->frequency) return -1;
    if (da->frequency > db->frequency) return 1;
    return 0;
}

// Insere na fila de forma ordenada
static void enqueue(queueNode **head, void *treeNode, int (*compare)(void*, void*)) {
    queueNode *new_node = (queueNode*) malloc(sizeof(queueNode));
    if (!new_node) return;
    new_node->data = treeNode;
    new_node->next = NULL;

    if (*head == NULL || compare(treeNode, (*head)->data) < 0) {
        new_node->next = *head;
        *head = new_node;
    } else {
        queueNode *current = *head;
        while (current->next != NULL && compare(treeNode, current->next->data) >= 0) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}

// Remove e retorna o elemento de maior prioridade/menor frequência fila
static void* dequeue(queueNode **head) {
    if (*head == NULL) return NULL;
    
    queueNode *temp = *head;
    void *treeNode = temp->data;
    *head = (*head)->next;
    free(temp);
    
    return treeNode;
}

// Função recursiva para varrer a árvore e montar a tabela de strings (Dicionário de tamanho 257)
static void generateDictionary(Node *root, char dict[256][257], char *current_path, int depth) {
    if (root == NULL) return;

    if (root->left == NULL && root->right == NULL) {
        current_path[depth] = '\0';
        HuffmanNode *hd = (HuffmanNode*)root->data;
        strcpy(dict[hd->byte], current_path);
        return;
    }

    current_path[depth] = '0';
    generateDictionary(root->left, dict, current_path, depth + 1);

    current_path[depth] = '1';
    generateDictionary(root->right, dict, current_path, depth + 1);
}

// Grava a árvore mapeada em Pré-Ordem no arquivo e conta o seu tamanho total
static void printTree_preOrder(Node *root, FILE *out, int *tree_size) {
    if (root == NULL) return;

    HuffmanNode *hd = (HuffmanNode*)root->data;
    
    if (root->left == NULL && root->right == NULL) {
        if (hd->byte == '*' || hd->byte == '\\') {
            fputc('\\', out);
            (*tree_size)++;
        }
        fputc(hd->byte, out);
        (*tree_size)++;
    } else {
        fputc('*', out);
        (*tree_size)++;
        printTree_preOrder(root->left, out, tree_size);
        printTree_preOrder(root->right, out, tree_size);
    }
}

static Node* generateBinaryTree(queueNode* queue, int (*compareFunction)(void*, void*)){
    while (queue != NULL && queue->next != NULL) {
        Node *lowestFreq = (Node*) dequeue(&queue);
        Node *secLowestFreq = (Node*) dequeue(&queue);

        HuffmanNode *left = (HuffmanNode*)lowestFreq->data;
        HuffmanNode *right = (HuffmanNode*)secLowestFreq->data;

        Node *parent = create_HuffmanNode('*', left->frequency + right->frequency);
        parent->left = lowestFreq;
        parent->right = secLowestFreq;

        enqueue(&queue, parent, compareFunction);
    }
    return (Node*) dequeue(&queue);
}

// Libera a arvore de forma recursiva
static void free_tree(Node *root) {
    if (root == NULL) return;
    
    free_tree(root->left);
    free_tree(root->right);
    
    if (root->data) { free(root->data); }
    
    free(root);
}