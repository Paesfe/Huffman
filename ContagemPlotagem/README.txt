# Comparação de Desempenho: BST x AVL

Este projeto realiza um estudo experimental comparando o desempenho de uma **Árvore Binária de Busca (BST)** simples com uma **Árvore AVL** balanceada.

A análise considera o número de comparações realizadas em três operações fundamentais:

- **Inserção**
- **Busca**
- **Remoção**

Os dados são gerados em linguagem C e posteriormente visualizados através de gráficos no **MATLAB Online**.

---

## Objetivo

O objetivo do experimento é analisar como o balanceamento influencia a eficiência das operações em árvores binárias.

Enquanto a BST depende diretamente da ordem de inserção dos elementos, a AVL mantém sua altura balanceada, garantindo melhor desempenho médio.

---

## Funcionamento do Programa

O programa executa as seguintes etapas:

### 1. Inserção de elementos

O arquivo `comparacoes.c` insere **32766 números inteiros**, de `1` a `32766`, sorteados aleatoriamente sem repetição.

Cada número é inserido simultaneamente em:

- uma BST
- uma AVL

Durante esse processo, o programa contabiliza quantas comparações de chave foram necessárias para posicionar cada novo elemento.

---

### 2. Busca aleatória

Após a construção das árvores, o programa realiza **200 buscas aleatórias**, registrando o número de comparações realizadas em cada estrutura.

---

### 3. Remoção aleatória

Em seguida, são realizadas **200 remoções aleatórias**, novamente contabilizando o número de comparações em ambas as árvores.

---

### 4. Exportação dos dados

Ao final, o programa gera três arquivos `.csv`:

- `dados_insercao.csv`
- `dados_sorteio.csv`
- `dados_remocao.csv`

Esses arquivos são utilizados para a geração dos gráficos.

---

## Critério de Contagem

A contagem considera **somente comparações de busca de posição**, ou seja:

- igualdade (`==`)
- menor (`<`)
- maior (`>`)

No caso da AVL:

- rotações de balanceamento
- recálculo de alturas

**não são contabilizados**.

Isso foi feito para isolar exclusivamente o custo lógico da busca, permitindo uma comparação mais justa entre as estruturas.

---

## Estrutura dos Arquivos

```text
.
├── comparacoes.c        # Código principal para geração dos dados
├── plot_arvores.m       # Script MATLAB para geração dos gráficos
├── dados_insercao.csv   # Dados das inserções
├── dados_sorteio.csv    # Dados das buscas
├── dados_remocao.csv    # Dados das remoções
├── teste1.png           # Exemplo dos gráficos gerados
└── explicacao.png       # Explicação visual dos resultados
```

---

## Compilação e Execução

Compile o programa com:

```bash
gcc comparacoes.c -o comparacoes
```

Execute com:

```bash
./comparacoes
```

A cada execução, novos arquivos `.csv` serão gerados.

---

## Geração dos Gráficos no MATLAB Online

Para gerar os gráficos:

1. Acesse o MATLAB Online.
2. Crie uma conta gratuita.
3. Faça upload dos arquivos:
   - `dados_insercao.csv`
   - `dados_sorteio.csv`
   - `dados_remocao.csv`
   - `plot_arvores.m`
4. Abra o script `plot_arvores.m`.
5. Clique no botão **Run**.

---

## Gráficos Gerados

O script gera quatro gráficos:

### 1. Inserção

Mostra a evolução do custo de inserção ao longo das 32766 inserções.

---

### 2. Busca

Mostra o custo das 200 buscas aleatórias.

---

### 3. Remoção

Mostra o custo das 200 remoções aleatórias.

---

### 4. Médias Gerais

Apresenta um gráfico de barras comparando as médias das operações entre BST e AVL.

---

## Interpretação dos Resultados

### BST

A BST apresenta maior variação no número de comparações.

Isso ocorre porque sua eficiência depende diretamente da ordem de inserção.

Em alguns casos, a árvore pode ficar desbalanceada, aumentando significativamente o custo das operações.

---

### AVL

A AVL mantém a árvore balanceada durante toda a execução.

Por isso:

- apresenta comportamento mais estável
- reduz picos de custo
- mantém complexidade próxima de `O(log n)`

---

### Comparação Final

Embora em alguns casos específicos a BST possa apresentar menor custo, isso ocorre apenas por coincidência na distribuição dos elementos.

Na média geral, a AVL apresenta desempenho mais consistente e eficiente.
