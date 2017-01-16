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
FunctionNode* FunctionMatch();
std::vector<Variable> FunctionHead();
BlockNode* Block(std::vector<Variable>);
void DeclarationPart(StatmList*);
void ConstDeclarationPart(void);
void VarDeclarationPart(StatmList*);
StatmList* StatmentPart(StatmList*);
StatmList* StatmentSequence(StatmList* init);
Statm* Statment(void);
Statm* For(void);
Expr* LogicExpr(void);
Expr* LogicExprPrime(Expr*);
Expr* RelExpr();
Expr* RelExprPrime(Expr*);
Statm* ElsePart(void);
Op LogicOp(void);
Op RelOp(void);
Expr* Expression(void);
Expr* ExpressionPrime(Expr*);
Expr* Term(void);
Expr* TermPrime(Expr*);
Expr* Factor(void);

LexicalSymbol Symb;
static int t =  0;
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
BlockNode* Block(std::vector<Variable> names) {
    /* Block -> DeclarationPart StatmentPart */
    SymbolTableMap* blockEnv = new SymbolTableMap();
    blockEnv->parentTable = getCurrentSymbolTable();
    StatmList* blockStam = new StatmList();
    setCurrentSymbolTable(blockEnv);
    for(auto it = names.begin(); it != names.end(); it++)
        declVar("Integer", (*it).name);
    DeclarationPart(blockStam);
    Statm* result = StatmentPart(blockStam);
    setCurrentSymbolTable(blockEnv->parentTable);
    return new BlockNode(names[0].name,blockEnv, result);
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
    if(Symb.type == kwFUNCTION){
        FunctionMatch();
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
FunctionNode* FunctionMatch(void){
    std::vector<Variable> proto = FunctionHead();
    BlockNode* bn = Block(proto);
    FunctionNode* r = new FunctionNode(proto[0].name, proto, bn);
    declFunc(proto[0].name, proto[0].type, (void *)r, t++);
    return r;
}
std::vector<Variable> FunctionHead(void){
    Compare(kwFUNCTION);
    std::string name = Compare(IDENT).ident;
    Compare(LPAR);
    std::vector<Variable> proto;
    proto.push_back(Variable());
    proto[0].name = name;
    while (Symb.type != RPAR){
        Variable para;
        para.name = Compare(IDENT).ident;
        Compare(COLON);
        if(Symb.type == kwINTEGER)
            para.type = "Integer";
        else
            para.type = "Unknown";
        Symb = readLexem();
        if(Symb.type == SEMICOLON)
            Compare(SEMICOLON);
        proto.push_back(para);
    }
    Compare(RPAR);Compare(COLON);
    if(Symb.type == kwINTEGER)
            proto[0].type = "Integer";
        else
            proto[0].type = "Unknown";
    Symb = readLexem();
    Compare(SEMICOLON);
    return proto;
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
            Expr *cond = LogicExpr();
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
            cond = LogicExpr();
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
                    case LPAR:{
                        Symb = readLexem();
                        std::vector<Expr*> args;
                        while (Symb.type != RPAR){
                            args.push_back(Expression());
                        }
                        Compare(RPAR);
                        result = CallFunc(id, args);
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
Expr *LogicExpr() {
    return LogicExprPrime(RelExpr());
}
Expr *LogicExprPrime(Expr* b){
    Op o;
    if((o = LogicOp()) != Error){
        return LogicExprPrime(new BinOp(o, b, RelExpr()));
    }
    else
        return b;
}
Expr *RelExpr() {
    return RelExprPrime(Expression());
}
Expr *RelExprPrime(Expr* b){
    Op o;
    if((o = RelOp()) != Error){
        return RelExprPrime(new BinOp(o, b, Expression()));
    }
    return b;
}
Op LogicOp(){
    switch(Symb.type){
        case kwAND:
            Symb = readLexem();
            return And;
        case kwOR:
            Symb = readLexem();
            return Or;
        case kwNOT:
            Symb = readLexem();
            return Not;
        default:
            return Error;
    }
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
                case LPAR:{
                    Symb = readLexem();
                        std::vector<Expr*> args;
                        while (Symb.type != RPAR){
                            args.push_back(Expression());
                            if(Symb.type == COMMA)
                                Symb = readLexem();
                        }
                        Compare(RPAR);
                        return CallFunc(id, args);
           
                }
                default: return VarOrConst(id);
            }
        }
        case LPAR:{ 
            Symb = readLexem();
            Expr* e = LogicExpr();
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
