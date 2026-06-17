# Estrutura de Dados — Projetos e Seminário

Repositório com implementações e material de seminário desenvolvidos para a disciplina de **Estrutura de Dados**. Reúne quatro trabalhos independentes, cada um em sua própria subpasta com README específico.

## Conteúdo do repositório

| Pasta | Descrição |
|---|---|
| [`Huffman/`](./Huffman) | Compactador/descompactador de arquivos genéricos usando codificação de Huffman. |
| [`SAT & SMT/`](./SAT & SMT) | Resolvedor de fórmulas booleanas (SAT) e de aritmética linear inteira (SMT-LIA) via árvore de decisão com backtracking. |
| [`Contagem e Plotagem/`](./ContagemPlotagem) | Comparação experimental de desempenho entre BST e AVL (inserção, busca e remoção), com geração de gráficos em MATLAB. |
| [`Seminario: Arvore B+/`](./SeminarioArvoreB+) | Slides do seminário sobre indexação de bancos de dados com Árvore B+. |

## Resumo de cada projeto

### 🗜️ Huffman
Implementa a árvore de Huffman clássica (fila de prioridade + construção bottom-up) para compactar e descompactar qualquer tipo de arquivo binário, serializando a árvore no próprio cabeçalho do arquivo compactado.

### 🔢 SAT/SMT Solver
Lê uma fórmula em CNF e, opcionalmente, restrições de aritmética linear inteira associadas a seus literais. Constrói uma árvore de decisão binária via backtracking para determinar satisfatibilidade booleana (SAT) ou satisfatibilidade combinada com consistência matemática (SMT).

### 🌳 BST x AVL — Comparação de Desempenho
Programa em C que insere, busca e remove milhares de elementos em uma BST simples e em uma AVL balanceada, contando comparações de chave em cada operação e exportando os resultados para CSV. Um script MATLAB plota os gráficos comparativos.

### 📊 Seminário: Árvore B+
Apresentação sobre o uso de Árvores B+ como estrutura de indexação em SGBDs (MySQL, Oracle), cobrindo o problema do *Full Table Scan*, a anatomia dos nós, o algoritmo de inserção Top-Down e o conceito de Clustered Index.

## Requisitos gerais

- **Compilador C** (GCC recomendado) para todos os projetos de código.
- **MATLAB** (ou MATLAB Online, gratuito) apenas para a etapa de plotagem do projeto BST x AVL.

## Como navegar

Cada subpasta é autocontida e possui seu próprio `README.md` com instruções detalhadas de compilação, execução e formato de entrada/saída. Comece pelo projeto de seu interesse na tabela acima.
