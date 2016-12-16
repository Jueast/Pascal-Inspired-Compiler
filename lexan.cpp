#include "lexan.h"
#include "input.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
typedef enum {
    LETTER, DIGIT, WHITE_SPACE, ENDFILE,UNDERLINE, NO_TYPE
}InputCharType;
// same order in LexSymbolType
const char *symbTable[40] = {
    "IDENT","INTEGER","PLUS","MINUS","TIMES","DIVIDE",
    "EQ","NEQ","LT","GT","LTE","GTE","LPAR",
    "RPAR","LBRA","RBRA","ASSIGN","COMMA","SEMICOLON",
    "COLON","DOT","kwPROGRAM","kwFUNCTION","kwVAR","kwCONST",
    "kwBEGIN","kwEND","kwINTEGER","kwIF","kwTHEN","kwELSE",
    "kwWHILE","kwDO","kwWRITE","kwREAD","EOI","ERR"
};
static int character; // input symbol
static InputCharType input; // input symbol type

void readInput(void){
    character = getChar();
    if ((character>='A' && character<='Z') || (character>='a' && character<='z'))
        input = LETTER;
    else if (character>='0' && character<='9')
        input = DIGIT;
    else if (character == EOF)
        input = ENDFILE;
    else if (character <= ' ')
        input = WHITE_SPACE;
    else if (character == '_')
        input = UNDERLINE; 
    else
        input = NO_TYPE;
}
void covertToUpper(char* str){
    for(;*str != '\0'; str++){
        *str = toupper(*str);
    }
}
const struct {const char* s; LexSymbolType symb;} keyWordTable[] = {
    {"PROGRAM", kwPROGRAM},
    {"CONST", kwCONST},
    {"VAR", kwVAR},
    {"FUNCTION",kwFUNCTION},
    {"BEGIN", kwBEGIN},
    {"END", kwEND},
    {"INTEGER", kwINTEGER},
    {"IF",kwIF},
    {"THEN",kwTHEN},
    {"ELSE",kwELSE},
    {"WHILE",kwWHILE},
    {"DO", kwDO},
    {"READ", kwREAD},
    {"WRITE", kwWRITE},
    {NULL, (LexSymbolType) 0}
};
LexSymbolType keyWord(const char* id) {
    int i = 0;
    char upperid[MAX_IDENT_LEN];
    strcpy(upperid, id);
    covertToUpper(upperid);
    while(keyWordTable[i].s){
        if(strcmp(upperid, keyWordTable[i].s) == 0)
            return keyWordTable[i].symb;
        else
            i++;
    }
    return IDENT;
}
void lexicalError(const char* text) {
    printf("\nLexical Analysis in line %d : %s\n", lineNumber, text);
    exit(1);
}
LexicalSymbol readLexem(void) {
    LexicalSymbol data;
start:
    switch(character) {
        case '{':
            readInput();
            goto comment;
        case '+':
            data.type = PLUS;
            readInput();
            return data;
        case '-':
            data.type = MINUS;
            readInput();
            return data;
        case '*':
            data.type = TIMES;
            readInput();
            return data;
        case '/':
            data.type = DIVIDE;
            readInput();
            return data;
        case '=':
            data.type = EQ;
            readInput();
            return data;
        case '<':
            readInput();
            goto lt;
        case '>':
            readInput();
            goto gt;
        case '(':
            data.type = LPAR;
            readInput();
            return data;
        case ')':
            data.type = RPAR;
            readInput();
            return data;
        case '[':
            data.type = LBRA;
            readInput();
            return data;
        case ']':
            data.type = RBRA;
            readInput();
            return data;
        case ':':
            readInput();
            goto colon;
        case ',':
            data.type = COMMA;
            readInput();
            return data;
        case ';':
            data.type = SEMICOLON;
            readInput();
            return data;
        case '.':
            data.type = DOT;
            readInput();
            return data;
        default:;
    }
    switch(input){
        case WHITE_SPACE:
            readInput();
            goto start;
        case ENDFILE:
            data.type = EOI;
            return data;
        case LETTER:
            data.ident += character;
            readInput();
            goto letter;
        case DIGIT:
            data.value = character - '0';
            data.type = INTEGER;
            readInput();
            goto digit;
        default:
            data.type = ERR;
            lexicalError("Invalid symbol.");
            return data;
     }
comment:
    switch(character) {
        case '}':
            readInput();
            goto start;
        default:;
    }
    switch(input) {
        case ENDFILE:
            data.type = ERR;
            lexicalError("Uneexpected end of file in a comment.");
            return data;
        default:
            readInput();
            goto comment;
    }
lt:
    if(character == '='){
        data.type=LTE;
        readInput();
        return data;
    }
    else if(character == '>'){
        data.type=NEQ;
        readInput();
        return data;
    }
    else{
        data.type=LT;
        return data; 
    }
gt: 
    if(character == '='){
        data.type=GTE;
        readInput();
        return data;
    }
    else{
        data.type=GT;
        return data; 
    }
colon:
    if(character == '='){
        data.type=ASSIGN;
        readInput();
        return data;
    }
    else{
        data.type=COLON;
        return data; 
    }
letter:
    switch(input) {
    case LETTER:
    case DIGIT:
    case UNDERLINE:
        data.ident += character;
        readInput();
        goto letter;
    default:
        data.type = keyWord(data.ident.c_str());
        return data;
    }
digit:
    switch(input) {
        case DIGIT:
            data.value = 10 * data.value + (character - '0');
            readInput();
            goto digit;
        default:
            return data;
  }
}
int initLexan(char *fileName) {
    if(!initInput(fileName)) return 0;
    readInput();
    return 1;
}
