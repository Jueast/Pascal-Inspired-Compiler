#include "ast.h"
#include <new>
#include <vector>
#include <iostream>

int IntConst::Val() {
    return val;
}
void IntConst::Translate(){
    std::cout << val;
}
void BinOp::Translate(){
    std::cout << '(';
    left->Translate();
    std::cout << ' ' << op << ' ';
    right->Translate();
    std::cout << ')';
}
BinOp::BinOp(char op, Expr* l, Expr* r) {
    this->op = op;
    left = l;
    right = r;
}

BinOp::~BinOp() {
    delete left;
    delete right;
}
void UnMinus::Translate() {
    std::cout << "(";
    std::cout << "-";
    expr->Translate();
    std::cout << ")";
}
UnMinus::UnMinus(Expr *e) {
    expr = e;
}

UnMinus::~UnMinus() {
    delete expr;
}

void Write::Translate() {
    std::cout << "Write: ";
    expr->Translate();
    std::cout << std::endl;
}
Write::Write(Expr *e) {
    expr = e;
}

Write::~Write() {
    delete expr;
}

StatmList::StatmList() {

}

StatmList::~StatmList() {
    for (auto iter=statm_list.begin(); iter != statm_list.end(); iter++){
        delete (*iter);
    }
}

void StatmList::add(Statm* s){
    statm_list.push_back(s);
}

std::vector<Statm*> StatmList::get(){
    return statm_list;
}

void StatmList::Translate(){
    for (auto iter=statm_list.begin(); iter != statm_list.end(); iter++){
        (*iter)->Translate();
    }
}
