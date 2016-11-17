#include <stdio.h>
#include "input.h"
int main(int argc, char *argv[]) {
    char *fileName;
    char c;
    if (argc == 1) {
        printf("Input from keyboard, write the source code terminated by a dot.\n");
    fileName = NULL;} 
    else {
        fileName = argv[1];
        printf("Input file %s.\n", fileName);
    }
    initInput(fileName);
    while((c = getChar()) != '\0'){
        putchar(c);
    }
    printf("End.\n");
    return 0;
}

