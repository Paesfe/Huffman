# Compactador de Arquivos com Codificação de Huffman

Implementação do algoritmo de **Huffman** para compactação e descompactação de arquivos binários genéricos (qualquer extensão), usando árvore de prefixos e uma fila de prioridade construída sobre lista encadeada.

## Como funciona

1. **Contagem de frequência**: o arquivo é lido byte a byte e a frequência de cada um dos 256 valores possíveis é contabilizada.
2. **Fila de prioridade**: cada byte com frequência > 0 se torna um nó-folha, inserido em ordem crescente de frequência em uma fila encadeada.
3. **Construção da árvore**: os dois nós de menor frequência são removidos repetidamente e combinados sob um nó pai (`*`), até restar apenas a raiz — algoritmo clássico de Huffman.
4. **Dicionário de códigos**: a árvore é percorrida (0 = esquerda, 1 = direita) gerando o código binário de cada byte.
5. **Compactação**: o arquivo original é escrito bit a bit conforme o dicionário, precedido por um cabeçalho com:
   - bits de "lixo" (padding do último byte) e tamanho da árvore serializada, compactados em 2 bytes;
   - a árvore em pré-ordem (raiz, esquerda, direita), com caractere de escape `\` para bytes que colidem com o marcador de nó interno (`*`) ou com o próprio escape.
6. **Descompactação**: o cabeçalho é lido, a árvore é reconstruída recursivamente, e os bits do arquivo `.huff` são decodificados navegando a árvore até cada folha.

## Estrutura de arquivos

```
.
└── HUFFMAN.c
```

## Compilando e executando

```bash
gcc HUFFMAN.c -o huffman
./huffman
```

O programa exibe um menu interativo:

```
Bem vindo ao Algoritmo de Huffman.
Selecione sua opcao:
1 - Compactar
2 - Descompactar
```

### Compactar
- Informe o nome do arquivo (com extensão) a ser compactado.
- O resultado é salvo com a mesma base de nome e extensão `.huff`.

### Descompactar
- Informe o nome do arquivo `.huff`.
- Informe a extensão original desejada para o arquivo restaurado (ex: `pdf`, `png`, `txt`).
- O resultado é salvo com a extensão informada.

## Observações técnicas

- Funciona com qualquer tipo de arquivo (texto, imagem, PDF, etc.), já que a leitura é feita em modo binário (`rb`/`wb`).
- O cabeçalho usa apenas 2 bytes: 3 bits para o lixo (padding, no máximo 7 bits) e 13 bits para o tamanho da árvore serializada.
- Empates de frequência são resolvidos pela ordem de inserção na fila (estabilidade da fila encadeada).
