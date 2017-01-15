#include "tabsym.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>
struct TabElement {
    SymbolType symbol_type;
    Variable var;
    VariableValue value;
    TabElement(){};
    TabElement(SymbolType s, Variable var, VariableValue value) 
            : symbol_type(s), var(var), value(value){};
    void output();
};

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

static std::map<std::string, TabElement> SymbolTable;

static void error(std::string id, std::string text) {
    std::cout << "identifier " << id << ": " << text << std::endl;
}

void freeSymbTab() {
    SymbolTable.clear();
}

void outputTab(){
    for(auto it = SymbolTable.begin(); it != SymbolTable.end(); it++){
        it->second.output();
    }
}

void declConstInt(std::string id, int val) {
    auto it = SymbolTable.find(id);
    if(it != SymbolTable.end()) {
        error(id, "is declared again.");
        return;
    }
    VariableValue vval;
    vval.integer = val;
    Variable var;
    var.type = "Integer";
    var.name = id;
    TabElement t(Const, var, vval);
    SymbolTable[id] = t;
}

void declConstFloat(std::string id, double val) {
    auto it = SymbolTable.find(id);
    if(it != SymbolTable.end()) {
        error(id, "is declared again.");
        return;
    }
    VariableValue vval;
    vval.floating = val;
    Variable var;
    var.type = "Float";
    var.name = id;
    TabElement t(Const, var, vval);
    SymbolTable[id] = t;
}

void declVar(std::string type, std::string name) {
    auto it = SymbolTable.find(name);
    if(it != SymbolTable.end()) {
        error(name, "is declared again.");
        return;
    }
    Variable var;
    var.type = type;
    var.name = name;
    VariableValue vval;
    TabElement t(VarId, var, vval);
    SymbolTable[name] = t;
}

SymbolType checkSymbolType(std::string id, int* v){
   auto it = SymbolTable.find(id);
   if(it != SymbolTable.end()){
        *v = it->second.value.integer;
        return it->second.symbol_type;
   }
   error(id, "is not declared.");
   return Undef;

}

std::vector<Variable> VarNames(void){ 
    std::vector<Variable> vars;
    for(auto it = SymbolTable.begin(); 
        it != SymbolTable.end(); it++){
        if(it->second.symbol_type == VarId){
            vars.push_back(it->second.var);
        }
    
    }
    return vars;
}
