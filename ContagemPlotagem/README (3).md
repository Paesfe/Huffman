# Comparação de Desempenho: BST x AVL

Estudo experimental comparando o número de comparações realizadas por uma **Árvore Binária de Busca (BST)** simples e uma **Árvore AVL** balanceada, nas operações de **inserção**, **busca** e **remoção**. Os dados são gerados em C e visualizados através de gráficos em MATLAB.

## Como funciona

1. **`plotagem.c`** insere `32766` números (de 1 a 32766, sorteados sem repetição) em uma BST e em uma AVL ao mesmo tempo, contando quantas comparações de chave cada estrutura realiza para posicionar cada novo elemento.
2. Em seguida, sorteia `200` buscas e `200` remoções, contando novamente o número de comparações feitas por cada árvore em cada operação.
3. Os resultados são exportados para três arquivos `.csv`, prontos para serem plotados no MATLAB.

**Observação sobre a contagem:** apenas comparações de *busca de posição* (igual / menor / maior) são contadas. Recálculo de altura e rotações de balanceamento da AVL são tratados como custo estrutural e não entram no contador — isso isola o efeito do balanceamento sobre o **custo de busca**, que é o que realmente queremos comparar.

## Estrutura de arquivos

```
.
├── plotagem.c           # Gera as árvores, conta comparações e exporta os CSVs
├── plotar.m             # Script MATLAB que lê os CSVs e plota os gráficos
├── dados_insercao.csv   # Exemplo de saída: comparações por inserção
├── dados_sorteio.csv    # Exemplo de saída: comparações por busca
├── dados_remocao.csv    # Exemplo de saída: comparações por remoção
└── explicacao.png       # Explicação dos gráficos gerados (teste1.png)
```

## Compilando e executando o gerador de dados

```bash
gcc plotagem.c -o plotagem
./plotagem
```

Isso sobrescreve `dados_insercao.csv`, `dados_sorteio.csv` e `dados_remocao.csv` com novos dados a cada execução (a semente aleatória é baseada no horário do sistema).

## Gerando os gráficos no MATLAB Online

Como o programa não exige instalação local do MATLAB, os gráficos podem ser gerados gratuitamente via navegador:

1. Acesse o site do **MATLAB Online** e crie uma conta gratuita (sem necessidade de licença/chave de ativação).
2. Arraste os arquivos `dados_insercao.csv`, `dados_sorteio.csv`, `dados_remocao.csv` e `plotar.m` para a área **Files**, à esquerda da tela.
3. Dê dois cliques em `plotar.m` para abri-lo e clique no botão verde **Run**.

## Gráficos gerados

1. **Comparações durante a inserção** — evolução do custo ao longo das 32766 inserções.
2. **Comparações durante a busca** — custo de 200 buscas aleatórias.
3. **Comparações durante a remoção** — custo de 200 remoções aleatórias.
4. **Médias gerais** — gráfico de barras resumindo a média de comparações de BST x AVL nas três operações.

## Interpretação dos resultados (ver `explicacao.png`)

- **BST (linha vermelha):** alta volatilidade. Como não há balanceamento, o custo de busca depende inteiramente da ordem de inserção — pode ficar perto da raiz (rápido) ou cair em um ramo longo e desequilibrado (lento).
- **AVL (linha azul):** muito mais estável, pois o algoritmo força o balanceamento a cada inserção, garantindo altura sempre logarítmica (`O(log n)`) e eliminando os picos de custo da BST.
- **Cruzamentos pontuais:** em alguns sorteios específicos a BST pode ser mais rápida que a AVL "por sorte" (encontrar o elemento perto da raiz). Isso não invalida a conclusão geral: na média de longo prazo, a AVL é consistentemente mais eficiente.
