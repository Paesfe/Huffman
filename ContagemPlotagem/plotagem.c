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

int Max(int a, int b) {
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

    node->height = Max(height(node->left), height(node->right)) + 1;
    nLeft->height = Max(height(nLeft->left), height(nLeft->right)) + 1; // recalculamos as alturas

    return nLeft; // nova raíz
}

AVLNode* leftRotate(AVLNode *node) {
    AVLNode *nRight = node->right; // salvamos a direita do nó atual (nova raíz)
    AVLNode *rotatedLeft = nRight->left; // salvamos a antiga esquerda da nova raíz

    nRight->left = node; // fazemos a rotação substituindo a esquerda
    node->right = rotatedLeft; // "colamos o que era a esquerda na direita da antiga raíz"

    node->height = Max(height(node->left), height(node->right)) + 1;
    nRight->height = Max(height(nRight->left), height(nRight->right)) + 1;

    return nRight; // nova raíz
}

/* ===================================================================
   INSERÇÃO COM CONTAGEM DE COMPARAÇÕES
   As funções abaixo são versões das inserções originais, mas que
   recebem um ponteiro "comparacoes" para acumular quantas comparações
   de chave foram feitas até o nó ser inserido (sem contar
   recálculo de altura/rotações da AVL, que são custos estruturais
   e não comparações de busca de posição).
   =================================================================== */

AVLNode* insertNode(AVLNode *node, int num) {
    if (node == NULL) return createAVLNode(num); // inserção normal

    if (num < node->num) node->left = insertNode(node->left, num); // inserindo na esquerda, caso condicional atendido, por recursão
    else if (num > node->num) node->right = insertNode(node->right, num); // inserindo na direita, caso condicional atendido, por recursão
    else return node; // se números forem iguais, retorna, pois não permite em árvores binárias/AVL

    node->height = Max(height(node->left), height(node->right)) + 1; // atualiza a altura

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

//  insercao na BST contando comparacoes ate encontrar a posicao de insercao
BSTNode* insertBSTCount(BSTNode *tree, int num, int *comparacoes) {
    BSTNode *node = createNode(num);
    if (tree == NULL) return node;
    else {
        BSTNode *aux = tree;
        while (1) {
            (*comparacoes)++; // comparacao de igualdade
            if (num == aux->num) {
                free(node);
                break;
            }
            (*comparacoes)++; // comparacao de menor/maior
            if (num < aux->num) {
                if (aux->left == NULL) {
                    aux->left = node;
                    break;
                } else aux = aux->left;
            } else {
                if (aux->right == NULL) {
                    aux->right = node;
                    break;
                } else aux = aux->right;
            }
        }
    }
    return tree;
}

//  insercao na AVL contando comparacoes ate encontrar a posicao de insercao
// (as rotacoes/balanceamento continuam acontecendo normalmente, mas nao
// somam ao contador, pois nao sao "comparacoes de busca")
AVLNode* insertAVLCount(AVLNode *node, int num, int *comparacoes) {
    if (node == NULL) return createAVLNode(num);

    (*comparacoes)++; // comparacao de igualdade
    if (num == node->num) return node;

    (*comparacoes)++; // comparacao de menor/maior
    if (num < node->num) node->left = insertAVLCount(node->left, num, comparacoes);
    else node->right = insertAVLCount(node->right, num, comparacoes);

    node->height = Max(height(node->left), height(node->right)) + 1;

    int balanceFactor = balance(node);

    if (balanceFactor > 1 && num < node->left->num) return rightRotate(node);
    if (balanceFactor < -1 && num > node->right->num) return leftRotate(node);
    if (balanceFactor > 1 && num > node->left->num) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balanceFactor < -1 && num < node->right->num) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
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

/* ===================================================================
   REMOÇÃO COM CONTAGEM DE COMPARAÇÕES
   =================================================================== */

//  encontra o menor valor de uma subarvore BST (usado na remocao
// de nos com dois filhos), contando as comparacoes percorridas
BSTNode* minValueNodeBST(BSTNode* node, int *comparacoes) {
    BSTNode* current = node;
    while (current->left != NULL) {
        (*comparacoes)++; // comparacao para decidir se desce a esquerda
        current = current->left;
    }
    return current;
}

//  remocao na BST contando comparacoes (igualdade + menor/maior)
// ate localizar o no a ser removido, igual a logica de busca
BSTNode* removeBST(BSTNode* root, int num, int *comparacoes) {
    if (root == NULL) return root;

    (*comparacoes)++; // comparacao de igualdade
    if (num == root->num) {
        // caso 1: no com 0 ou 1 filho
        if (root->left == NULL) {
            BSTNode *temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            BSTNode *temp = root->left;
            free(root);
            return temp;
        }
        // caso 2: no com 2 filhos -> pega o sucessor (menor da direita)
        BSTNode* temp = minValueNodeBST(root->right, comparacoes);
        root->num = temp->num;
        root->right = removeBST(root->right, temp->num, comparacoes);
        return root;
    }

    (*comparacoes)++; // comparacao de menor/maior
    if (num < root->num) root->left = removeBST(root->left, num, comparacoes);
    else root->right = removeBST(root->right, num, comparacoes);

    return root;
}

//  encontra o menor valor de uma subarvore AVL, contando comparacoes
AVLNode* minValueNodeAVL(AVLNode* node, int *comparacoes) {
    AVLNode* current = node;
    while (current->left != NULL) {
        (*comparacoes)++; // comparacao para decidir se desce a esquerda
        current = current->left;
    }
    return current;
}

// remocao na AVL contando comparacoes de busca (igualdade + menor/maior)
// mantem o rebalanceamento normal apos a remocao (sem somar ao contador,
// pois rotacoes nao sao comparacoes de busca)
AVLNode* removeAVL(AVLNode* root, int num, int *comparacoes) {
    if (root == NULL) return root;

    (*comparacoes)++; // comparacao de igualdade
    if (num == root->num) {
        if (root->left == NULL || root->right == NULL) {
            AVLNode *temp = root->left ? root->left : root->right;
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp;
            }
            free(temp);
        } else {
            AVLNode* temp = minValueNodeAVL(root->right, comparacoes);
            root->num = temp->num;
            root->right = removeAVL(root->right, temp->num, comparacoes);
        }
    } else {
        (*comparacoes)++; // comparacao de menor/maior
        if (num < root->num) root->left = removeAVL(root->left, num, comparacoes);
        else root->right = removeAVL(root->right, num, comparacoes);
    }

    if (root == NULL) return root;

    // rebalanceamento padrao da AVL apos remocao
    root->height = Max(height(root->left), height(root->right)) + 1;
    int balanceFactor = balance(root);

    if (balanceFactor > 1 && balance(root->left) >= 0) return rightRotate(root);
    if (balanceFactor > 1 && balance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balanceFactor < -1 && balance(root->right) <= 0) return leftRotate(root);
    if (balanceFactor < -1 && balance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

int main() {
    // configura a semente para os sorteios aleatórios
    srand(time(NULL));

    int n = 32766;         // total de elementos a inserir
    int sorteios = 200;    // quantas buscas/remocoes faremos
    int numExiste[32767] = {0};

    BSTNode *BSTroot = NULL;
    AVLNode *AVLroot = NULL;

    /* ===============================================================
        arquivo CSV para registrar a contagem de comparacoes
       feitas durante a INSERCAO de cada um dos n elementos
       =============================================================== */
    FILE *fileInsercao = fopen("dados_insercao.csv", "w");
    if (fileInsercao == NULL) {
        printf("Erro ao criar arquivo CSV de insercao!\n");
        return 1;
    }
    fprintf(fileInsercao, "Insercao,NumeroInserido,Comp_BST,Comp_AVL\n");

    printf("Inserindo %d dados nas arvores...\n", n);
    for (int i = 1; i <= n; i++) {
        int numeroInserido = (rand() % n) + 1;
        while (numExiste[numeroInserido] == 1) numeroInserido = (rand() % n) + 1;

        //  contadores zerados a cada insercao
        int compsInsBST = 0, compsInsAVL = 0;

        BSTroot = insertBSTCount(BSTroot, numeroInserido, &compsInsBST);
        AVLroot = insertAVLCount(AVLroot, numeroInserido, &compsInsAVL);

        //  grava no csv de insercao
        fprintf(fileInsercao, "%d,%d,%d,%d\n", i, numeroInserido, compsInsBST, compsInsAVL);

        numExiste[numeroInserido] = 1;
    }
    fclose(fileInsercao);

    // criando o arquivo para o matlab (busca)
    FILE *file = fopen("dados_sorteio.csv", "w");
    if (file == NULL) {
        printf("Erro ao criar arquivo CSV!\n");
        return 1;
    }
    
    // cabeçalho do csv
    fprintf(file, "Sorteio,NumeroSorteado,Comp_BST,Comp_AVL\n");

    printf("Realizando buscas e salvando no arquivo...\n");
    for (int i = 1; i <= sorteios; i++) {
        int numero_sorteado = (rand() % n) + 1;
        
        int compsBST = searchBST(BSTroot, numero_sorteado);
        int compsAVL = searchAVL(AVLroot, numero_sorteado);

        fprintf(file, "%d,%d,%d,%d\n", i, numero_sorteado, compsBST, compsAVL);
    }

    fclose(file);

    /* ===============================================================
       secao de REMOCAO
       Sorteamos "sorteios" numeros que ainda existem nas arvores e
       removemos cada um, contando as comparacoes feitas em cada
       arvore durante a remocao. Como o numero precisa existir para
       ser removido das duas arvores da mesma forma, sorteamos entre
       os numeros marcados como presentes em numExiste.
       =============================================================== */
    FILE *fileRemocao = fopen("dados_remocao.csv", "w");
    if (fileRemocao == NULL) {
        printf("Erro ao criar arquivo CSV de remocao!\n");
        return 1;
    }
    fprintf(fileRemocao, "Remocao,NumeroRemovido,Comp_BST,Comp_AVL\n");

    printf("Realizando remocoes e salvando no arquivo...\n");
    for (int i = 1; i <= sorteios; i++) {
        //sorteia um numero que ainda esta presente nas arvores
        int numero_removido = (rand() % n) + 1;
        while (numExiste[numero_removido] == 0) numero_removido = (rand() % n) + 1;

        int compsRemBST = 0, compsRemAVL = 0;

        BSTroot = removeBST(BSTroot, numero_removido, &compsRemBST);
        AVLroot = removeAVL(AVLroot, numero_removido, &compsRemAVL);

        fprintf(fileRemocao, "%d,%d,%d,%d\n", i, numero_removido, compsRemBST, compsRemAVL);

        // marca como removido para nao ser sorteado de novo
        numExiste[numero_removido] = 0;
    }
    fclose(fileRemocao);

    printf("Concluido! Arquivos 'dados_insercao.csv', 'dados_sorteio.csv' e 'dados_remocao.csv' prontos para o MATLAB.\n");

    return 0;
}
