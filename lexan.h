/* lexan.h */
/**
 * Lexical elements that are recognised by Lexical analy
 */
#include <string>
typedef enum {
    IDENT, INTEGER, PLUS, MINUS, TIMES, DIVIDE,
    EQ, NEQ, LT, GT, LTE, GTE, LPAR, RPAR, LBRA, RBRA, ASSIGN,
    COMMA, SEMICOLON, COLON, DOT,
    kwPROGRAM, kwFUNCTION, kwVAR, kwCONST, kwBEGIN, kwEND,
    kwINTEGER,
    kwIF, kwTHEN, kwELSE,
    kwARRAY, kwOF,
    kwWHILE, kwDO, kwWRITE, kwREAD,
    EOI, ERR
 } LexSymbolType;

extern const char *symbTable[40];

#define MAX_IDENT_LEN 32

typedef struct LexicalSymbol {
  LexSymbolType type;
  std::string      ident;  /* atribute of IDENT token */
  int       value;                 /* atribute of INTEGER token */
} LexicalSymbol;

LexicalSymbol readLexem(void);
int initLexan(char*);
