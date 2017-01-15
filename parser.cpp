#include "parser.h"
#include "ast.h"
#include "lexan.h"
#include "tabsym.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

StatmList* Program(void);
void ProgramHead(void);
StatmList* Block(void);
void DeclarationPart(void);
void ConstDeclarationPart(void);
void VarDeclarationPart(void);
StatmList* StatmentPart(void);
StatmList* StatmentSequence(StatmList* init);
Statm* Statment(void);
Statm* For(void);
Expr* Condition(void);
Statm* ElsePart(void);
Op RelOp(void);
Expr* Expression(void);
Expr* ExpressionPrime(Expr*);
Expr* Term(void);
Expr* TermPrime(Expr*);
Expr* Factor(void);

LexicalSymbol Symb;

void CompareError(LexSymbolType s) { 
    std::cout << "Error while comparing, expected " << symbTable[s] 
            << " in " << lineNumber << " line."<< std::endl;
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
    StatmList* result = Block();
    Compare(DOT);
    return result;
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
    if(Symb.type == kwCONST){
        Compare(kwCONST);
        ConstDeclarationPart();
    }
    if(Symb.type == kwVAR){
        Compare(kwVAR);
        VarDeclarationPart();
    }
    return;
}
void ConstDeclarationPart() {
   if(Symb.type != IDENT)
        return;
   std::string name = Symb.ident;
   Compare(IDENT);
   Compare(COLON);
   if(Symb.type == kwINTEGER){
        Compare(kwINTEGER);
        Compare(EQ);
        int value = Compare(INTEGER).value;
        declConstInt(name, value);
        Compare(SEMICOLON);
   }
   else{}
   ConstDeclarationPart();
}

void VarDeclarationPart() {
   if(Symb.type != IDENT)
        return;
   std::vector<std::string> names;
   names.push_back(Symb.ident);
   Compare(IDENT);
   while(Symb.type != COLON){
        Compare(COMMA);
        LexicalSymbol id = Compare(IDENT);
        names.push_back(id.ident);
   }
   Compare(COLON);
   if(Symb.type == kwINTEGER){
        Compare(kwINTEGER);
        for(auto it = names.begin(); it < names.end(); it++){
            declVar("Integer", *it);
        }
   }
   else{}
   Compare(SEMICOLON);
   VarDeclarationPart();
}

StatmList* StatmentPart(void) {
    /* StatmentPart ->begin StatmentSequence end.*/
    Compare(kwBEGIN);
    StatmList* n = StatmentSequence(nullptr);
    Compare(kwEND);
    if(Symb.type == SEMICOLON){
        Symb = readLexem();
    }
    return n;
}
StatmList* StatmentSequence(StatmList * init) {
    if(!init)
        init = new StatmList();
    while(Symb.type != kwEND){
        init->add(Statment());
    }
    return init;
}

Statm* Statment(void){
    switch(Symb.type) {
        case kwWRITE: {
            Symb = readLexem();
            Expr* n = Expression();
            Compare(SEMICOLON);
            return new Write(n);
        }
        case kwREAD: {
            Symb = readLexem();
            auto id = Compare(IDENT);
            Compare(SEMICOLON);
            return new Read(new Var(id.ident, false));     
        }
        case kwIF: {
            Symb = readLexem();
            Expr *cond = Condition();
            Compare(kwTHEN);
            Statm* thenPart;
            if(Symb.type == kwBEGIN)
                thenPart = StatmentPart();
            else
                thenPart = Statment();
            return new If(cond, thenPart, ElsePart());
        }
        case kwWHILE: {
            Expr *cond;
            Symb = readLexem();
            cond = Condition();
            Compare(kwDO);
            Statm* body;
            if(Symb.type == kwBEGIN)
                body = StatmentPart();
            else
                body = Statment();
            return new While(cond, body);

        }
        case kwFOR:
            return For();
        case kwBREAK:
            Symb = readLexem();
            if(Symb.type == SEMICOLON)
                Symb = readLexem();
            return new Break();
        case IDENT: {
            Statm* result = NULL; 
            auto id = Compare(IDENT).ident;
            int v;
            SymbolType st = checkSymbolType(id, &v);
            switch(st){
                    case Const:
                    case VarId:
                        Compare(ASSIGN);
                        result = new Assign(new Var(id, false), Expression());
                    case Func:
                    default:break;
            }
            Compare(SEMICOLON);
            return result; 
     }
        default:
            ExpansionError("Statment", Symb.type);
            return nullptr;
    }
}

Statm* For(){
    Symb = readLexem();
    std::string name = Compare(IDENT).ident;
    Compare(ASSIGN);
    Expr* init = Expression();
    Statm* initstat = new Assign(new Var(name, false), init);
    int step = Symb.type == kwTO ? 1 : -1;
    Symb = readLexem();
    Expr* fin = Expression();
    BinOp* cond = new BinOp(Neq, new BinOp(Sub, new Var(name, true), new IntConst(step)), fin);
    Compare(kwDO);
    StatmList* body = new StatmList();
    body->add(initstat);
    if(Symb.type == kwBEGIN){
        Symb = readLexem();
        body = StatmentSequence(body);
        Compare(kwEND);
        if(Symb.type == SEMICOLON)
            Symb = readLexem();
    } else 
        body->add(Statment());
    body->add(new Assign(new Var(name, false), new BinOp(Add, new Var(name, true), new IntConst(step))));
    return new While(cond, body);
}

Statm* ElsePart() {
    Statm* elsePart = nullptr;
    if (Symb.type == kwELSE) {
        Symb = readLexem();
        if(Symb.type == kwBEGIN)
            elsePart = StatmentPart();
        else
            elsePart = Statment();
    }
    return elsePart;
}
Expr* Condition() {
    Expr *left = Expression();
    Op op = RelOp();
    Expr *right = Expression();
    return new BinOp(op, left, right);
}
Op RelOp() {
    switch(Symb.type) {
        case EQ:
            Symb = readLexem();
            return Eq;
        case NEQ:
            Symb = readLexem();
            return Neq;
        case LT:
            Symb = readLexem();
            return Lt;
        case GT:
            Symb = readLexem();
            return Gt;
        case LTE:
            Symb = readLexem();
            return Lte;
        case GTE:
            Symb = readLexem();
            return Gte;
        default:
            ExpansionError("RelOp", Symb.type);
            return Error;
    
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
            return ExpressionPrime(new BinOp(Add, l, Term()));
        case MINUS:
            /* E'-> - T E' */
            Symb = readLexem();
//            printf(" E' -> - T E'\n");
            return ExpressionPrime(new BinOp(Sub, l, Term()));
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
            return TermPrime(new BinOp(Mult, l, Factor()));
        case DIVIDE:
            Symb = readLexem();
//            printf(" T' -> / F T'\n");
            return TermPrime(new BinOp(Div, l, Factor()));
        case kwMOD:
            Symb = readLexem();
            return TermPrime(new BinOp(Mod, l, Factor()));
        default:
            return l;

    }
}

Expr* Factor(){
    switch (Symb.type) {
        case IDENT: {
            std::string id = Symb.ident;
            Symb = readLexem();
            return VarOrConst(id);
        }
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
