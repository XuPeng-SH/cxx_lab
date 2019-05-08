#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NExpression*> ExpressionList;

class Node {
public:
    virtual ~Node() = default;
    virtual llvm::Value* codeGen(CodeGenContext& context) { return nullptr; }
};

class NStatement : public Node {};
class NExpression : public Node {};

class NInteger : public NExpression {
public:
    long long _value;
    NInteger(long long value_) : _value(value_) {}
    virtual llvm::Value* codeGen(CodeGenContext& context) override;
};

class NDouble : public NExpression {
public:
    double _value;
    NDouble(double value_) : _value(value_) {}
    virtual llvm::Value* codeGen(CodeGenContext& context) override;
};

class NIdentifier : public NExpression {
public:
    std::string _name;
    NIdentifier(const std::string& name_) : _name(name_) {}
    virtual llvm::Value* codeGen(CodeGenContext& context) override;
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& _id;
    ExpressionList _arguments;
    NMethodCall(const NIdentifier& id_, const ExpressionList& arguments_) :
        _id(id_), _argumets(arguments_) {}
    virtual llvm::Value* codeGen(CodeGenContext& context) override;
};

class NBinaryOperator : public NExpression {
public:
    int _op;
    NExpression& _lhs;
    NExpression& _rhs;
    NBinaryOperator(int op_, NExpression& lhs_, NExpression& rhs_) :
        op_(_op), _argumets(arguments_) {}
    virtual llvm::Value* codeGen(CodeGenContext& context) override;
};
