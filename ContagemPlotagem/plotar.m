% =========================================================================
% Script MATLAB para plotar as comparacoes de BST x AVL
% nas operacoes de INSERCAO, BUSCA e REMOCAO
% =========================================================================

%% --- Leitura dos dados de INSERCAO ---
%  le o arquivo gerado durante a fase de insercao de todos os
% elementos nas arvores
dadosInsercao = readtable('dados_insercao.csv');

insercao   = dadosInsercao.Insercao;
compBSTins = dadosInsercao.Comp_BST;
compAVLins = dadosInsercao.Comp_AVL;

%% --- Leitura dos dados de BUSCA (original) ---
dadosBusca = readtable('dados_sorteio.csv');

sorteio    = dadosBusca.Sorteio;
compBSTbus = dadosBusca.Comp_BST;
compAVLbus = dadosBusca.Comp_AVL;

%% --- Leitura dos dados de REMOCAO ---
%  le o arquivo gerado durante a fase de remocao dos elementos
dadosRemocao = readtable('dados_remocao.csv');

remocao    = dadosRemocao.Remocao;
compBSTrem = dadosRemocao.Comp_BST;
compAVLrem = dadosRemocao.Comp_AVL;

%% --- Figura 1: Comparacoes durante a INSERCAO ---
%  grafico mostrando como o numero de comparacoes evolui ao longo
% das 32766 insercoes (a BST tende a degenerar mais que a AVL)
figure;
plot(insercao, compBSTins, 'b-', 'LineWidth', 1);
hold on;
plot(insercao, compAVLins, 'r-', 'LineWidth', 1);
hold off;
xlabel('Numero da insercao');
ylabel('Numero de comparacoes');
title('Comparacoes durante a insercao: BST x AVL');
legend('BST', 'AVL', 'Location', 'northwest');
grid on;

%% --- Figura 2: Comparacoes durante a BUSCA (original) ---
figure;
plot(sorteio, compBSTbus, 'b-o', 'LineWidth', 1, 'MarkerSize', 4);
hold on;
plot(sorteio, compAVLbus, 'r-o', 'LineWidth', 1, 'MarkerSize', 4);
hold off;
xlabel('Numero do sorteio');
ylabel('Numero de comparacoes');
title('Comparacoes durante a busca: BST x AVL');
legend('BST', 'AVL', 'Location', 'northwest');
grid on;

%% --- Figura 3: Comparacoes durante a REMOCAO ---
%  grafico equivalente ao de busca, mas para as remocoes
figure;
plot(remocao, compBSTrem, 'b-o', 'LineWidth', 1, 'MarkerSize', 4);
hold on;
plot(remocao, compAVLrem, 'r-o', 'LineWidth', 1, 'MarkerSize', 4);
hold off;
xlabel('Numero da remocao');
ylabel('Numero de comparacoes');
title('Comparacoes durante a remocao: BST x AVL');
legend('BST', 'AVL', 'Location', 'northwest');
grid on;

%% --- Figura 4: Comparacao das medias gerais (resumo) ---
%  grafico de barras comparando a media de comparacoes de BST e AVL
% nas tres operacoes, facilitando a visualizacao geral do desempenho
mediasBST = [mean(compBSTins), mean(compBSTbus), mean(compBSTrem)];
mediasAVL = [mean(compAVLins), mean(compAVLbus), mean(compAVLrem)];

figure;
bar([mediasBST; mediasAVL]');
set(gca, 'XTickLabel', {'Insercao', 'Busca', 'Remocao'});
ylabel('Media de comparacoes');
title('Media de comparacoes por operacao: BST x AVL');
legend('BST', 'AVL', 'Location', 'northeast');
grid on;
