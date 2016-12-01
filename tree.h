#include <types.h> 
class ExprAST {
public:
    virtual ~ExprAST(){}
};

class IntegerExprAST : public ExprAST {
    INT Val;

public:
    IntegerExprAST(INT Val) : Val(Val);
}

