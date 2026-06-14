#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // necessário para as funções randômicas

typedef struct BSTNode {
    int num;
    struct BSTNode *left, *right;
} BSTNode;

typedef struct AVLNode {
    int num;
    struct AVLNode *left, *right;
    int height;
} AVLNode;

AVLNode* createAVLNode(int num) {
    AVLNode *node = (AVLNode*) malloc(sizeof(AVLNode));
    node->num = num;
    node->height = 1; // um novo nó é inicialmente adicionado numa folha
    node->left = NULL;
    node->right = NULL;
    return node;
}

BSTNode* createNode(int num) {
    BSTNode *node = (BSTNode*) malloc(sizeof(BSTNode));
    node->num = num;
    node->left = NULL;
    node->right = NULL;
    return node;
}

int height(AVLNode *node) {
    if (node == NULL) return 0;
    else return node->height;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int balance(AVLNode *node) {
    if (node == NULL) return 0;
    else return height(node->left) - height(node->right); // calculando o fator de balanceamento
}

AVLNode* rightRotate(AVLNode *node) {
    AVLNode *nLeft = node->left; // salvamos o nó à esquerda do que queremos rotacionar (nova raíz)
    AVLNode *rotatedRight = nLeft->right; // salvamos a direita do mesmo

    nLeft->right = node; // fazemos a rotação substituindo a direita
    node->left = rotatedRight; // "colamos" o que era a direita na esquerda da antiga raíz

    node->height = max(height(node->left), height(node->right)) + 1;
    nLeft->height = max(height(nLeft->left), height(nLeft->right)) + 1; // recalculamos as alturas

    return nLeft; // nova raíz
}

AVLNode* leftRotate(AVLNode *node) {
    AVLNode *nRight = node->right; // salvamos a direita do nó atual (nova raíz)
    AVLNode *rotatedLeft = nRight->left; // salvamos a antiga esquerda da nova raíz

    nRight->left = node; // fazemos a rotação substituindo a esquerda
    node->right = rotatedLeft; // "colamos o que era a esquerda na direita da antiga raíz"

    node->height = max(height(node->left), height(node->right)) + 1;
    nRight->height = max(height(nRight->left), height(nRight->right)) + 1;

    return nRight; // nova raíz
}


AVLNode* insertNode(AVLNode *node, int num) {
    if (node == NULL) return createAVLNode(num); // inserção normal

    if (num < node->num) node->left = insertNode(node->left, num); // inserindo na esquerda, caso condicional atendido, por recursão
    else if (num > node->num) node->right = insertNode(node->right, num); // inserindo na direita, caso condicional atendido, por recursão
    else return node; // se números forem iguais, retorna, pois não permite em árvores binárias/AVL

    node->height = max(height(node->left), height(node->right)) + 1; // atualiza a altura

    int balanceFactor = balance(node); // checa desbalanceamento

    if (balanceFactor > 1 && num < node->left->num) return rightRotate(node); // caso left-left
    if (balanceFactor < -1 && num > node->right->num) return leftRotate(node); // caso right-right
    if (balanceFactor > 1 && num > node->left->num) { // caso left-right
        node->left = leftRotate(node->left);
        return rightRotate(node); // fazemos uma rotação à esquerda e depois à direita
    }
    if (balanceFactor < -1 && num < node->right->num) { // caso right-left
        node->right = rightRotate(node->right);
        return leftRotate(node); // fazemos uma rotação à direita e depois à esquerda
    }

    return node; // ao fim de tudo, retornamos o nó atual
}


BSTNode* insertBST(BSTNode *tree, int num) {
    BSTNode *node = createNode(num); // alocamos o nó
    if (tree == NULL) return node; // se a árvore estiver vazia, ele é a raíz
    else {
        BSTNode *aux = tree; // ponteiro para iteração
        while (1) {
            if (num < aux->num) {
                if (aux->left == NULL) {
                    aux->left = node; // atribuição ao achar espaço vazio
                    break;
                } else aux = aux->left;
            } else if (num > aux->num) {
                if (aux->right == NULL) {
                    aux->right = node;
                    break;
                } else aux = aux->right;
            } else {
                free(node); // liberamos o nó, pois não inserimos o mesmo número duas vezes em uma árvore de busca binária
                break;
            }
        }
    }
    return tree;
}

int searchBST(BSTNode* root, int num) {
    int comparacoes = 0;
    BSTNode* current = root;
    while (current != NULL) {
        comparacoes++; // comparação de igualdade
        if (num == current->num) return comparacoes;
        
        comparacoes++; // comparação de menor/maior
        if (num < current->num) current = current->left;
        else current = current->right;
    }
    return comparacoes;
}

int searchAVL(AVLNode* root, int num) {
    int comparacoes = 0;
    AVLNode* current = root;
    while (current != NULL) {
        comparacoes++; // comparação de igualdade
        if (num == current->num) return comparacoes;
        
        comparacoes++; // comparação de menor/maior
        if (num < current->num) current = current->left;
        else current = current->right;
    }
    return comparacoes;
}

int main() {
    // configura a semente para os sorteios aleatórios
    srand(time(NULL));
    
    int n = 10000;         // total de elementos a inserir
    int sorteios = 50; // quantas buscas faremos
    int numExiste[10001] = {0};

    BSTNode *BSTroot = NULL;
    AVLNode *AVLroot = NULL;

    printf("Inserindo %d dados ordenados nas arvores...\n", n);
    for (int i = 1; i <= n; i++) {
        int numeroInserido = (rand() % n) + 1;
        while (numExiste[numeroInserido] == 1) numeroInserido = (rand() % n) + 1;
        BSTroot = insertBST(BSTroot, numeroInserido);
        AVLroot = insertNode(AVLroot, numeroInserido);
        numExiste[numeroInserido] = 1;
    }

    // criando o arquivo para o matlab
    FILE *file = fopen("dados_sorteio.csv", "w");
    if (file == NULL) {
        printf("Erro ao criar arquivo CSV!\n");
        return 1;
    }
    
    // cabeçalho do csv
    fprintf(file, "Sorteio,NumeroSorteado,Comp_BST,Comp_AVL\n");

    printf("Realizando sorteios e salvando no arquivo...\n");
    for (int i = 1; i <= sorteios; i++) {
        int numero_sorteado = (rand() % n) + 1;
        
        int compsBST = searchBST(BSTroot, numero_sorteado);
        int compsAVL = searchAVL(AVLroot, numero_sorteado);

        fprintf(file, "%d,%d,%d,%d\n", i, numero_sorteado, compsBST, compsAVL);
    }

    fclose(file);
    printf("Concluido! Arquivo 'dados_sorteio.csv' pronto para o MATLAB.\n");

    return 0;
}