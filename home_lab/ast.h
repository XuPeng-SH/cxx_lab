#pragma once

#include <string>
#include <vector>

using namespace std;

class ExprAST {
public:
    virtual ~ExprAST() {}
};

class NumberExprAST : public ExprAST {
    double val_;
public:
    NumberExprAST(double val) : val_(val) {}
};

class VariableExprAST : public ExprAST {
    string name_;
public:
    VariableExprAST(const string& name) : name_(name) {}
};

class BinaryExprAST : public ExprAST {
    char op_;
    ExprAST* lh_;
    ExprAST* rh_;
public:
    BinaryExprAST(char op, ExprAST* lh, ExprAST* rh) :
        op_(op), lh_(lh), rh_(rh) {}
};

class CallExprAST : public ExprAST {
    string callee_;
    vector<ExprAST*> args_;
public:
    CallExprAST(const string& callee, const vector<ExprAST*>& args) : callee_(callee), args_(args) {}
};

class PrototypeExprAST : public ExprAST {
    string name_;
    vector<ExprAST*> args_;
public:
    PrototypeExprAST(const string& name, const vector<ExprAST*> args) :
        name_(name), args_(args) {}
};

class FunctionExprAST : public ExprAST {
    ExprAST* body_;
    PrototypeExprAST* proto_;
public:
    FunctionExprAST(PrototypeExprAST* proto, ExprAST* body) :
        proto_(proto), body_(body) {}
};

ExprAST* Error(const char* str);
PrototypeExprAST* ErrorP(const char* str);
FunctionExprAST* ErrorF(const char* str);
