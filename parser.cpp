#include "parser.h"
#include "ast.h"
#include "lexan.h"
#include "tabsym.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

BlockNode* Program(void);
std::string ProgramHead(void);
BlockNode* mainBlock(std::string);
void DeclarationPart(StatmList*);
void ConstDeclarationPart(void);
void VarDeclarationPart(StatmList*);
StatmList* StatmentPart(StatmList*);
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

BlockNode* Program(void) {
    /* Program -> ProgramHead Block */
    std::string returnName = ProgramHead();
    BlockNode* result = mainBlock(returnName);
    Compare(DOT);
    return result;
}

std::string ProgramHead() {
    /* ProgramHead -> program <name> ; */ 
    Compare(kwPROGRAM);
    std::string result = Symb.ident;
    Symb = readLexem();
    Compare(SEMICOLON);
    return result;
}

BlockNode* mainBlock(std::string name) {
    /* Block -> DeclarationPart StatmentPart */
    SymbolTableMap* blockEnv = getGlobalSymbolTable();
    StatmList* blockStam = new StatmList();
    setCurrentSymbolTable(blockEnv);
    declVar("Integer", name);
    DeclarationPart(blockStam);
    Statm* result = StatmentPart(blockStam);
    return new BlockNode(name, blockEnv, result);
}

void DeclarationPart(StatmList* block) {
    if(Symb.type == kwCONST){
        Compare(kwCONST);
        ConstDeclarationPart();
    }
    if(Symb.type == kwVAR){
        Compare(kwVAR);
        VarDeclarationPart(block);
    }
    if(Symb.type == kwBEGIN)
        return;
    else
        DeclarationPart(block);
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

void VarDeclarationPart(StatmList* block) {
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
   }else if(Symb.type == kwARRAY) {
        Symb = readLexem();
        Compare(LBRA);
        int sign;
        if(Symb.type == MINUS){
            Symb = readLexem();
            sign = -1;
        } else sign = 1;
        int start = Compare(INTEGER).value * sign;
        Compare(DOT);Compare(DOT);
        if(Symb.type == MINUS){
            Symb = readLexem();
            sign = -1;
        } else sign = 1;
        int fin = Compare(INTEGER).value * sign;
        Compare(RBRA);
        Compare(kwOF);
        if(Symb.type == kwINTEGER){
            Compare(kwINTEGER);
            for(auto it = names.begin(); it < names.end(); it++)
                declArrayVar("Integer", *it, (fin-start)+1, start);
        } else{}
   } else {}
   if(Symb.type == EQ){
        Symb = readLexem();
        Expr* init = Expression();
        for(auto it = names.begin(); it < names.end(); it++){
            block->add(new Assign(new Var(*it, false), init));
        }
   }
   Compare(SEMICOLON);
   VarDeclarationPart(block);
}

StatmList* StatmentPart(StatmList* init) {
    /* StatmentPart ->begin StatmentSequence end.*/
    Compare(kwBEGIN);
    StatmList* n = StatmentSequence(init);
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
                thenPart = StatmentPart(nullptr);
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
                body = StatmentPart(nullptr);
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
            switch(Symb.type){
                    case LBRA:{
                        Symb = readLexem();
                        Expr* index = Expression();
                        Compare(RBRA);
                        VarInArray* v = ArrayAccess(id, index, false);
                        Compare(ASSIGN);
                        result = new Assign(v, Expression());
                        break; 
                    }
                    default:
                        Compare(ASSIGN);
                        result = new Assign(new Var(id, false), Expression());
                        break;
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
    StatmList* block = new StatmList();
    Symb = readLexem();
    std::string name = Compare(IDENT).ident;
    Compare(ASSIGN);
    Expr* init = Expression();
    Statm* initstat = new Assign(new Var(name, false), init);
    block->add(initstat);
    int step = Symb.type == kwTO ? 1 : -1;
    Symb = readLexem();
    Expr* fin = Expression();
    BinOp* cond = new BinOp(Neq, new BinOp(Sub, new Var(name, true), new IntConst(step)), fin);
    Compare(kwDO);
    StatmList* body = new StatmList();
    if(Symb.type == kwBEGIN){
        Symb = readLexem();
        body = StatmentSequence(body);
        Compare(kwEND);
        if(Symb.type == SEMICOLON)
            Symb = readLexem();
    } else 
        body->add(Statment());
    body->add(new Assign(new Var(name, false), new BinOp(Add, new Var(name, true), new IntConst(step))));
    block->add(new While(cond, body));
    return block;
}

Statm* ElsePart() {
    Statm* elsePart = nullptr;
    if (Symb.type == kwELSE) {
        Symb = readLexem();
        if(Symb.type == kwBEGIN)
            elsePart = StatmentPart(nullptr);
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
            switch(Symb.type){
                case LBRA:{
                    Symb = readLexem();
                    Expr* index = Expression();
                    Compare(RBRA);
                    return ArrayAccess(id, index, true); 
                }
                default: return VarOrConst(id);
            }
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
