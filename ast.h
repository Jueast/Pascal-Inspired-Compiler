#ifndef LOCAL_SFE_TEST
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#endif
#include <vector>
#include <string>
#include "tabsym.h"
#ifndef _NODE_
#define _NODE_
enum Op {
    Add, Sub, Mult, Div,Mod,Lt, Gt, Eq, Neq, Lte, Gte,And, Or, Not, Error
};
class Node {
public:
    virtual Node* Optimize() {return this;}
    virtual void Translate(int i) = 0;
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen() = 0;
#endif
    virtual ~Node() {}
};

class Expr : public Node {
};

class Statm : public Node {
};

class BlockNode : public Node {
    SymbolTableMap* SymbolTable;
    Statm* statmList;
public:
    std::string name;
    BlockNode(std::string, SymbolTableMap*, Statm*);
    SymbolTableMap* getSymbolTable(){ return SymbolTable;}
    Statm* getStatmList() {return statmList;}
    virtual ~BlockNode();
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif
};
class FunctionNode : public Node {
    BlockNode* FuncEnvAndBody;
public:
    std::vector<Variable> FuncProto; // First one is the function type and name;
    std::string name;
    FunctionNode(std::string, std::vector<Variable>, BlockNode*);
    SymbolTableMap* getSymbolTable(){ return FuncEnvAndBody->getSymbolTable();}
    virtual ~FunctionNode();
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Function* decl();
    virtual llvm::Value* codegen(){return nullptr;}
    llvm::Value* codegen(llvm::Function* F);
#endif
};
class CallNode : public Statm, public Expr {
    std::vector<Expr*> args;
public:
    std::string name;
    CallNode(std::string, std::vector<Expr*>);
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif

}; 
class IntConst : public Expr {
    int val;
public:
    IntConst(int c) : val(c) {};
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif
    int Val();
};

class Var : public Expr {
public:
    bool rvalue;
    std::string name;
    Var(std::string, bool);
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif
};
class VarInArray : public Var {
public:
    Expr* index; 
    VarInArray(std::string, bool, Expr*);
    virtual void Translate(int i);
    virtual ~VarInArray();
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif
};

class BinOp : public Expr {
    Op op;
    Expr *left, *right;
public:
    BinOp(Op c, Expr* l, Expr* r);
    virtual ~BinOp();
  //  virtual Node *Optimize(){};
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif
    virtual void Translate(int i);
};

class UnMinus : public Expr {
    Expr *expr;
public:
    UnMinus(Expr* e);
    ~UnMinus();
   // Node *Optimize(){};
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif
    void Translate(int i);
};
class Break: public Statm {
public:
        virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
        virtual llvm::Value* codegen();
#endif
};
class Empty: public Statm {
    virtual void Translate(int i) {}
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif

};

class If : public Statm {
    Expr *cond;
    Statm *thenstm;
    Statm *elsestm;
public:
    If(Expr*, Statm*, Statm*);
    virtual ~If();
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif

};

class While : public Statm {
    Expr *cond;
    Statm *body;
public:
    While(Expr*, Statm*);
    virtual ~While();
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif

};


class Write : public Statm {
    Expr *expr;
public:
    Write(Expr* expr);
    virtual ~Write();
    //virtual Node *Optimize(){};
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();  
#endif 
};

class Read : public Statm {
    Var *var;
public:
    Read(Var* var);
    virtual ~Read();
    //virtual Node *Optimize(){};
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();  
#endif 
};


class Assign : public Statm {
    Var *var;
    Expr *expr;
public:
    Assign(Var*, Expr*);
    virtual ~Assign();
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();  
#endif 
};

class StatmList : public Statm {
    std::vector<Statm*> statm_list;
public:
    StatmList();
    ~StatmList();
    /* Statments Queue */
    void add(Statm* s);
    std::vector<Statm*> get();
    virtual void Translate(int i);
#ifndef LOCAL_SFE_TEST
    virtual llvm::Value* codegen();
#endif
};

Expr* VarOrConst(std::string id);
VarInArray* ArrayAccess(std::string, Expr*, bool);
CallNode* CallFunc(std::string, std::vector<Expr*>);

#endif
