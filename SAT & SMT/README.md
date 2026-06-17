# SAT Solver / SMT Solver (LIA)

## Visão Geral

Este projeto implementa um resolvedor de **SAT** e **SMT (LIA - Linear
Integer Arithmetic)** utilizando **árvores de decisão com
backtracking**.

A ideia central é que o **SMT (Satisfiability Modulo Theories)** é uma
expansão natural do **SAT (Boolean Satisfiability)**:

-   **SAT** verifica apenas se existe uma combinação de valores
    booleanos (`true`/`false`) que torna uma fórmula lógica verdadeira.
-   **SMT** faz a mesma verificação lógica, mas adiciona **restrições
    matemáticas** sobre variáveis.

Em outras palavras:

``` text
SAT ⊂ SMT
```

Ou seja, todo problema SAT pode ser visto como um caso particular de SMT
sem teoria adicional.

------------------------------------------------------------------------

## Lógica do Projeto

O fluxo do programa funciona assim:

### 1. Leitura do arquivo

O parser identifica dois possíveis modos:

-   Apenas `p cnf` → modo **SAT**
-   `p cnf` + `t lia` → modo **SMT**

------------------------------------------------------------------------

### 2. Avaliação booleana

A fórmula CNF é avaliada:

-   Se alguma cláusula for falsa → poda (UNSAT)
-   Se todas forem verdadeiras → SAT
-   Se ainda existirem variáveis indefinidas → continua expandindo

------------------------------------------------------------------------

### 3. Expansão SMT

No modo SMT:

Cada átomo booleano pode representar uma restrição matemática.

Exemplo:

``` text
f1 → x >= 5
f2 → x <= 10
```

Se:

``` text
f1 = true
f2 = true
```

Então:

``` text
5 <= x <= 10
```

O solver calcula esse intervalo.

Se o intervalo for inválido:

``` text
x >= 10 e x <= 2
```

Esse ramo é podado imediatamente.

Isso torna SMT mais eficiente que SAT puro em muitos problemas.

------------------------------------------------------------------------

# Modelo de Entrada SAT

Formato padrão:

``` text
c comentario opcional
p cnf <numero_variaveis> <numero_clausulas>

<literal1> <literal2> ... 0
<literal1> <literal2> ... 0
...
```

Exemplo:

``` text
c Exemplo SAT
p cnf 3 2
1 -2 0
2 3 0
```

Interpretação:

``` text
(x1 OR ¬x2) AND (x2 OR x3)
```

------------------------------------------------------------------------

# Modelo de Entrada SMT

O SMT mantém exatamente o mesmo padrão do SAT.

A única diferença é adicionar:

``` text
t lia <numero_restricoes>
```

Formato:

``` text
c comentario opcional
p cnf <numero_variaveis> <numero_clausulas>

<clausulas>

t lia <numero_restricoes>

f<id> <equacao>
```

Exemplo:

``` text
c Exemplo SMT
p cnf 2 2
1 0
2 0

t lia 2
f1 1 x >= 5
f2 1 x <= 10
```

------------------------------------------------------------------------

# Diferença entre SAT e SMT no Input

O parser sempre lê a parte booleana primeiro. Se encontrar a diretiva
`t lia`, o programa ativa o modo SMT e processa também as restrições
matemáticas.

------------------------------------------------------------------------

# Estrutura do Projeto

``` text
Sat Solver/
├── src/
│   ├── main.c
│   ├── sat.c
│   ├── smt.c
│
├── include/
│   ├── sat.h
│   ├── smt.h
│
├── Test Cases/
│   ├── teste_sat.txt
│   ├── teste_smt.txt
```

------------------------------------------------------------------------

# Compilação e Execução 

## Pré-requisitos

É necessário ter um compilador C instalado.

------------------------------------------------------------------------

## Compilação

Na raiz do projeto:

Linux/macOS:

``` bash
gcc src/main.c src/sat.c src/smt.c -o solver
```

Windows:

``` bash
gcc src/main.c src/sat.c src/smt.c -o solver.exe
```

------------------------------------------------------------------------

## Execução

Execute na raiz do projeto.

Linux/macOS:

``` bash
./solver
```

Windows:

``` bash
solver.exe
```

O programa solicitará:

``` text
Digite o nome do arquivo a ser lido:
```

------------------------------------------------------------------------

## Importante

O executável deve ser rodado a partir da raiz do projeto para que a
busca automática pelos arquivos funcione corretamente.

------------------------------------------------------------------------

# Resumo

## SAT

-   Resolve lógica booleana

## SMT

-   Resolve lógica booleana
-   Adiciona teoria matemática
-   Faz poda antecipada usando consistência matemática

A implementação mantém a mesma árvore para ambos. O SMT apenas adiciona
uma camada extra de validação.
