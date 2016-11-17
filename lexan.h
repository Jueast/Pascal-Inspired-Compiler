/* lexan.h */
/**
 * Lexical elements that are recognised by Lexical analy
 */
 typedef enum {
    IDENT, INTEGER, PLUS, MINUS, TIMES, DIVIDE,
    EQ, NEQ, LT, GT, LTE, GTE, LPAR, RPAR, LBRA, RBRA, ASSIGN,
    COMMA, SEMICOLON, COLON, DOT,
    kwPROGRAM, kwFUNCTION, kwVAR, kwCONST, kwBEGIN, kwEND,
    kwINTEGER,
    kwIF, kwTHEN, kwELSE,
    kwWHILE, kwDO, kwWRITELN, kwREADLN,
    EOI, ERR
 } LexSymbolType;

extern const char *symbTable[40];

#define MAX_IDENT_LEN 32

typedef struct LexicalSymbol {
  LexSymbolType type;
  char      ident[MAX_IDENT_LEN];  /* atribute of IDENT token */
  int       value;                 /* atribute of INTEGER token */
} LexicalSymbol;

LexicalSymbol readLexem(void);
int initLexan(char*);
