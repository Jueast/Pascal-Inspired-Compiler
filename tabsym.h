#include <string>
#include <vector>
typedef struct Variable{
    std::string name;
    std::string type;
} Variable;
union VariableValue {
    int integer;
    double floating;
};
enum SymbolType {Undef, Var, Const, Func};
void declConstInt(std::string, int);
void declConstFloat(std::string, float);
void declVar(std::string, std::string);
void declFunc(std::string FnName, std::string type, std::vector<Variable> Args);



