Nesta pasta, estarão todos os arquivos necessários para a execução e estudo do SAT e SMT.
Em "sat.c", está o código exclusivo para execução do SAT Solver no padrão DIMACS, usando arquivos com a extensão ".cnf".
Em "smt.c" está o código exclusivo para execução do SMT Solver no padrão próprio.
Para criar um arquivo que seja lido corretamente pelo SMT, o arquivo deve estar nesse padrão:

c Este é um arquivo de teste para o meu SMT
c A formula fixa no codigo é (E1 OR E2)
2x + 5 <= 10
3x - 2 >= 4

Apenas com as duas equações, escritas com os devidos espaços e sempre com o coeficiente no "x" (mesmo que seja 1).
No arquivo "testeSMT.txt" se encontra outro exemplo.
No arquivo "testeSAT.cnf" se encontra um arquivo teste para o SAT Solver.
