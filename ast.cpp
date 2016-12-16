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


#ifndef LOCAL_SFE_TEST
extern IRBuilder<> builder;
extern Module* module;

void LogError(const char *Str) {
    std::cout << Str;
    exit(1);
}

Value* IntConst::codegen() {
    return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), val);
}

Value* BinOp::codegen() {
    Value* L = left->codegen();
    Value* R = right->codegen();

    switch (op) {
        case '+':
            return builder.CreateAdd(L, R, ".addtmp");
        case '-':
            return builder.CreateSub(L, R, ".subtmp");
        case '*':
            return builder.CreateMul(L, R, ".multmp");
        case '/':
            return builder.CreateSDiv(L, R, ".divtmp");
        default:
            LogError("Errors in BinOp code genearting! Wrong OP!\n ");
            return nullptr;
    }
}

Value* UnMinus::codegen() {
    Value* E = expr->codegen();
    return builder.CreateNeg(E, ".negtmp");
}

Value* Write::codegen() {
    Value* E = expr->codegen();
    Value* formatStr = builder.CreateGlobalStringPtr("value = %d\n");
    std::vector<llvm::Value *> values;
    values.push_back(formatStr);
    values.push_back(E);
    return builder.CreateCall(printFunc(), values);
}

Value* StatmList::codegen() {
    for(auto iter = statm_list.begin(); iter < statm_list.end(); iter++){
        (*iter)->codegen();
    }
    return nullptr;
}
#endif
