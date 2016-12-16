/* tree.h */
#include <vector>
#ifndef _NODE_
#define _NODE_

class Node {
public:
    virtual Node* Optimize() {return this;}
    virtual void Translate() = 0;
    virtual ~Node() {}
};

class Expr : public Node {
};

class Statm : public Node {
};


class IntConst : public Expr {
    int val;
public:
    IntConst(int c) : val(c) {};
    virtual void Translate();
    int Val();
};

class BinOp : public Expr {
    char op;
    Expr *left, *right;
public:
    BinOp(char c, Expr* l, Expr* r);
    virtual ~BinOp();
  //  virtual Node *Optimize(){};
    virtual void Translate();
};

class UnMinus : public Expr {
    Expr *expr;
public:
    UnMinus(Expr* e);
    ~UnMinus();
   // Node *Optimize(){};
    void Translate();
};

class Write : public Statm {
    Expr *expr;
public:
    Write(Expr* expr);
    virtual ~Write();
    //virtual Node *Optimize(){};
    virtual void Translate();
};

class StatmList : public Node {
    std::vector<Statm*> statm_list;
public:
    StatmList();
    ~StatmList();
    /* Statments Queue */
    void add(Statm* s);
    std::vector<Statm*> get();
    virtual void Translate();

};
#endif
