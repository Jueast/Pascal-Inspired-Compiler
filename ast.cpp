#include "ast.h"
#include <new>
#include <vector>


int IntConst::Val() {
    return val;
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

UnMinus::UnMinus(Expr *e) {
    expr = e;
}

UnMinus::~UnMinus() {
    delete expr;
}

Write::Write(Expr *e) {
    expr = e;
}

Write::~Write() {
    delete expr;
}

StatmList::StatmList() {}

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

