#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Struct para cada nó da arvore
typedef struct HuffmanNode{
    unsigned char byte; //Caractere a ser lido
    int frequencia; //Frequencia na qual o caractere ja apareceu no arquivo
    //Ponteiros para os filhos da esquerda e da direita
    struct HuffmanNode *left;
    struct HuffmanNode *right;

}HuffmanNode;

//O nó da fila
typedef struct QNode{
    void *dado; //Dado generico
    struct QNode *next;
}QNode;

//"Controlador da fila" (Guarda a cabeca e o tamanho, apenas)
typedef struct Queue{
    QNode *Head;
    int Size;
}Queue;

//Comparador de frequencia que auxlia a funcao de inserir ordenado
//Retorna negativo se menor, 0 se igual e positivo se maior
int CompararNos(void *a, void *b){
    HuffmanNode *noA = (HuffmanNode*) a; //Casting que tranforma para variavel tipo HuffmanNode
    HuffmanNode *noB = (HuffmanNode*) b;

    return noA->frequencia - noB->frequencia;
}

//Funcao responsavel por ler o arquivo em modo binario e preencher o array de frequencia;
int contarFrequencia(const char *NomeArquivo, int *frequencias){

    //Abrimos o arquivo estritamente para leitura em binario
    FILE *arquivo = fopen(NomeArquivo, "rb");
    //Teste para ver se o arquivo foi aberto corretamente
    if(arquivo == NULL){
        printf("Erro: Nao foi possivel abrir o arquivo '%s'.\n", NomeArquivo);
        return 0; // Retorna falha
    }

    //Aux para leitura de cada byte
    unsigned char ByteLido;

    //O fread tentará ler um bloco exatamente do tamnho da variável Bytelido
    //Enquanto houver blocos de bytes a serem lidos retornará 1 mantendo o loop
    while(fread(&ByteLido, sizeof(ByteLido), 1, arquivo) == 1){
        //Incrementa o array de frequencia na exata posicao do byte lido;
        frequencias[ByteLido]++;
    }

    fclose(arquivo);
    return 1;//Leitura bem sucedida;

}

//Funcao que insere um elemento na fila mantendo a ordem, do menor pro maior
void InserirOrdenado(Queue *Fila, void *dado, int (*FuncaoDeComparacao)(void*, void*)){
    //Primeiro, criamos o novo nó da fila
    QNode *NewNode = (QNode*) malloc(sizeof(QNode));
    if(NewNode == NULL){
        printf("Erro de alocacao de memoria!\n");
        return;
    }
    NewNode->dado = dado;
    NewNode->next = NULL;

    //Se a fila estiver vazia
    if(Fila->Head == NULL){
        Fila->Head = NewNode;
        Fila->Size++;
        return;
    }

    //Se o novo no for o novo menor no da fila
    if(FuncaoDeComparacao(NewNode->dado, Fila->Head->dado) < 0){
        NewNode->next = Fila->Head;
        Fila->Head = NewNode;
        Fila->Size++;
        return;
    }

    //Se a insercao for feita no meio ou no final da fila

    //No aux para percorrer a fila
    QNode *Aux = Fila->Head;

    // O loop avança enquanto houver um próximo nó E esse próximo nó for MENOR ou IGUAL ao novo.
    while(Aux->next != NULL && FuncaoDeComparacao(Aux->next->dado, NewNode->dado) <= 0) Aux = Aux->next;

    //Agora o aux ja está no ponto onde deve ser inserido
    NewNode->next = Aux->next;
    Aux->next = NewNode;

    Fila->Size++;
}

//Funcao que cria um novo no da arvoreHuffman
HuffmanNode *CriarNo(unsigned char byte, int frequencia) {
    HuffmanNode *NewNode = (HuffmanNode*) malloc(sizeof(HuffmanNode));
    if(NewNode != NULL){
        NewNode->byte = byte;
        NewNode->frequencia = frequencia;
        NewNode->left = NULL;
        NewNode->right = NULL;
    }

    return NewNode;
}

//Remove o primeiro elemento da fila e retorna o dado (*void)
void *Pop(Queue *queue){
    if(queue->Head == NULL) return NULL;

    //Guardamos o ponteiro do no da cabeca e o dado dentro dele
    QNode *NoRemovido = queue->Head;
    void *DadoCabeca = NoRemovido->dado;

    //A cabeca da fila passa a ser o nó seguinte
    queue->Head=NoRemovido->next;
    queue->Size--;

    return DadoCabeca;
}

//Constrói a árvore consumindo a fila e retornando o nó raiz
HuffmanNode *ContruirArvore(Queue *queue, int (*FuncaoDeComparacao)(void*, void*)){
    //Loop funciona até que sobre apenas a raiz da arvore na fila
    while(queue->Size > 1){
        //Primeiro, removemos os dois nós de menor frequencia que estão no comeco da fila
        //O casting é necessario, pois a Pop devolve *void e precisamos guardar os dados
        HuffmanNode *filhoLeft = (HuffmanNode*) Pop(queue);
        HuffmanNode *filhoRight = (HuffmanNode*) Pop(queue);

        //Agora criaremos o nó pai com um caractere de controle como byte (*) e somando as frequencias dos filhos
        HuffmanNode *pai = CriarNo('*', filhoLeft->frequencia + filhoRight->frequencia);

        //Ligamos os ponteiros dos filhos ao pai
        pai->left = filhoLeft;
        pai->right = filhoRight;

        //O nó pai é inserido na fila novamente
        InserirOrdenado(queue, pai, FuncaoDeComparacao);
    }

    //Agora que o loop terminou o tamanho da fila é 1 e o unico elemento que sobrou é a raiz da arvore
    //Novamente o casting é necessário, pois a funcao retorna um nó e o pop um *void
    return (HuffmanNode*) Pop(queue);
}

//Funcao recursiva responsavel por percorrer a arvore e preencher a matriz de dicionario
void GerarDicionario(HuffmanNode *raiz, char dicionario[256][256], char *caminho, int colunas){
    //Por seguranca, verificamos se a arvore nao esta vazia
    if(raiz == NULL) return;

    //Verifica se é um no folha, se for, achamos o caractere original do arquivo
    if(raiz->left == NULL && raiz->right == NULL){
        //Encerramos a string do caminho atual
        caminho[colunas] = '\0';

        //copia a string de 0's e 1's construida no dicionario
        strcpy(dicionario[raiz->byte], caminho);
        return;
    }

    //Se nao for folha continua descendo a arvore
    
    //Se for para esquerda o bit vira '0'
    caminho[colunas] = '0';
    GerarDicionario(raiz->left, dicionario, caminho, colunas+1);
    //Se for para direita o bit vira '1'
    caminho[colunas] = '1';
    GerarDicionario(raiz->right, dicionario, caminho, colunas+1);
}

//Calcula quantos bytes a arvore ocupa no cabecalho do arquivo
int TamanhoArvore(HuffmanNode *raiz){

    //Caso base: chegou no fundo e o nó não existe
    if(raiz == NULL) return 0;

    //Verifica se o no é folha
    if(raiz->left == NULL && raiz->right == NULL){
        //Se a folha for um * ou c'\', precisara do caractere de escape '\', logo, ocupara 2 bytes
        if(raiz->byte == '*' || raiz->byte == '\\') return 2;
        //Se nao é qualquer outro caractere ocupara 1 byte
        return 1; 
    }

    //Se for no pai(nao-folha), ocupa 1 byte do '*' + o tamanho da sua subarvore
    return 1 + TamanhoArvore(raiz->left) + TamanhoArvore(raiz->right);
}

int CalculadoraLixo(int frequencias[256], char dicionario[256][256]){
    long long totalBits = 0;

    //Descobre o tamanho total dos dados comprimidos em bits
    for(int i = 0; i<256; i++){
        //Frequencia (quantas vezes aparece no arquivo) * tamanho da string (quantos bits ocupa no arquivo comprimido)
        if(frequencias[i]>0) totalBits += frequencias[i] * strlen(dicionario[i]);
    }

    //Calculamos quantos bits validos sobraram no ultimo byte
    int resto = totalBits % 8;

    //Se for 0, o ultimo byte nao possui lixo
    if(resto == 0) return 0;
    //Se sobrou algo necessario, calculamos o lixo e retornamos ele
    return 8 - resto;
}

//Imprime a arvore no arquivo gerado
void ImprimirArvorePreOrdem(HuffmanNode *raiz, FILE *arquivo){
    //Verificacao se a arvore nao esta vazia
    if(raiz == NULL) return;

    //Se for nó folha
    //Se a "letra" for um asterisco ou barra, imprime a barra de escape
    if(raiz->left == NULL && raiz->right==NULL) if(raiz->byte == '*' || raiz->byte == '\\') fputc('\\', arquivo);
    
    //Imprimimos o caractere, seja ele folha ou nó pai
    fputc(raiz->byte, arquivo);

    //Continua a recursão para os filhos da raiz
    ImprimirArvorePreOrdem(raiz->left, arquivo);
    ImprimirArvorePreOrdem(raiz->right, arquivo);
}

//Funcao que le o arquivo e grava os bits compactados no arquivo final
int CompactarDados(const char *nomeArquvioEntrada, FILE *arquivoSaida, char dicionario[256][256]){
        FILE *arquivoEntrada = fopen(nomeArquvioEntrada, "rb");
        if(arquivoEntrada == NULL){
            printf("Erro ao abrir o arquivo de entrada para compactacao!\n");
            return 0;
        }

        //Declaramos um Buffer para os bits que serao escritos
        unsigned char byteBuffer = 0;
        //Variavel que representa o endereco onde o bit deve ser escrito no buffer
        int bitIndex = 7;

        unsigned char byteLidoEntrada;

        //Lemos o arquivo byte a byte novamente
        while(fread(&byteLidoEntrada, sizeof(unsigned char), 1, arquivoEntrada) == 1){

            char *codigo = dicionario[byteLidoEntrada];

            for(int i=0; codigo[i] != '\0'; i++){
                if(codigo[i] == '1') byteBuffer = byteBuffer | (1 << bitIndex);

                bitIndex--;//Anda para o proximo bit a direita

                //Se o buffer já esta completo(8 bits), o bitIndex passou do bit 0
                if(bitIndex < 0){
                    fputc(byteBuffer, arquivoSaida); //Despeja o byte comprimido no arquivo .huff
                    byteBuffer = 0; //Esvazia o buffer
                    bitIndex = 7; //reinicia o endereco, voltando para o topo
                }
            }
        }

        //Se o buffer nao esvaziou no final, despeja o que sobrou
        if(bitIndex != 7) fputc(byteBuffer, arquivoSaida);

        fclose(arquivoEntrada);
        return 1;
    }

HuffmanNode *RemontarArvore(FILE *arquivo, int *tamanhoArvore){
    //Caso base, se o tamanho zerou, a arvore acabou
    if(*tamanhoArvore == 0) return NULL;

    //Lemos um byte do arquivo, descontando 1 do tamanho da arvore
    unsigned char byteLido = fgetc(arquivo);
    (*tamanhoArvore)--;

    //Se o byte for '\', o proximo caractere será sempre um folha
    if(byteLido == '\\'){
        byteLido = fgetc(arquivo); //Le o verdadeiro caractere('*' ou '\')
        (*tamanhoArvore)--;

        //Retornamos um nó folha, a frequencia nao importa mais, por isso, passamos 0
        return CriarNo(byteLido, 0); 
    }

    //Se for no pai ('*')
    if(byteLido == '*'){
        HuffmanNode *pai = CriarNo('*', 0);
        //A recurcao faz toda a subarvore da esquerda e depois segue para direita
        pai->left = RemontarArvore(arquivo, tamanhoArvore);
        pai->right = RemontarArvore(arquivo, tamanhoArvore);
        return pai;
    }

    //Se for qualquer outro no folha
    return CriarNo(byteLido, 0);
}

//Funcao que coordena a descompreção, comeca lendo e interpretando o cabecalho, depois remonta a arvore e, por fim, descompacta o arquivo.
void DescomprimirArquivo(const char *nomeArquivoComprimido){
    //Comecamos abrindo o arquivo
    FILE *arquivoEntrada = fopen(nomeArquivoComprimido, "rb");
    
    if(arquivoEntrada == NULL){
        printf("Erro: Nao foi possivel abrir o arquivo '%s'.\n", nomeArquivoComprimido);
        return;
    }

    //Ler os dois primeiros bytes do cabecalho(lixo e tamanho da arvore)
    unsigned char byte1 = fgetc(arquivoEntrada);
    unsigned char byte2 = fgetc(arquivoEntrada);

    //Extrair o tamanho do lixo movendo os 3 primeiros bits para direita do byte. (ex:11011001 >> 00000110)
    int tamanhoLixo = byte1 >> 5;

    //Extrair o tamanho da arvore
    //Primeiro aplica-se uma mascara 00011111 para isolar os 5 ultimos bits do primeiro byte
    //Depois junta aos outros 8 bits do segundo byte (11011001 00010011 >> 00011001 00010011)
    int tamanhoArvore = ((byte1 & 31) << 8) | byte2;

    printf("\n--- Lendo o Cabecalho do arquivo .huff ---\n");
    printf("Tamanho do Lixo identificado: %d bits\n", tamanhoLixo);
    printf("Tamanho da Arvore identificado: %d bytes\n", tamanhoArvore);

    //Agora iremos ler a arvore e remonta-la usando outra funcao
    HuffmanNode *raiz = RemontarArvore(arquivoEntrada, &tamanhoArvore);

    if(raiz != NULL) printf("Arvore remontada com sucesso!\n");
    else{
        printf("Erro ao remontar a árvore.\n");
        fclose(arquivoEntrada);
        return;
    }

    //Por ultimo, recriamos o arquivo descompactado, lemos o byte atual e tentamos ler o proximo para saber se estamos no ultimo byte ou nao
    //Comecamos preparando o nome do arquivo
    char nomeSaida[500];
    strcpy(nomeSaida, nomeArquivoComprimido);

    //Procuramos onde o .huff comeca no nome e "arranca ele fora" substituindo por \0
    char *extensao = strstr(nomeSaida, ".huff");
    if(extensao != NULL) *extensao = '\0';
    //Se o arquivo nao tiver o .huff, colocamos um prefixo apenas para nao sobreescrever o original
    else{
        char temp[500];
        strcpy(temp, "descomprimido_");
        strcat(temp, nomeSaida);
        strcpy(nomeSaida, temp);
    }

    FILE *arquivoSaida = fopen(nomeSaida, "wb");
    if (arquivoSaida == NULL){
        printf("Erro ao criar o arquivo de saida!\n");
        fclose(arquivoEntrada);
        return;
    }

    //Agora vamos ler o bits e traduzir o arquivo
    HuffmanNode *atual = raiz; //começamos a partir da raiz da arvore
    unsigned char byteLido, proximoByte;

    //Lemos o primeiro byte de dados
    if(fread(&byteLido, sizeof(unsigned char), 1, arquivoEntrada) == 1){

        while(1){
            //Tenta ler o proximo byte para saber se estamos no final ou nao
            int leuProximo = fread(&proximoByte, sizeof(unsigned char), 1, arquivoEntrada);
            
            //Vamos gerar o limite de Bits
            int limiteBits = 8; //Declara o limite como 8
            if(leuProximo == 0) limiteBits = 8-tamanhoLixo; //Se o fread nao deu certo, estamos no final, logo, o limite é 8 - o tamanho do lixo


            //O laço desce do bit mais a esquerda para o limite
            for(int i=7; i>=8-limiteBits; i--){
                //Extraimos o bit na posicao i usando a mascara
                int bit = (byteLido >> i) & 1;

                //Navegamos na arvore
                if(bit == 1) atual = atual->right;
                else atual = atual->left;

                //Checamos se o no tem filhos, se nao, eh folha, logo encontramos o caractere
                if(atual->left == NULL && atual->right == NULL){
                    fputc(atual->byte, arquivoSaida); //Escreve o caractere no arquivo descomprimido
                    atual = raiz; //Reinicia na raiz para buscar a proxima letra
                }
            }

            //Se nao conseguiu ler o proximo, ja terminamos
            if(leuProximo == 0) break;

            //O byte que analisaremos no proximo loop sera o "proximoByte"
            byteLido = proximoByte;
        }
    }

    fclose(arquivoEntrada);
    fclose(arquivoSaida);
    
    printf(">> Descompressao finalizada com SUCESSO! <<\n");
    printf(">> Arquivo original recuperado: %s\n", nomeSaida);
}




int main() {
    int opcao;
    char nomeArquivo[256];
    char nomeSaida[520]; // Maior para caber o nome original + ".huff"

    printf("==================================\n");
    printf("       COMPRESSOR DE HUFFMAN      \n");
    printf("==================================\n");
    printf("1 - Comprimir arquivo\n");
    printf("2 - Descomprimir arquivo\n");
    printf("0 - Sair\n");
    printf("==================================\n");
    printf("Escolha uma opcao: ");
    
    // Lê a opção do usuário
    if (scanf("%d", &opcao) != 1) {
        printf("Entrada invalida.\n");
        return 1;
    }

    if (opcao == 1) {
        // --- FLUXO DE COMPRESSÃO ---
        printf("\nDigite o nome do arquivo a ser comprimido (com a extensao): ");
        scanf("%s", nomeArquivo);

        // Gera o nome de saída juntando o nome original com ".huff"
        strcpy(nomeSaida, nomeArquivo);
        strcat(nomeSaida, ".huff");

        printf("\nIniciando compactacao de '%s' para '%s'...\n", nomeArquivo, nomeSaida);

        // 1. Ler as frequências
        int frequencias[256] = {0};
        if (!contarFrequencia(nomeArquivo, frequencias)) {
            return 1; 
        }

        // 2. Inicializar a fila
        Queue minhaFila;
        minhaFila.Head = NULL;
        minhaFila.Size = 0;

        // 3. Enfileirar os nós
        for (int i = 0; i < 256; i++) {
            if (frequencias[i] > 0) {
                HuffmanNode *novoNo = CriarNo(i, frequencias[i]);
                InserirOrdenado(&minhaFila, novoNo, CompararNos);
            }
        }

        // Validação de segurança para arquivos vazios
        if (minhaFila.Size == 0) {
            printf("Erro: O arquivo esta vazio. Nada a compactar.\n");
            return 0;
        }

        // 4. Construir a árvore e o dicionário
        HuffmanNode *raiz = ContruirArvore(&minhaFila, CompararNos);
        char dicionario[256][256] = {0};
        char caminho[256] = {0};
        GerarDicionario(raiz, dicionario, caminho, 0);

        // 5. Cálculos do Cabeçalho
        int tamanhoArv = TamanhoArvore(raiz);
        int tamanhoLixo = CalculadoraLixo(frequencias, dicionario);

        // 6. Criar o arquivo final e escrever os dados
        FILE *arquivoSaida = fopen(nomeSaida, "wb");
        if (arquivoSaida == NULL) {
            printf("Erro ao criar o arquivo compactado!\n");
            return 1;
        }

        // Escreve os 2 bytes do cabeçalho
        unsigned char byte1 = (tamanhoLixo << 5) | (tamanhoArv >> 8);
        unsigned char byte2 = tamanhoArv & 255;
        fputc(byte1, arquivoSaida);
        fputc(byte2, arquivoSaida);

        // Escreve a árvore em pré-ordem
        ImprimirArvorePreOrdem(raiz, arquivoSaida);

        // Compacta os dados reais
        if (CompactarDados(nomeArquivo, arquivoSaida, dicionario)) {
            printf("\n>> Compactacao finalizada com SUCESSO! <<\n");
            printf(">> Arquivo gerado: %s\n", nomeSaida);
        }

        fclose(arquivoSaida);

    } else if (opcao == 2) {
        // --- FLUXO DE DESCOMPRESSÃO (A fazer) ---
        printf("\nDigite o nome do arquivo a ser descomprimido (ex: arquivo.huff): ");
        scanf("%s", nomeArquivo);
        
        DescomprimirArquivo(nomeArquivo);
    } else if (opcao == 0) {
        printf("\nSaindo...\n");
    } else {
        printf("\nOpcao invalida!\n");
    }

    return 0;
}