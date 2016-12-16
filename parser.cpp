#include "parser.h"
#include "ast.h"
#include "lexan.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

StatmList* Program(void);
void ProgramHead(void);
StatmList* Block(void);
void DeclarationPart(void);
StatmList* StatmentPart(void);
StatmList* StatmentSequence(void);
Statm* Statment(void); // Only Write Now...
Expr* Expression(void);
Expr* ExpressionPrime(Expr*);
Expr* Term(void);
Expr* TermPrime(Expr*);
Expr* Factor(void);

LexicalSymbol Symb;

void CompareError(LexSymbolType s) { 
    std::cout << "Error while comparing, expected " << symbTable[s] << std::endl;
    exit(1);
}

void ExpansionError(std::string nonterminal, LexSymbolType s) {
    std::cout << "Error while expanding nonterminal " << nonterminal
              << ", unexpected token " << symbTable[s] << "." << std::endl;
    exit(1);
}

LexicalSymbol Compare(LexSymbolType s) {
    if (Symb.type == s){
        LexicalSymbol t = Symb;
        Symb = readLexem();
        return t;
    }
    else CompareError(s);
    return Symb; // Add to pass static analysis.
}

StatmList* Program(void) {
    /* Program -> ProgramHead Block */
    ProgramHead();
    return Block();
}

void ProgramHead() {
    /* ProgramHead -> program <name> ; */ 
    Compare(kwPROGRAM);
    std::cout << "Program Name: " << Symb.ident << std::endl;
    Symb = readLexem();
    Compare(SEMICOLON);
}

StatmList* Block(void) {
    /* Block -> DeclarationPart StatmentPart */
    DeclarationPart();
    return StatmentPart();
}

void DeclarationPart() {
    return;
}

StatmList* StatmentPart(void) {
    /* StatmentPart ->begin StatmentSequence end.*/
    Compare(kwBEGIN);
    StatmList* n = StatmentSequence();
    Compare(kwEND);
    Compare(DOT);
    return n;
}
StatmList* StatmentSequence(void) {
    StatmList* sl = new StatmList();
    while(Symb.type != kwEND){
        sl->add(Statment());
    }
    return sl;
}

Statm* Statment(void){
    switch(Symb.type) {
        case kwWRITE: {
            Symb = readLexem();
            Expr* n = Expression();
            Compare(SEMICOLON);
            return new Write(n);
        }
        default:
            ExpansionError("Statment", Symb.type);
            return nullptr;
    }
}
Expr* Expression(void) {
    /* E -> T E' */
//    std::cout << "E -> T E'" << std::endl;
    return ExpressionPrime(Term());
}

Expr* ExpressionPrime(Expr* l) {
    switch (Symb.type) {
        case PLUS:
            /* E'-> + T E' */
            Symb = readLexem();
//            printf(" E' -> + T E'\n");
            return ExpressionPrime(new BinOp('+', l, Term()));
        case MINUS:
            /* E'-> - T E' */
            Symb = readLexem();
//            printf(" E' -> - T E'\n");
            return ExpressionPrime(new BinOp('-', l, Term()));
        default:
            return l;
    }
}

Expr* Term() {
    return TermPrime(Factor());
}

Expr* TermPrime(Expr * l) {
    switch (Symb.type) {
        case TIMES:
            Symb = readLexem();
//            printf(" T' -> * F T' \n");
            return TermPrime(new BinOp('*', l, Factor()));
        case DIVIDE:
            Symb = readLexem();
//            printf(" T' -> / F T'\n");
            return TermPrime(new BinOp('/', l, Factor()));
        default:
            return l;

    }
}

Expr* Factor(){
    switch (Symb.type) {
        case LPAR:{ 
            Symb = readLexem();
            Expr* e = Expression();
            Compare(RPAR);
            return e;
        }
        case MINUS: { 
            Symb = readLexem();
            Expr* f = Factor();
            return new UnMinus(f);
        }
        case INTEGER: {
            int v = Symb.value;
            Symb = readLexem();
            return new IntConst(v);
        }
        default:
            ExpansionError("Factor", Symb.type);
            return 0;
        
    }
}

int initParser(char *fileName) {
    if(!initLexan(fileName)) return 0;
    Symb = readLexem();
    return 1;
}
