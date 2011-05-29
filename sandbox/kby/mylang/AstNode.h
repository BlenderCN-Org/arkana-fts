#include "main.h"

#include <iostream>
#include <vector>
#include "llvm/Value.h"

namespace AST {
class CodeGenContext;
class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;

class Node {
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(CodeGenContext& context) = 0 ;
};

class Expression : public Node {
public:
    virtual ~Expression() {}
};

class Statement : public Node {
public:
    virtual ~Statement() {}
};

class Integer : public Expression {
    long long value;
public:
    Integer(long long value) : value(value) {}
    virtual ~Integer() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Double : public Expression {
    double value;
public:
    Double(double value) : value(value) {}
    virtual ~Double() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class String : public Expression {
    std::string value;
public:
    String(const std::string& value) : value(value) {}
    virtual ~String() { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Boolean : public Expression {
    std::string value;
    int boolVal;
public:
    Boolean(const std::string& value) : value(value)
    {
        boolVal = value == "#t" ? 1 : 0;
    }
    virtual ~Boolean() {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Identifier : public Expression {
    std::string name;
public:
    Identifier(const std::string& name) : name(name) {}
    virtual ~Identifier() {}
    std::string getName() const {return name;} 
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class MethodCall : public Expression {
    Identifier* id;
    ExpressionList* arguments;
public:
    MethodCall(Identifier* id, ExpressionList* arguments) : id(id), arguments(arguments) {}
    MethodCall(Identifier* id) : id(id), arguments(nullptr) {}
    virtual ~MethodCall()
    {
        for(ExpressionList::iterator i = arguments->begin() ; i != arguments->end() ; ++i) {
            delete *i;
        }
        arguments->clear();
        delete arguments;
        delete id;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class UnaryOperator : public Expression {
    int op;
    Expression* rhs;
public:
    UnaryOperator(int op, Expression* rhs) : rhs(rhs), op(op) {}
    virtual ~UnaryOperator()
    {
        delete rhs;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

// Binary operators are +,-,*,/
class BinaryOp : public Expression {
    int op;
    Expression* lhs;
    Expression* rhs;
public:
    BinaryOp(Expression* lhs, int op, Expression* rhs) : lhs(lhs), rhs(rhs), op(op) {}
    virtual ~BinaryOp()
    {
        delete lhs;
        delete rhs;
    }
    int getOperator() const {return op;}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class CompOperator : public Expression {
    int op;
    Expression* lhs;
    Expression* rhs;
public:
    CompOperator(Expression* lhs, int op, Expression* rhs) : lhs(lhs), rhs(rhs), op(op) {}
    virtual ~CompOperator()
    {
        delete lhs;
        delete rhs;
    }
    int getOperator() const {return op;}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Assignment : public Expression {
    Identifier* lhs;
    Expression* rhs;
public:
    Assignment(Identifier* lhs, Expression* rhs) : lhs(lhs), rhs(rhs) {}
    virtual ~Assignment() {delete lhs; delete rhs; }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Block : public Expression {
public:
    StatementList statements;
    Block() {}
    virtual ~Block()
    {
        for(StatementList::iterator i = statements.begin() ; i != statements.end() ; ++i) {
            delete *i;
        }
        statements.clear();
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class ExpressionStatement : public Statement
 {
    Expression* expression;
public:
    ExpressionStatement(Expression* expression) : expression(expression) {}
    virtual ~ExpressionStatement() { delete expression; }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class VariableDeclaration : public Statement
 {
    Identifier* type;
    Identifier* id;
    Expression *assignmentExpr;
public:
    VariableDeclaration(Identifier* type, Identifier* id) : type(type), id(id), assignmentExpr(nullptr) {}
    VariableDeclaration(Identifier* type, Identifier* id, Expression *assignmentExpr) : type(type), id(id), assignmentExpr(assignmentExpr) {}
    virtual ~VariableDeclaration()
    {
        delete assignmentExpr;
        delete id;
        delete type;
    }
    const Identifier& getIdentifierOfVariablenType() const {return *type;}
    std::string getVariablenName() const {return id->getName();}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class FunctionDeclaration : public Statement
 {
    Identifier* type;
    Identifier* id;
    VariableList* arguments;
    Block* block;
public:
    FunctionDeclaration(Identifier* type, Identifier* id, VariableList* arguments, Block* block)
    : type(type), id(id), arguments(arguments), block(block) {}
    FunctionDeclaration(Identifier* id, VariableList* arguments, Block* block)
    : type(new Identifier("void")), id(id), arguments(arguments), block(block) {}
    virtual ~FunctionDeclaration()
    {
        for(VariableList::iterator i = arguments->begin() ; i != arguments->end() ; ++i) {
            delete *i;
        }
        delete type;
        delete id;
        delete arguments;
        delete block;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Conditional : public Statement
 {
    CompOperator* cmpOp;
    Expression *thenExpr;
    Expression *elseExpr;
public:
    Conditional(Expression* op, Expression *thenExpr,Expression *elseExpr = nullptr)
    : cmpOp((CompOperator*)op) , thenExpr(thenExpr), elseExpr(elseExpr) { }
    virtual ~Conditional()
    {
        delete cmpOp; delete thenExpr; delete elseExpr;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class WhileLoop : public Statement
 {
    Expression* condition;
    Block *loopBlock;
    Block *elseBlock;
public:
    WhileLoop(Expression* expr, Block *loopBlock,Block *elseBlock = nullptr)
    : condition(expr) , loopBlock(loopBlock), elseBlock(elseBlock) {}
    virtual ~WhileLoop()
    {
        delete condition; delete loopBlock; delete elseBlock;
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};


class Return : public Statement
 {
    Expression* retExpr;
public:
    Return(Expression* expr = NULL) : retExpr(expr) {}
    virtual ~Return() {delete retExpr;}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

};
