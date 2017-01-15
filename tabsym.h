#include <string>
#include <vector>
#include <map>
#ifndef __SYMBOL_TABLE__
#define __SYMBOL_TABLE__
typedef struct Variable{
    std::string name;
    std::string type;
} Variable;
union VariableValue {
    int integer;
    double floating;
};

enum SymbolType {Undef, VarId, Const, Func, ArrayVar};
struct TabElement {
    SymbolType symbol_type;
    Variable var;
    VariableValue value;
    int size;
    int bias;
    TabElement(){};
    TabElement(SymbolType s, Variable var, VariableValue value, int size, int b) 
            : symbol_type(s), var(var), value(value), size(size), bias(b){};
    void output();
};

typedef struct tableStruct{ 
    std::map<std::string, TabElement> table;
    struct tableStruct* parentTable;
    tableStruct() : parentTable(nullptr){}
} SymbolTableMap;
#endif
void freeSymbTab(SymbolTableMap*);
void outputTab(SymbolTableMap*);
void setCurrentSymbolTable(SymbolTableMap*);
SymbolTableMap* getGlobalSymbolTable();
SymbolTableMap* getCurrentSymbolTable();
void declConstInt(std::string, int);
void declConstFloat(std::string, float);
void declVar(std::string, std::string);
void declArrayVar(std::string, std::string, int size, int bias);
void declFunc(std::string FnName, std::string type, std::vector<Variable> Args);
SymbolType checkSymbolType(SymbolTableMap*, std::string, int* v);
const TabElement* getTabElement(SymbolTableMap*, std::string);
std::vector<TabElement> VarNames(SymbolTableMap*);


