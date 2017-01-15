#include "ast.h"
#include "tabsym.h"
#include <new>
#include <vector>
#include <string>
#include <iostream>

Var::Var(std::string n, bool rv){
    name = n; rvalue = rv;
}

void Var::Translate(){
    std::cout << name;
}

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
void Assign::Translate() {
    std::cout << "Assign: ";
    var->Translate();
    std::cout << " = ";
    expr->Translate();
    std::cout << std::endl;
}

Assign::Assign(Var* v, Expr* e){
    var = v; expr = e;
}

Assign::~Assign(){
    delete(var);
    delete(expr);
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
void Read::Translate() {
    std::cout << "Read: ";
    var->Translate();
    std::cout  << " from stdin." << std::endl;
}
Read::Read(Var *v) {
    var = v;
}

Read::~Read() {
    delete var;
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

Expr* VarOrConst(std::string id){
    int v;
    SymbolType st = checkSymbolType(id, &v);
    switch(st){
        case VarId:
            return new Var(id, true);
        case Const:
            return new IntConst(v);
        case Func:
        default:
            return NULL;
    }
}
