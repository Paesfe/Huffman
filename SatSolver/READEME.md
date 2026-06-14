# SAT & SMT Solver (LIA) com Árvore de Decisão e Backtracking

Este projeto é um solucionador lógico duplo desenvolvido em linguagem C. Ele atua tanto como um **SAT Solver** (Satisfiabilidade Booleana) para Fórmulas na Forma Normal Conjuntiva (CNF), quanto como um **SMT Solver** (Satisfiability Modulo Theories) para lidar com restrições matemáticas baseadas na Teoria da Aritmética Linear Inteira (**LIA**).

O programa busca uma solução válida por meio de uma árvore de decisão binária com técnica de *Backtracking* (busca com retrocesso) e algoritmos de poda antecipada, validando tanto a lógica booleana quanto a consistência dos intervalos matemáticos.

---

## 1. Lógica por Trás do Funcionamento

O problema central consiste em determinar se existe uma atribuição lógica (`Verdadeiro` ou `Falso`) para um conjunto de variáveis que torne toda a fórmula verdadeira, respeitando simultaneamente as regras matemáticas associadas a elas.

### A. O Formato de Entrada Híbrido
O programa recebe os dados através de um arquivo texto local unificado:
* Linhas iniciadas com `c` são comentários.
* A linha `p cnf [átomos] [cláusulas]` define o cabeçalho lógico padrão DIMACS.
* As linhas numéricas seguintes representam os literais lógicos (terminando em `0`).
* A linha `t lia [num_equacoes]` define o cabeçalho matemático SMT.
* As equações seguem o formato mapeado: `f[ID] [a]x +- [b] [operador] [k]` (Ex: `f1 2x + 3 >= 9`).

### B. O Motor SAT: Avaliação e Árvore de Decisão (`solveSAT`)
A busca booleana é feita através de uma árvore binária recursiva, onde o estado das variáveis é rastreado **no próprio ramo da árvore** (através do ponteiro `parent`), eliminando a necessidade de arrays globais:
1. Cada nível testa um átomo lógico ($x_1, x_2, \dots$). O ramo esquerdo assume `1` (True) e o direito `0` (False).
2. Avaliação por Curto-Circuito: Se uma única cláusula falhar inteiramente (todos os literais falsos), a fórmula inteira falha. O algoritmo **poda** o ramo e retrocede (*Backtracking*).

### C. O Motor SMT: Consistência LIA (`solveSMT`)
Quando equações matemáticas são carregadas, o solver não busca apenas satisfazer as cláusulas booleanas, mas garantir que as regras numéricas não entrem em colapso.
1. Uma atribuição de variável (ex: $f_1 = \text{True}$) ativa a restrição matemática associada (ex: $2x \ge 10$).
2. A negação lógica inverte o operador (se $f_1 = \text{False}$, então a restrição vira $2x < 10$).
3. O algoritmo calcula dinamicamente as interseções, adaptando as equações para isolar o $x$ através de divisões de piso e teto (`floorDiv`, `ceilDiv`). Se a intersecção final resultar em um máximo menor que um mínimo (Ex: $x \ge 5$ e $x \le 2$), a teoria é considerada matematicamente **inconsistente** e o ramo sofre poda SMT imediata.

---

## 2. Estruturas de Dados

O programa utiliza estruturas dinâmicas e listas encadeadas multidimensionais para gerenciar o problema:

### Estruturas Lógicas (SAT)
* **`Literal`**: Nó de uma lista encadeada. Armazena o ID do átomo (`atomID`) e se ele está negado (`isNegative`).
* **`Clause`**: Nó que guarda a cabeça da lista de literais de uma cláusula.
* **`Formula`**: Nó mestre que orquestra o problema lógico (`clauseHead`, `clauseCount`, `atomCount`).
* **`DecisionNode`**: Nó da Árvore de Decisão. Armazena o átomo testado, o valor assumido (`polarity`), os filhos (`left`, `right`) e o ponteiro fundamental de histórico (`parent`), usado para inferir o caminho tomado sem o uso de arrays auxiliares.

### Estruturas Matemáticas (SMT/LIA)
* **`LIAConstraint`**: Representa uma equação matemática isolada. Armazena os coeficientes linear/angular, símbolo de operação, limite constante, e o ID da variável booleana atrelada a ela.
* **`LIATheory`**: Gerencia a lista completa de restrições matemáticas do arquivo.
* **`Interval`**: Estrutura de retorno que guarda o limite inferior (`minimumValue`) e superior (`maximumValue`) permitidos para a incógnita matemática $x$.

---

## 3. Documentação das Principais Funções

### Gerenciamento de Arquivos
* `openFile()`: Solicita o arquivo e tenta abri-lo de forma resiliente, buscando localmente, na pasta `Test Cases/` e na raiz paralela.
* `readFile()`: É um **parser unificado** que extrai os comandos lógicos e matemáticos num só passe lógico através de um `switch/case`.
* `readEquations()`: Faz o *parsing* de strings do C para separar coeficientes, variáveis, e deslocamentos das equações LIA (Ex: de string `"2x + 3 >= 9"` para dados estruturados).

### Processamento de Árvore e Lógica
* `evaluateFormula()`: Analisa as cláusulas cruzando-as com a interpretação atual do nó-folha e retorna se é `1` (SAT), `0` (UNSAT) ou `-1` (Indefinido/Inconclusivo).
* `getVarValue()`: Sobe na hierarquia da árvore do nó atual até a raiz testando os ponteiros `parent` para recuperar o valor (0 ou 1) que foi delegado ao átomo em etapas anteriores.
* `solveSAT()` / `solveSMT()`: Funções mutuamente exclusivas acionadas dependendo do modo de execução. Realizam a descida recursiva (*backtracking in-place*) criando novos `DecisionNode`s. O `solveSMT` engloba o papel de chamar o `evaluateMathematicalConsistency()`.
* `calculateLIAInterval()`: Motor de redução algébrica. Limpa os offsets, inverte multiplicadores negativos, isola o $x$ convertendo sinais de desigualdade e retorna uma estrutura `Interval` finalizada.