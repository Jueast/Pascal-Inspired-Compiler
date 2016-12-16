#include <cstdio>
#include "ast.h"
#include "lexan.h"
#include "parser.h"

int main(int argc, char *argv[]) {
    char *fileName;
    Node *res;
    printf("Evaluation of an expression by traversing an abstract syntax tree.\n");
    if (argc == 1) {
        printf("Input from keyboard, write the source code terminated by a dot");
        fileName = NULL;
    } 
    else {
        fileName = argv[1];
        printf("Input file %s.\n", fileName);
    }
    if(!initParser(fileName)) {
        printf("Error creating the syntax analyzer.\n");
        return 0;
    }
    res = Program();
    res->Translate();
    printf("\n---------------End------------------\n");
    return 0;
}
