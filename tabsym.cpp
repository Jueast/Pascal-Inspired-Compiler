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
        default:
            stype = "Undefined";break;
    }
    std::cout << stype << " " << var.type 
              << " " << var.name << ": " 
              << value.integer << std::endl;
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
    TabElement t(Const, var, vval);
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
    TabElement t(Const, var, vval);
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
    TabElement t(VarId, var, vval);
    (CurrentSymbolTable->table)[name] = t;
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

std::vector<Variable> VarNames(SymbolTableMap* SymbolTable){ 
    std::vector<Variable> vars;
    for(auto it = SymbolTable->table.begin(); 
        it != SymbolTable->table.end(); it++){
        if(it->second.symbol_type == VarId){
            vars.push_back(it->second.var);
        }
    
    }
    return vars;
}
