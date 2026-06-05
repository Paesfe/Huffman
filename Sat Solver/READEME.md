# SAT Solver com Árvore de Decisão e Backtracking

Este projeto é um **SAT Solver** simplificado desenvolvido em linguagem C. O programa lê a Fórmula na Forma Normal Conjuntiva (CNF), padrão **DIMACS**, e buscar uma solução válida por meio de uma árvore de decisão binária com técnica de *Backtracking* (busca com retrocesso) e algoritmos de poda. Sua representação em memória é dada por listas encadeadas multidimensionais.

---

## 1. Lógica por Trás do Funcionamento

O problema do SAT consiste em determinar se existe uma atribuição de valores lógicos (`Verdadeiro` ou `Falso`) para um conjunto de variáveis booleanas que torne toda a fórmula verdadeira.

### A. O Formato DIMACS (.cnf)
O programa recebe os dados através da entrada padrão seguindo as regras:
* Linhas iniciadas com `c` são comentários.
* A linha `p cnf [variáveis] [cláusulas]` define o cabeçalho.
* As linhas seguintes contêm números inteiros que representam os literais: números positivos são variáveis normais, números negativos representam variáveis negadas e o número `0` indica o fim daquela cláusula.

### B. Avaliação por Curto-Circuito (`evaluateFormula`)
1.  **Cláusula Verdadeira:** Se uma cláusula contiver **pelo menos um** literal verdadeiro, a cláusula inteira já é considerada **Veradeira**.
2.  **Poda por Inviabilidade (Falsa):** Se uma cláusula tiver todos os seus literais avaliados como falsos e nenhum literal indefinido, ela se torna **Falsa**. Como as cláusulas são unidas por `E`, se uma única cláusula falhar, a fórmula inteira falha. O algoritmo então **poda** o restante deste ramo e retrocede (*Backtracking*).
3.  **Indefinido:** Se a fórmula não for totalmente verdadeira nem contiver cláusulas explicitamente falsas, significa que precisamos continuar descendo na árvore e testando mais variáveis.

### C. A Árvore de Decisão Binária (`solveSAT`)
A busca é feita através de uma árvore binária recursiva:
* Cada nível da árvore testa uma variável específica ($x_1, x_2, \dots$).
* O **ramo esquerdo** assume que a variável atual vale `1` (True).
* O **ramo direito** assume que a variável atual vale `0` (False).

---

## 2. Estruturas de Dados

O programa utiliza estruturas de dados dinâmicas para mapear a fórmula e o progresso da árvore de decisão:

* **`literal`**: Nó de uma lista encadeada. Armazena o índice da variável (`variable`) e se ela está negada (`isNegative`).
* **`clause`**: Nó de uma lista encadeada de cláusulas.
* **`formula`**: Nó mestre que gerencia o problema, guardando a cabeça da lista de cláusulas, a quantidade delas (`clauseCount`) e o total de variáveis únicas (`variableCount`).
* **`partialInt`**: Encapsula um array dinâmico (`valores`) onde o índice do array corresponde ao número da variável. Guarda os estados: `UNDEFINED` (-1), `False` (0) ou `True` (1).
* **`nodeBinaryTree`**: Representa um nó da Árvore de Decisão. Ele armazena qual variável está testando, o valor escolhido, uma cópia do estado da interpretação naquele momento, ponteiros para os filhos esquerdo/direito e um booleano `isSAT` para registrar se aquele caminho obteve sucesso.

---

## 3. Documentação das Funções

### Gerenciamento e Leitura da Fórmula
* `formula *initializeFormula()`: Aloca a estrutura principal da fórmula e zera seus contadores.
* `formula *createFormulaCNF()`: Faz leitura da entrada padrão. Ignora comentários (`c`), interpreta o cabeçalho (`p cnf`) e orquestra a leitura das cláusulas.
* `int formulaReader(formula *f)`: Um laço que roda de acordo com a quantidade de cláusulas informada no cabeçalho, instanciando cada uma delas.
* `clause *addClause(formula *f)`: Cria uma cláusula vazia e a insere no **início** da lista encadeada da fórmula (funciona como uma Pilha).
* `literal *addLiteral(literal *l, int variable, bool isNegative)`: Cria um novo literal e o insere no **início** da lista encadeada da cláusula correspondente.
* `int clauseReader(clause *c, formula *f)`: Lê os números inteiros do terminal até encontrar o `0`. Valida se o número não ultrapassa o limite máximo de variáveis e adiciona o literal positivo ou negativo à cláusula.
* `void printFormula(formula *f)`: Função utilitária que percorre as listas encadeadas e exibe a fórmula de maneira legível no terminal, ex: `(1 V ~2) ^ (~1 V 3)`.

### Motor de Inferência e Árvore de Decisão
* `partialInt initializePartialInterp(formula *f)`: Aloca o array de interpretação e define todas as variáveis inicialmente como `UNDEFINED` (-1).
* `int evaluateFormula(formula *f, partialInt *pi)`: Analisa o estado atual das variáveis contra as cláusulas e retorna `1` (SAT), `0` (UNSAT) ou `-1` (Indefinido).
* `partialInt cloneInterpretation(partialInt *oldPi, int totalVars, int currentVar, short guess)`: Aloca um novo array, copia as escolhas feitas pelos nós anteriores da árvore e aplica o novo "chute" em cada nó
* `nodeBinaryTree *solveSAT(formula *f, partialInt *pi, int currentVar)`: Cria um nó da árvore, avalia o estado atual da fórmula e ramifica recursivamente para a esquerda (True) e para a direita (False)
* `void printSolution(nodeBinaryTree *node, int totalVars)`: Percorre a árvore de decisão seguindo estritamente os nós que possuem `isSAT == true` até a folha de sucesso, imprimindo a combinação de variáveis que solucionou o problema.

### Gerenciamento Dinâmico de Memória
* `void freeFormula(formula *f)`: Deleta os literais de uma cláusula, depois deleta a cláusula em si, repetindo o processo até liberar a estrutura da fórmula inteira.
* `void freeTree(nodeBinaryTree *node)`: Executa uma varredura recursiva pós-ordem na árvore de decisão. Ela desce até os nós folha, libera o array de valores clonado 
