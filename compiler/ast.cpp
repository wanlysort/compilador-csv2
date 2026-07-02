// =============================================================================
// ast.cpp — Implementación de los nodos del AST para el Subconjunto C#
// =============================================================================

#include "ast.h"
#include "visitor.h"
#include <iostream>

// =============================================================================
// Exp y Operadores
// =============================================================================

Exp::~Exp() {}

std::string Exp::binopToChar(BinaryOp op)
{
    switch (op)
    {
    case PLUS_OP:
        return "+";
    case MINUS_OP:
        return "-";
    case MUL_OP:
        return "*";
    case DIV_OP:
        return "/";
    case LE_OP:
        return "<=";
    case LT_OP:
        return "<";
    case GT_OP:
        return ">";
    case GE_OP:
        return ">=";
    case EQ_OP:
        return "==";
    case NE_OP:
        return "!=";
    case AND_OP:
        return "&&";
    case OR_OP:
        return "||";
    default:
        return "?";
    }
}

// =============================================================================
// Infraestructura de Tipos Complejos
// =============================================================================

TypeNode::TypeNode(const std::string &base, int ptrs, const std::string &generic)
    : baseName(base), pointerLevel(ptrs), genericType(generic) {}

int TypeNode::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// =============================================================================
// Inicializadores e Items de Listas
// =============================================================================

InitValueNode::InitValueNode(Exp *exp) : expression(exp) {}

InitValueNode::InitValueNode(std::vector<InitValueNode *> items)
    : expression(nullptr), listItems(items) {}

InitValueNode::~InitValueNode()
{
    delete expression;
    for (auto item : listItems)
    {
        delete item;
    }
}

int InitValueNode::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// =============================================================================
// Expresiones Específicas
// =============================================================================

// ---- BinaryExp ----
BinaryExp::BinaryExp(Exp *l, Exp *r, BinaryOp o) : left(l), right(r), op(o) {}

BinaryExp::~BinaryExp()
{
    delete left;
    delete right;
}

int BinaryExp::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- UnaryExp ----
UnaryExp::UnaryExp(Exp *opnd, UnaryOp o) : operand(opnd), targetId(""), op(o) {}
UnaryExp::UnaryExp(const std::string &id, UnaryOp o) : operand(nullptr), targetId(id), op(o) {}

UnaryExp::~UnaryExp()
{
    delete operand;
}

int UnaryExp::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- NumberExp ----
NumberExp::NumberExp(int v) : value(v) {}
NumberExp::~NumberExp() {}

int NumberExp::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- BoolExp ----
bool value;
BoolExp::BoolExp(bool v) : value(v) {}
BoolExp::~BoolExp() {}

int BoolExp::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- StringLiteralExp ----
StringLiteralExp::StringLiteralExp(const std::string &v) : value(v) {}
StringLiteralExp::~StringLiteralExp() {}

int StringLiteralExp::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- LambdaDecExp ----
LambdaDecExp::LambdaDecExp(std::vector<TypeNode *> pTypes, std::vector<std::string> pNames, Body *b)
    : paramTypes(pTypes), paramNames(pNames), body(b) {}

LambdaDecExp::~LambdaDecExp()
{
    for (auto t : paramTypes)
        delete t;
    delete body;
}

int LambdaDecExp::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- CallExp ----
CallExp::CallExp(const std::string &fn, const std::vector<Exp *> &a)
    : functionName(fn), args(a) {}

CallExp::~CallExp()
{
    for (auto arg : args)
    {
        delete arg;
    }

    
}

int CallExp::accept(Visitor *visitor)
{
    return visitor->visit(this);
}
// =============================================================================
// Infraestructura de Identificación y Acceso a Datos (LValue)
// =============================================================================

VarNode::VarNode(const std::string &name, std::vector<int> dims, InitValueNode *init)
    : id(name), dimensions(dims), initializer(init) {}

VarNode::~VarNode()
{
    delete initializer;
}
int VarNode::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

LValueNode::LValueNode(int ptrs, std::vector<std::string> ids, std::vector<std::vector<Exp *>> accesses)
    : dereferenceLevel(ptrs), memberIds(ids), dimAccesses(accesses) {}

LValueNode::~LValueNode()
{
    for (auto &basicList : dimAccesses)
    {
        for (auto exp : basicList)
            delete exp;
    }
}

int LValueNode::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// =============================================================================
// Sentencias (Stm)
// =============================================================================

Stm::~Stm() {}

// ---- AssignStm ----
AssignStm::AssignStm(LValueNode *tgt, Exp *expresion) : target(tgt), e(expresion) {}
AssignStm::~AssignStm()
{
    delete target;
    delete e;
}
int AssignStm::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- PrintfStm ----
PrintfStm::PrintfStm(const std::string &fmt, Exp *expresion) : format(fmt), e(expresion) {}
PrintfStm::~PrintfStm()
{
    delete e;
}
int PrintfStm::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- ReturnStm ----
ReturnStm::ReturnStm(Exp *expresion) : e(expresion) {}
ReturnStm::~ReturnStm()
{
    delete e;
}
int ReturnStm::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- IfStm ----
IfStm::IfStm(std::vector<Exp *> conds, std::vector<Body *> brs, Body *els)
    : conditions(conds), branches(brs), elseBranch(els) {}

IfStm::~IfStm()
{
    for (auto c : conditions)
        delete c;
    for (auto b : branches)
        delete b;
    delete elseBranch;
}
int IfStm::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- WhileStm ----
WhileStm::WhileStm(Exp *c, Body *t) : condition(c), b(t) {}
WhileStm::~WhileStm()
{
    delete condition;
    delete b;
}
int WhileStm::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- DoWhileStm ----
DoWhileStm::DoWhileStm(Body *body, Exp *condition) : body(body), condition(condition) {}
DoWhileStm::~DoWhileStm()
{
    delete body;
    delete condition;
}
int DoWhileStm::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- ForStm ----
ForStm::ForStm(TypeNode *t, const std::string &id, Exp *init, Exp *cond, const std::string &upId, bool inc, Body *b)
    : initType(t), initId(id), initExpr(init), condition(cond), updateId(upId), isIncrement(inc), body(b) {}

ForStm::~ForStm()
{
    delete initType;
    delete initExpr;
    delete condition;
    delete body;
}
int ForStm::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// =============================================================================
// Estructuras de Declaración Global y Bloques
// =============================================================================

// ---- VarDec ----
VarDec::VarDec(TypeNode *t, std::vector<VarNode *> list) : type(t), varList(list) {}
VarDec::~VarDec()
{
    delete type;
    for (auto v : varList)
        delete v;
}
int VarDec::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- Body ----
Body::~Body()
{
    for (auto d : declarations)
        delete d;
    for (auto s : statements)
        delete s;
}
int Body::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- FunDec ----
FunDec::FunDec(const std::string &generic, TypeNode *rType, const std::string &name,
               std::vector<TypeNode *> pTypes, std::vector<std::string> pNames, Body *b)
    : genericTypeParam(generic), returnType(rType), nombre(name), paramTypes(pTypes), paramNames(pNames), cuerpo(b) {}

FunDec::~FunDec()
{
    delete returnType;
    for (auto t : paramTypes)
        delete t;
    delete cuerpo;
}
int FunDec::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- StructDec ----
StructDec::StructDec(const std::string &name, std::vector<TypeNode *> mTypes, std::vector<std::string> mNames)
    : nombre(name), memberTypes(mTypes), memberNames(mNames) {}

StructDec::~StructDec()
{
    for (auto t : memberTypes)
        delete t;
}
int StructDec::accept(Visitor *visitor)
{
    return visitor->visit(this);
}

// ---- Program ----
int Program::accept(Visitor *visitor)
{
    return visitor->visit(this);
}