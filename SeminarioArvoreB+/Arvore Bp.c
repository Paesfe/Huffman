#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define ORDEM 4 // Máximo de filhos

typedef struct NoBPlus {
    int num_chaves;
    int chaves[ORDEM - 1];             // Array de chaves
    struct NoBPlus *filhos[ORDEM];     // Array de ponteiros para os filhos
    
    bool eh_folha;                     // Flag de controle
    
    struct NoBPlus *proximo_irmao;     // ponteiro para o próximo da lista
} NoBPlus;

NoBPlus* descobrir_caminho(NoBPlus *atual, int chave) {
    int i = 0;
    
    // Varre o array de chaves. 
    // Para no momento em que achar uma chave MAIOR que a nossa.
    while (i < atual->num_chaves && chave >= atual->chaves[i]) {
        i++;
    }
    
    // A mágica: o índice 'i' onde o laço parou é exatamente
    // o índice do ponteiro correto no array de filhos!
    return atual->filhos[i];
}

void realizar_split_no_filho(NoBPlus *pai, NoBPlus *filho_cheio) {
    // Cria o irmão da direita que vai receber a metade dos dados
    NoBPlus *novo_irmao = criar_no();
    novo_irmao->eh_folha = filho_cheio->eh_folha;

    int meio = ORDEM / 2;
    int chave_promovida = filho_cheio->chaves[meio];

    // Transfere a metade direita dos arrays para o novo irmão
    transferir_metade_direita(filho_cheio, novo_irmao, meio);
    filho_cheio->num_chaves = meio; // Atualiza o tamanho do nó original

    // A REGRA DA B+: Se for folha, a chave promovida é COPIADA para cima, 
    // mas também continua lá embaixo no novo_irmao. 
    // Se for nó interno, ela sobe e SAI debaixo.

    // Abre espaço no pai e insere a chave_promovida e o ponteiro para o novo_irmao
    inserir_chave_e_filho_no_pai(pai, chave_promovida, novo_irmao);

    // Refazendo a Lista Encadeada (Apenas se forem folhas!)
    if (filho_cheio->eh_folha) {
        novo_irmao->proximo_irmao = filho_cheio->proximo_irmao;
        filho_cheio->proximo_irmao = novo_irmao;
    }
}

NoBPlus* quebrar_raiz_antiga(NoBPlus *raiz_velha) {
    // 1. Cria uma nova raiz completamente vazia
    NoBPlus *nova_raiz = criar_no();
    nova_raiz->eh_folha = false;
    nova_raiz->num_chaves = 0;

    // 2. A raiz velha é "rebaixada" e vira o primeiro filho da nova raiz
    nova_raiz->filhos[0] = raiz_velha;

    // 3. Agora chamamos a função padrão de split!
    // Ela vai quebrar a raiz velha em duas e subir o elemento do meio
    // para preencher a nossa nova_raiz (que agora é o pai).
    realizar_split_no_filho(nova_raiz, raiz_velha);

    // Retorna a nova estrutura, aumentando a altura da árvore em +1
    return nova_raiz;
}

void inserir_BPlus_TopDown(NoBPlus *raiz, int chave) {
    // 1. Caso especial: A própria raiz está cheia antes de começarmos?
    // Se sim, quebramos a raiz e a árvore ganha um nível a mais de altura.
    if (raiz->num_chaves == ORDEM - 1) raiz = quebrar_raiz_antiga(raiz);

    NoBPlus *atual = raiz;

    // 2. Desce a árvore até chegar na folha
    while (!atual->eh_folha) {
        NoBPlus *proximo_filho = descobrir_caminho(atual, chave);

        // O SEGREDO TOP-DOWN: O filho está cheio? Quebra ele ANTES de descer!
        if (proximo_filho->num_chaves == ORDEM - 1) {
            realizar_split_no_filho(atual, proximo_filho);
            
            // Como o filho foi quebrado em dois, recalculamos para qual lado descer
            proximo_filho = descobrir_caminho(atual, chave);
        }
        
        atual = proximo_filho;
    }

    // 3. Chegamos na folha! Temos certeza absoluta de que há espaço.
    inserir_no_array_ordenado(atual, chave);
}