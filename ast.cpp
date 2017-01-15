#include "ast.h"
#include "tabsym.h"
#include <new>
#include <vector>
#include <string>
#include <iostream>
const std::string OpNames[] = 
{
    "+", "-", "*", "/", "mod", "<", ">", "=", "<>", "<=", ">=", "err"
};
Var::Var(std::string n, bool rv){
    name = n; rvalue = rv;
}

void Var::Translate(int i){
    std::cout << name;
}

int IntConst::Val() {
    return val;
}
void IntConst::Translate(int i){
    std::cout << val;
}
void BinOp::Translate(int i){
    std::cout << '(';
    left->Translate(i);
    std::cout << ' ' << OpNames[op] << ' ';
    right->Translate(i);
    std::cout << ')';
}
BinOp::BinOp(Op op, Expr* l, Expr* r) {
    this->op = op;
    left = l;
    right = r;
}

BinOp::~BinOp() {
    delete left;
    delete right;
}
void UnMinus::Translate(int i) {
    std::cout << "(";
    std::cout << "-";
    expr->Translate(i);
    std::cout << ")";
}
UnMinus::UnMinus(Expr *e) {
    expr = e;
}

UnMinus::~UnMinus() {
    delete expr;
}
void Assign::Translate(int i) {
    for(int j =0; i != j; j++)
            std::cout << "    ";
    if (checkSymbolType(var->name, NULL) != VarId) {
        std::cout << "Erro Assign to ";
        var->Translate(i);
        std::cout << std::endl;
        return;
    }
    std::cout << "Assign: ";
    var->Translate(i);
    std::cout << " = ";
    expr->Translate(i);
    std::cout << std::endl;
}

Assign::Assign(Var* v, Expr* e){
    var = v; expr = e;
}

Assign::~Assign(){
    delete(var);
    delete(expr);
}

void Write::Translate(int i) {
    for(int j =0; i != j; j++)
        std::cout << "    ";
    std::cout << "Write: ";
    expr->Translate(i);
    std::cout << std::endl;
}
Write::Write(Expr *e) {
    expr = e;
}

Write::~Write() {
    delete expr;
}
void Read::Translate(int i) {
    for(int j =0; i != j; j++)
        std::cout << "    ";
    std::cout << "Read: ";
    var->Translate(i);
    std::cout  << " from stdin." << std::endl;
}
Read::Read(Var *v) {
    var = v;
}

Read::~Read() {
    delete var;
}

If::If(Expr *c, Statm *ts, Statm *es)
{ cond = c; thenstm = ts; elsestm = es; }  

If::~If()
{ delete cond; delete thenstm; delete elsestm; }
void If::Translate(int i){
    for(int j = 0; i != j;j++)
        std::cout << "    ";
    std::cout << "IF ";
    cond->Translate(i);
    std::cout << " THEN:" << std::endl;
    thenstm->Translate(i+1);
    if(elsestm){
        std::cout<<"ELSE: " << std::endl;
        elsestm->Translate(i+1);
    }
}
While::While(Expr *c, Statm *b)
{ cond = c; body = b; }

While::~While()
{ delete body; delete cond;}
void While::Translate(int i){
    for(int j = 0; i != j; j++)
        std::cout << "    ";
    std::cout << "WHILE ";
    cond->Translate(i);
    std::cout << " DO:" << std::endl;
    body->Translate(i+1);
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

void StatmList::Translate(int i){
    for (auto iter=statm_list.begin(); iter != statm_list.end(); iter++){
        (*iter)->Translate(i);
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
