% script MATLAB Online para plotagem do experimento

% 1. Importa os dados do arquivo (que você acabou de fazer o upload)
dados = readtable('dados_sorteio.csv');

% 2. Prepara a janela do gráfico
figure('Name', 'Comparacao de Estruturas de Dados', 'NumberTitle', 'off');
hold on;

% 3. Desenha as duas linhas comparativas
plot(dados.Sorteio, dados.Comp_BST, 'r-o', 'LineWidth', 1.5, 'MarkerFaceColor', 'r');
plot(dados.Sorteio, dados.Comp_AVL, 'b-s', 'LineWidth', 1.5, 'MarkerFaceColor', 'b');

% 4. Deixa o gráfico com aparência profissional
grid on;
title('Arvore Desbalanceada vs Arvore AVL (Custo de Busca)');
xlabel('Indice do Sorteio (1 a 50)');
ylabel('Numero de Comparacoes (Ifs)');
legend('Arvore Desbalanceada (BST)', 'Arvore AVL', 'Location', 'best');

disp('Grafico gerado com sucesso!');