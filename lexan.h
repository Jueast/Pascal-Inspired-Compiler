/* lexan.h */
/**
 * Lexical elements that are recognised by Lexical analy
 */
#include <string>
typedef enum {
    IDENT, INTEGER, PLUS, MINUS, TIMES, DIVIDE, kwMOD,
    EQ, NEQ, LT, GT, LTE, GTE, LPAR,
    kwAND,kwOR,kwNOT,
    RPAR, LBRA, RBRA, ASSIGN,
    COMMA, SEMICOLON, COLON, DOT,
    kwPROGRAM, kwFUNCTION, kwVAR, kwCONST, kwBEGIN, kwEND,
    kwINTEGER,
    kwIF, kwTHEN, kwELSE,
    kwARRAY, kwOF,
    kwWHILE, kwDO, kwWRITE, kwREAD,
    kwFOR, kwTO, kwDOWNTO, kwBREAK,
    EOI, ERR
 } LexSymbolType;

extern const char *symbTable[50];

#define MAX_IDENT_LEN 32

typedef struct LexicalSymbol {
  LexSymbolType type;
  std::string      ident;  /* atribute of IDENT token */
  int       value;                 /* atribute of INTEGER token */
} LexicalSymbol;

LexicalSymbol readLexem(void);
int initLexan(char*);
