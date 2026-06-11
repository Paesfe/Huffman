#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/descompresser.h"

// Binary tree node
typedef struct Node {
    void* data;
    struct Node* left;   
    struct Node* right;  
} Node;

typedef struct {
    unsigned char byte;
} HuffmanNode;


static Node* create_HuffmanNode(unsigned char byte);
static Node* rebuild_BinaryTree(FILE *inputFile);
static int isBitSet(unsigned char byte, int i);
static void free_tree(Node *root);


void DecompressFile(const char* filePath) {
    FILE *inputFile = fopen(filePath, "rb");
    if (inputFile == NULL) {
        printf("\nErro: O arquivo '%s' nao pôde ser aberto.\n", filePath);
        printf("Descompressao abortada.\n");
        return;
    }

    int header_b1 = fgetc(inputFile);
    int header_b2 = fgetc(inputFile);
    
    if (header_b1 == EOF || header_b2 == EOF) {
        printf("\nErro: Cabeçalho corrompido ou arquivo inválido.\n");
        fclose(inputFile);
        return;
    }
    
    int trashSize = header_b1 >> 5; 
    int treeSize = ((header_b1 & 0x1F) << 8) | header_b2;

    Node *root = rebuild_BinaryTree(inputFile);
    if (root == NULL) {
        printf("\nErro: Falha ao reconstruir a arvore de Huffman.\n");
        fclose(inputFile);
        return;
    }

    char outPath[256];
    strcpy(outPath, filePath);
    
    char *ext = strstr(outPath, ".huff");
    if (ext == NULL) {
        strcat(outPath, "_descomprimido.txt"); 
    } else { 
        *ext = '\0';
        strcat(outPath, "_descomprimido.txt");
    }

    FILE *outputFile = fopen(outPath, "wb");
    if (outputFile == NULL) {
        printf("\nErro: Nao foi possivel criar o arquivo de saida.\n");
        free_tree(root);
        fclose(inputFile);
        return;
    }

    
    Node *currentNode = root;
    int currentByte = fgetc(inputFile);
    int nextByte = fgetc(inputFile);

    while (nextByte != EOF) {
        for (int i = 7; i >= 0; i--) {
            if ( isBitSet((unsigned char)currentByte, i) ) { currentNode = currentNode->right; } 
            else { currentNode = currentNode->left; }

            if (currentNode == NULL) {
                printf("\nErro: Árvore corrompida durante o processamento de bits.\n");
                fclose(inputFile); fclose(outputFile); free_tree(root);
                return;
            }

            if (currentNode->left == NULL && currentNode->right == NULL) {
                HuffmanNode *huffmandData = (HuffmanNode*)currentNode->data;
                fputc(huffmandData->byte, outputFile);
                currentNode = root; 
            }
        }
        currentByte = nextByte;
        nextByte = fgetc(inputFile);
    }

    // Ultimo Byte, processando com exclusão do lixo
    if (currentByte != EOF) {
        for (int i = 7; i >= trashSize; i--) {
            if (isBitSet((unsigned char)currentByte, i)) { currentNode = currentNode->right; } 
            else { currentNode = currentNode->left; }

            if (currentNode == NULL) {
                printf("\nErro: Árvore corrompida no tratamento final do lixo.\n");
                fclose(inputFile); fclose(outputFile); free_tree(root);
                return;
            }

            if (currentNode->left == NULL && currentNode->right == NULL) {
                HuffmanNode *huffmandData = (HuffmanNode*)currentNode->data;
                fputc(huffmandData->byte, outputFile);
                currentNode = root;
            }
        }
    }

    fclose(inputFile);
    fclose(outputFile);
    free_tree(root);

    printf("\nArquivo descomprimido com sucesso!\n");
    printf("Salvo em: %s\n", outPath);
}



// Cria um nó da árvore encapsulando o byte recuperado
static Node* create_HuffmanNode(unsigned char byte) {
    HuffmanNode* huffmanData = (HuffmanNode*) malloc(sizeof(HuffmanNode));
    if (!huffmanData) return NULL;
    
    huffmanData->byte = byte;

    Node *node = (Node*) malloc(sizeof(Node));
    if (!node) { free(huffmanData); return NULL; }
    
    node->data = huffmanData;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

// Reconstrói a árvore 
static Node* rebuild_BinaryTree(FILE *inputFile) {
    int currentChar = fgetc(inputFile);
    if (currentChar == EOF) return NULL;

    if (currentChar == '\\') {    
        currentChar = fgetc(inputFile);
        return create_HuffmanNode((unsigned char)currentChar);
    } else if (currentChar == '*') {
        Node *node = create_HuffmanNode('*');
        if (!node) return NULL;
        node->left = rebuild_BinaryTree(inputFile);
        node->right = rebuild_BinaryTree(inputFile);
        return node;
    } else {
        return create_HuffmanNode((unsigned char)currentChar);
    }
}

// Verifica se um bit específico está ativo (1) no byte
static int isBitSet(unsigned char byte, int i) {
    unsigned char mask = 1 << i;
    return (byte & mask) != 0;
}

// Libera a arvore de forma recursiva
static void free_tree(Node *root) {
    if (root == NULL) return;
    
    free_tree(root->left);
    free_tree(root->right);
    
    if (root->data) { free(root->data); }
    
    free(root);
}