#include "tabsym.h"
#include <string>
#include <iostream>
#include <map>

struct TabElement {
    SymbolType symbol_type;
    Variable var;
    VariableValue value;
    TabElement(){};
    TabElement(SymbolType s, Variable var, VariableValue value) 
            : symbol_type(s), var(var), value(value){};
};


static std::map<std::string, TabElement> SymbolTable;

static void error(std::string id, std::string text) {
    std::cout << "identifier " << id << ": " << text << std::endl;
}

void freeSymbTab() {
    SymbolTable.clear();
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
    TabElement t(Var, var, vval);
    SymbolTable[name] = t;
}
