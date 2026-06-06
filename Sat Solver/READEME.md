# SAT Solver com Árvore de Decisão e Backtracking

Este projeto é um **SAT Solver** simplificado desenvolvido em linguagem C. O programa lê a Fórmula na Forma Normal Conjuntiva (CNF), padrão **DIMACS**, e buscar uma solução válida por meio de uma árvore de decisão binária com técnica de *Backtracking* (busca com retrocesso) e algoritmos de poda. Sua representação em memória é dada por listas encadeadas multidimensionais.

---

## 1. Lógica por Trás do Funcionamento

O problema do SAT consiste em determinar se existe uma atribuição de valores lógicos (`Verdadeiro` ou `Falso`) para um conjunto de variáveis booleanas que torne toda a fórmula verdadeira.

### A. O Formato DIMACS (.cnf)
O programa recebe os dados através de um arquivo texto local seguindo as regras:
* Linhas iniciadas com `c` são comentários.
* A linha `p cnf [átomos] [cláusulas]` define o cabeçalho.
* As linhas seguintes contêm números inteiros que representam os literais: números positivos são átomos normais, números negativos representam átomos negados e o número `0` indica o fim daquela cláusula.

### B. Avaliação por Curto-Circuito (`evaluateFormula`)
1.  **Cláusula Verdadeira:** Se uma cláusula contiver **pelo menos um** literal verdadeiro, a cláusula inteira já é considerada **Verdadeira**.
2.  **Poda por Inviabilidade (Falsa):** Se uma cláusula tiver todos os seus literais avaliados como falsos e nenhum literal indefinido, ela se torna **Falsa**. Como as cláusulas são unidas por `E`, se uma única cláusula falhar, a fórmula inteira falha. O algoritmo então **poda** o restante deste ramo e retrocede (*Backtracking*).
3.  **Indefinido:** Se a fórmula não for totalmente verdadeira nem contiver cláusulas explicitamente falsas, significa que precisamos continuar descendo na árvore e testando mais possibilidades.

### C. A Árvore de Decisão Binária (`solveSAT`)
A busca é feita através de uma árvore binária recursiva:
* Cada nível da árvore testa um átomo lógico específico ($x_1, x_2, \dots$).
* O **ramo esquerdo** assume que o átomo atual vale `1` (True).
* O **ramo direito** assume que o átomo atual vale `0` (False).
* Se ambos os ramos falharem, o nó retorna falso para o seu "pai" e desfaz as alterações de memória (Backtracking in-place).

---

## 2. Estruturas de Dados

O programa utiliza estruturas de dados dinâmicas para mapear a fórmula e o progresso da árvore de decisão:

* **`literal`**: Nó de uma lista encadeada. Armazena o ID do átomo correspondente (`atomID`) e se ele está negado (`isNegative`).
* **`clause`**: Nó de uma lista encadeada de cláusulas.
* **`formula`**: Nó mestre que gerencia o problema, guardando a cabeça da lista de cláusulas, a quantidade delas (`clauseCount`) e o total de variáveis únicas (`atomCount`).
* **`partialInt`**: Encapsula um array dinâmico (`truthValue`) modificado de forma in-place durante o percurso da árvore. Guarda os estados: `UNDEFINED` (-1), `False` (0) ou `True` (1).
* **`DecisionNode`**: Representa um nó da Árvore de Decisão. Ele armazena qual átomo está testando, o valor escolhido, ponteiros para os filhos esquerdo/direito e um booleano `isSAT` para registrar se aquele caminho obteve sucesso.

---

## 3. Documentação das Funções

### Gerenciamento de Arquivos e Leitura
* `FILE *openFile()`: Solicita ao usuário o nome do arquivo texto. Possui lógica de "fallback" inteligente que tenta abrir o arquivo no diretório atual e, em caso de erro, tenta no diretório pai (`../`).
* `formula *initializeFormula()`: Aloca a estrutura principal da fórmula e zera seus contadores.
* `formula *createFormulaCNF(FILE *file)`: Faz leitura do arquivo validado. Ignora comentários (`c`), interpreta o cabeçalho (`p cnf`) e orquestra a leitura das cláusulas.
* `int formulaReader(formula *f, FILE *file)`: Um laço que roda de acordo com a quantidade de cláusulas informada no cabeçalho, instanciando cada uma delas.
* `clause *addClause(formula *f)`: Cria uma cláusula vazia e a insere no **início** da lista encadeada da fórmula (funciona como uma Pilha).
* `literal *addLiteral(literal *l, int variable, bool isNegative)`: Cria um novo literal e o insere no **início** da lista encadeada da cláusula correspondente.
* `int clauseReader(clause *c, formula *f, FILE *file)`: Lê os inteiros do arquivo até encontrar o `0`. Valida se o número não ultrapassa o limite máximo e adiciona o literal à cláusula.
* `void printFormula(formula *f)`: Função utilitária que percorre as listas encadeadas e exibe a fórmula de maneira legível no terminal, ex: `(1 V ~2) ^ (~1 V 3)`.

### Motor de Inferência e Árvore de Decisão
* `partialInt initializePartialInterp(formula *f)`: Aloca o array de interpretação e define todas as variáveis inicialmente como `UNDEFINED` (-1).
* `int evaluateFormula(formula *f, partialInt *pi)`: Analisa o estado atual do array global de interpretações contra as cláusulas e retorna `1` (SAT), `0` (UNSAT) ou `-1` (Indefinido).
* `DecisionNode *solveSAT(formula *f, partialInt *pi, int currentVar)`: Executa a recursão e o *Backtracking* modificando os valores do ponteiro do array principal `pi` (in-place). Avalia o estado atual da fórmula e ramifica para esquerda (True) e direita (False). 
* `void printSolution(DecisionNode *root, int totalVars, partialInt *pi)`: Verifica a flag `isSAT` da árvore resultante e exibe em tela os valores do array de interpretação que resolveram o problema.

### Gerenciamento Dinâmico de Memória
* `void freeFormula(formula *f)`: Deleta os literais de uma cláusula, depois deleta a cláusula em si, repetindo o processo até liberar a estrutura da fórmula inteira.
* `void freeTree(DecisionNode *node)`: Executa uma varredura recursiva pós-ordem na árvore de decisão para deletar os nós da memória heap.