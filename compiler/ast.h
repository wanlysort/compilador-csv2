#ifndef AST_H
#define AST_H

// =============================================================================
// ast.h — Árbol de Sintaxis Abstracta (AST) para Subconjunto C#
// =============================================================================

#include <list>
#include <ostream>
#include <string>
#include <vector>

class Visitor;
class TypeVisitor;

// Forward Declarations de los nuevos componentes estructurales
class TypeNode;
class VarNode;
class InitValueNode;
class LValueNode;
class Stm;
class VarDec;
class Body;
class FunDec;
class StructDec;
class CaseBlock;

// =============================================================================
// Enumeraciones de Operadores
// =============================================================================

enum BinaryOp
{
    PLUS_OP,
    MINUS_OP,
    MUL_OP,
    DIV_OP,
    GE_OP,
    LE_OP,
    LT_OP,
    GT_OP,
    EQ_OP,
    NE_OP,
    AND_OP,
    OR_OP
};

enum UnaryOp
{
    NOT_OP,
    ADDRESS_OF_OP // Para soportar &id en los factores
};

// =============================================================================
// Infraestructura de Tipos Complejos (Type ::= BaseType (*)*)
// =============================================================================

class TypeNode
{
public:
    std::string baseName;    // int, string, bool, o id de un struct
    std::string genericType; // Nombre del tipo si es genérico (ej: T en Lista<T>)
    int pointerLevel;        // Cuántos '*' tiene (0 = normal, 1 = ptr*, 2 = ptr**)

    TypeNode(const std::string &base, int ptrs = 0, const std::string &generic = "");
    int accept(Visitor *visitor);
    ~TypeNode() = default;
};

// =============================================================================
// Expresiones (Exp) y Componentes de Inicialización
// =============================================================================

class Exp
{
public:
    virtual int accept(Visitor *visitor) = 0;
    virtual ~Exp() = 0;
    static std::string binopToChar(BinaryOp op);
};

// ---- Inicializadores (InitValue ::= AExp | InitList) ----
class InitValueNode
{
public:
    Exp *expression;                        // nullptr si es un InitList ({...})
    std::vector<InitValueNode *> listItems; // Elementos si es un InitList, vacío si es Exp

    InitValueNode(Exp *exp);
    InitValueNode(std::vector<InitValueNode *> items);
    int accept(Visitor *visitor);
    ~InitValueNode();
};

// ---- Binarias ----
class BinaryExp : public Exp
{
public:
    Exp *left;
    Exp *right;
    BinaryOp op;
    BinaryExp(Exp *l, Exp *r, BinaryOp op);
    int accept(Visitor *visitor) override;
    ~BinaryExp();
};

// ---- Unitarias (!Exp o &id) ----
class UnaryExp : public Exp
{
public:
    Exp *operand;         // Para ! o LValue en Factor
    std::string targetId; // Específico si es un address-of (&id)
    UnaryOp op;

    UnaryExp(Exp *operand, UnaryOp op);
    UnaryExp(const std::string &id, UnaryOp op); // Sobrecarga para &id
    int accept(Visitor *v) override;
    ~UnaryExp();
};

// ---- Literales ----
class NumberExp : public Exp
{
public:
    int value;
    NumberExp(int v);
    int accept(Visitor *visitor) override;
    ~NumberExp();
};

class BoolExp : public Exp
{
public:
    bool value;
    BoolExp(bool v);
    int accept(Visitor *visitor) override;
    ~BoolExp();
};

class StringLiteralExp : public Exp
{
public:
    std::string value;
    StringLiteralExp(const std::string &v);
    int accept(Visitor *visitor) override;
    ~StringLiteralExp();
};

// ---- Funciones Anónimas (LambdaDec ::= ( [Params] ) => { Body }) ----
class LambdaDecExp : public Exp
{
public:
    std::vector<TypeNode *> paramTypes;
    std::vector<std::string> paramNames;
    Body *body;

    LambdaDecExp(std::vector<TypeNode *> pTypes, std::vector<std::string> pNames, Body *b);
    int accept(Visitor *visitor) override;
    ~LambdaDecExp();
};

class CallExp : public Exp
{
public:
    std::string functionName;
    std::vector<Exp *> args;

    CallExp(
        const std::string &fn,
        const std::vector<Exp *> &a);

    int accept(Visitor *v) override;
    ~CallExp();
};

// =============================================================================
// Infraestructura de Identificación y Acceso a Datos (LValue)
// =============================================================================

// Representa variables y arreglos multidimensionales (id [Num] [Num])
class VarNode
{
public:
    std::string id;
    std::vector<int> dimensions; // Lista de tamaños estáticos asignados ([10][10])
    InitValueNode *initializer;  // nullptr si no tiene asignación explícita inicial

    VarNode(const std::string &name, std::vector<int> dims = {}, InitValueNode *init = nullptr);
    int accept(Visitor *visitor);

    ~VarNode();
};

// Representa el resolvedor dinámico de memoria y miembros de estructuras
// LValue ::= (*)* id DimAccess (. id DimAccess)*
class LValueNode : public Exp // Hereda de Exp porque un LValue es un Factor directo
{
public:
    int dereferenceLevel;                        // Cuántos '*' anteponen al LValue
    std::vector<std::string> memberIds;          // Componentes jerárquicos (ej: ["grupo", "alumno", "nota"])
    std::vector<std::vector<Exp *>> dimAccesses; // Accesos dinámicos por nivel (ej: grupo[0][1] -> accesses[0] tiene un Exp*)

    LValueNode(int ptrs, std::vector<std::string> ids, std::vector<std::vector<Exp *>> accesses);
    int accept(Visitor *visitor) override;
    ~LValueNode();
};

// =============================================================================
// Sentencias (Stm) — Actualizado sin Switch ni Break
// =============================================================================

class Stm
{
public:
    virtual int accept(Visitor *visitor) = 0;
    virtual ~Stm() = 0;
};

class AssignStm : public Stm
{
public:
    LValueNode *target;
    Exp *e;
    AssignStm(LValueNode *target, Exp *e);
    int accept(Visitor *visitor) override;
    ~AssignStm();
};

class PrintfStm : public Stm
{
public:
    std::string format;
    Exp *e;
    PrintfStm(const std::string &fmt, Exp *expresion);
    int accept(Visitor *visitor) override;
    ~PrintfStm();
};

class CallStm : public Stm
{
public:
    CallExp *call;
    CallStm(CallExp *c);
    int accept(Visitor *visitor) override;
    ~CallStm();
};

class ReturnStm : public Stm
{
public:
    Exp *e;
    ReturnStm(Exp *expresion);
    int accept(Visitor *visitor) override;
    ~ReturnStm();
};

class IfStm : public Stm
{
public:
    std::vector<Exp *> conditions; // if, y los else if subsequentes
    std::vector<Body *> branches;  // Bloques asociados
    Body *elseBranch;              // nullptr si no hay else

    IfStm(std::vector<Exp *> conds, std::vector<Body *> brs, Body *els);
    int accept(Visitor *visitor) override;
    ~IfStm();
};

class WhileStm : public Stm
{
public:
    Exp *condition;
    Body *b;
    WhileStm(Exp *condition, Body *t);
    int accept(Visitor *visitor) override;
    ~WhileStm();
};

class DoWhileStm : public Stm
{
public:
    Body *body;
    Exp *condition;
    DoWhileStm(Body *body, Exp *condition);
    int accept(Visitor *v) override;
    ~DoWhileStm();
};

class ForStm : public Stm
{
public:
    TypeNode *initType;
    std::string initId;
    Exp *initExpr;
    Exp *condition;
    std::string updateId;
    bool isIncrement; // true para ++, false para --
    Body *body;

    ForStm(TypeNode *t, const std::string &id, Exp *init, Exp *cond, const std::string &upId, bool inc, Body *b);
    int accept(Visitor *v) override;
    ~ForStm();
};

// =============================================================================
// Estructuras de Declaración Global
// =============================================================================

class VarDec
{
public:
    TypeNode *type;
    std::vector<VarNode *> varList;
    VarDec(TypeNode *t, std::vector<VarNode *> list);
    int accept(Visitor *visitor);
    ~VarDec();
};

class Body
{
public:
    std::vector<VarDec *> declarations;
    std::vector<Stm *> statements;
    std::vector<bool> isStatementOrder; // Controla el orden mixto de ejecuciones

    Body() = default;
    int accept(Visitor *visitor);
    ~Body();
};

class FunDec
{
public:
    std::string genericTypeParam; // <id> opcional
    TypeNode *returnType;
    std::string nombre;
    std::vector<TypeNode *> paramTypes;
    std::vector<std::string> paramNames;
    Body *cuerpo;

    FunDec(const std::string &generic, TypeNode *rType, const std::string &name,
           std::vector<TypeNode *> pTypes, std::vector<std::string> pNames, Body *b);
    int accept(Visitor *visitor);
    ~FunDec();
};

class StructDec
{
public:
    std::string nombre;
    std::vector<TypeNode *> memberTypes;
    std::vector<std::string> memberNames;

    StructDec(const std::string &name, std::vector<TypeNode *> mTypes, std::vector<std::string> mNames);
    int accept(Visitor *visitor);
    ~StructDec();
};

class Program
{
public:
    std::vector<StructDec *> sdlist;
    std::vector<VarDec *> vdlist;
    std::vector<FunDec *> fdlist;

    Program() = default;
    int accept(Visitor *visitor);
    ~Program() = default;
};

#endif // AST_H