/*
    Desenvolver bibliotecas locais
    #include <../include/compresser.h>
    #include <../include/descompresser.h>
    #include <../include/reader.h>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_SIZE 256

void displayMenu();
void startCompress();
void startDecompress();
void getPath(char *filePath, int size);

//Place Holder
void CompressFile(const char* filePath);
void DecompressFile(const char* filePath);

int main(){
    while(1){
        displayMenu();

        int CommandInput;
        scanf("%d", &CommandInput);
        
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

//Place Holder
void CompressFile(const char* filePath){}
void DecompressFile(const char* filePath){}