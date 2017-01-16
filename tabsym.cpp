#include "tabsym.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>
void TabElement::output(){
    std::string stype;
    switch(symbol_type){
        case VarId:
            stype = "Var";break;
        case Const:
            stype = "Const";break;
        case Func:
            stype = "Function";break;
        case ArrayVar:
            stype = "ArrayVar";break;
        default:
            stype = "Undefined";break;
    }
    std::cout << stype << " " << var.type 
              << " " << var.name << ": " 
              << (symbol_type == Const ? value.integer : 0) << std::endl;
}
static SymbolTableMap* CurrentSymbolTable = new SymbolTableMap();
static SymbolTableMap* GlobalSymbolTable = CurrentSymbolTable;
static void error(std::string id, std::string text) {
    std::cout << "identifier " << id << ": " << text << std::endl;
}
void setCurrentSymbolTable(SymbolTableMap *s){
    CurrentSymbolTable = s;
}
SymbolTableMap* getGlobalSymbolTable() {
    return GlobalSymbolTable;
}
SymbolTableMap* getCurrentSymbolTable() {
    return CurrentSymbolTable;
}
void freeSymbTab(SymbolTableMap* SymbolTable) {
    SymbolTable->table.clear();
}

void outputTab(SymbolTableMap* SymbolTable){
    if(!SymbolTable)
        SymbolTable = getCurrentSymbolTable();
    for(auto it = SymbolTable->table.begin(); it != SymbolTable->table.end(); it++){
        it->second.output();
    }
}

void declConstInt(std::string id, int val) {
    auto it = CurrentSymbolTable->table.find(id);
    if(it != CurrentSymbolTable->table.end()) {
        error(id, "is declared again.");
        return;
    }
    VariableValue vval;
    vval.integer = val;
    Variable var;
    var.type = "Integer";
    var.name = id;
    TabElement t(Const, var, vval, 1, 0);
    (CurrentSymbolTable->table)[id] = t;
}

void declConstFloat(std::string id, double val) {
    auto it = CurrentSymbolTable->table.find(id);
    if(it != CurrentSymbolTable->table.end()) {
        error(id, "is declared again.");
        return;
    }
    VariableValue vval;
    vval.floating = val;
    Variable var;
    var.type = "Float";
    var.name = id;
    TabElement t(Const, var, vval, 1, 0);
    (CurrentSymbolTable->table)[id] = t;
}

void declVar(std::string type, std::string name) {
    auto it = CurrentSymbolTable->table.find(name);
    if(it != CurrentSymbolTable->table.end()) {
        error(name, "is declared again.");
        return;
    }
    Variable var;
    var.type = type;
    var.name = name;
    VariableValue vval;
    TabElement t(VarId, var, vval, 1, 0);
    (CurrentSymbolTable->table)[name] = t;
}
void declArrayVar(std::string type, std::string name, int size, int bias){
    auto it = CurrentSymbolTable->table.find(name);
    if(it != CurrentSymbolTable->table.end()) {
        error(name, "is declared again.");
        return;
    }
    Variable var;
    var.type = type;
    var.name = name;
    VariableValue vval;
    TabElement t(ArrayVar, var, vval, size, bias);
    (CurrentSymbolTable->table)[name] = t;
}

void declFunc(std::string FnName, std::string type, void* FunNode){
    auto it = CurrentSymbolTable->table.find(FnName);
    if(it != CurrentSymbolTable->table.end()) {
        error(FnName, "is declared again.");
        return;
    }
    Variable var;
    var.type = type;
    var.name = FnName;
    VariableValue vval;
    vval.ptr = FunNode;
    TabElement t(Func, var, vval, 1, 0);
    (CurrentSymbolTable->table)[FnName] = t;
}

const TabElement* getTabElement(SymbolTableMap* st, std::string id){
    auto it = st->table.find(id);
    if(it != st->table.end()){
        return &(it->second); 
    }
    if(!st->parentTable)
        error(id, "is not declared");
    else
        return getTabElement(st->parentTable, id);
    return nullptr;
}
SymbolType checkSymbolType(SymbolTableMap* SymbolTable, std::string id, int* v){
   auto it = SymbolTable->table.find(id);
   if(it != SymbolTable->table.end()){
        if(v)
            *v = it->second.value.integer;
        return it->second.symbol_type;
   }
   if(!SymbolTable->parentTable)
        error(id, "is not declared.");
   else 
        return checkSymbolType(SymbolTable->parentTable, id, v);
   return Undef;

}

std::vector<TabElement> VarNames(SymbolTableMap* SymbolTable){ 
    std::vector<TabElement> vars;
    for(auto it = SymbolTable->table.begin(); 
        it != SymbolTable->table.end(); it++){
        if(it->second.symbol_type == VarId || it->second.symbol_type == ArrayVar){
            vars.push_back(it->second);
        }
    
    }
    return vars;
}
std::vector<TabElement> FunNames(SymbolTableMap* st){
    std::vector<TabElement> funcs;
    for(auto it = st->table.begin();
        it != st->table.end(); it++){
        if(it->second.symbol_type == Func)
                funcs.push_back(it->second);
    }
    return funcs;
}
