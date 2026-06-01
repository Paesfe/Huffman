#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// CORREÇÃO: Inclusão correta e limpa dos arquivos de cabeçalho (.h)
#include "../include/compresser.h"
#include "../include/descompresser.h"

#define PATH_SIZE 256

void displayMenu();
void startCompress();
void startDecompress();
void getPath(char *filePath, int size);

int main(){
    while(1){
        displayMenu();

        int CommandInput;
        if (scanf("%d", &CommandInput) != 1) {
            // Limpa o buffer em caso de input inválido não numérico
            while (getchar() != '\n');
            printf("Invalid input, try again!\n");
            continue;
        }
        
        switch (CommandInput){
            case 1:
                startCompress();
                continue;

            case 2:
                startDecompress();
                continue;
            
            case 3:
                exit(0);
            
            default:
                printf("Invalid input, try again!\n");
                break;
        }
    }
    return 0;
}

void displayMenu() {
    printf("\n--------------------------\n");
    printf("Select an option:\n[1] - Compress file\n[2] - Decompress file\n[3] - Exit");
    printf("\n--------------------------\n");
    printf("Enter choice: ");
}

void startCompress() {
    char filePath[PATH_SIZE];
    
    printf("--------------------------\n");
    printf("Enter the file path to be compressed: ");
    getPath(filePath, PATH_SIZE);

    CompressFile(filePath);
}

void startDecompress() {
    char filePath[PATH_SIZE];
    
    printf("--------------------------\n");
    printf("Enter the file path to be decompressed: ");
    getPath(filePath, PATH_SIZE);

    DecompressFile(filePath);
}

void getPath(char *filePath, int size) {    
    int c;
    while ((c = getchar()) != '\n' && c != EOF); 

    fgets(filePath, size, stdin);
    filePath[strcspn(filePath, "\n")] = '\0';
}