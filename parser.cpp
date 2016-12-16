#include "parser.h"
#include "tree.h"
#include "lexan.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

std::vector<Node*> Program(void);
void ProgramHead(void);
std::vector<Node*> Block(void);
void DeclarationPart(void);
std::vector<Node*> StatmentPart(void);
std::vector<Node*> StatmentSequence(void);
Node* Statment(void);
Node* Expression(void);
Node* ExpressionPrime(void);
Node* Term(void);
Node* TermPrime(Node*);
Node* Factor(void);

LexicalSymbol Symb;

void CompareError(LexSymbolType s) { 
    std::cout << "Error while comparing, expected %s." << symbTable[s] << std::endl;
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
    else{
        CompareError(s);
    }
}
std::vector<Node*> Program(void) {
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

std::vector<Node*> Block(void) {
    /* Block -> DeclarationPart StatmentPart */
    DeclarationPart();
    return StatmentPart();
}

void DeclarationPart() {
    return;
}

std::vector<Node*> StatmentPart(void) {
    /* StatmentPart ->begin StatmentSequence end.*/
    Compare(kwBEGIN);
    std::vector<Node*> n = StatmentSequence();
    Compare(kwEND);
    Compare(DOT);
    return n;
}
std::vector<Node*> StatmentSequence(void) {
    std::vector<Node*> statments_list;
    while(Symb.type != kwEND){
        statments_list.push_back(Statment());
    }
    return statments_list;
}

Node* Statment(void){
    Node* n = Expression();
    Compare(SEMICOLON);
    return n;
}
Node* Expression(void) {
    /* E -> T E' */
    std::cout << "E -> T E'" << std::endl;
    return ExpressionPrim(Term());
}

Node *ExpressionPrime(void) {
    switch (Symb.type) {
        case PLUS:
            /* E'-> + T E' */
            Symb = readLexem();
            printf(" E' -> + T E'\n");
            return ExpressionPrime(Term());
        case MINUS:
            /* E'-> - T E' */
            Symb = readLexem();
            printf(" E' -> - T E'\n");
            return ExpressionPrime(Term());
        case 

}

