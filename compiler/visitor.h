#ifndef VISITOR_H
#define VISITOR_H

// =============================================================================
// visitor.h — Definición de los Visitantes del AST (TypeChecker y GenCode)
// =============================================================================
// Implementa el patrón de diseño Visitor sobre el Árbol de Sintaxis Abstracta
// orientado a subconjuntos de C#.
// =============================================================================

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include "environment.h"

// ---- Forward Declarations del AST ----
class Exp;
class Program;
class StructDec;
class FunDec;
class VarDec;
class VarNode;
class InitValueNode;
class TypeNode;
class Body;
class LValueNode;

// Sentencias
class AssignStm;
class PrintfStm;
class CallStm;
class ReturnStm;
class IfStm;
class WhileStm;
class DoWhileStm;
class ForStm;

// Expresiones
class BinaryExp;
class UnaryExp;
class NumberExp;
class BoolExp;
class StringLiteralExp;
class LambdaDecExp;

// =============================================================================
// Clase Base Abstracta: Visitor
// =============================================================================
class Visitor
{
public:
    virtual ~Visitor() = default;

    // Nodos Estructurales y de Tipado
    virtual int visit(Program *p) = 0;
    virtual int visit(StructDec *sd) = 0;
    virtual int visit(FunDec *fd) = 0;
    virtual int visit(VarDec *vd) = 0;
    virtual int visit(VarNode *vn) = 0;
    virtual int visit(InitValueNode *ivn) = 0;
    virtual int visit(TypeNode *tn) = 0;
    virtual int visit(Body *body) = 0;
    virtual int visit(LValueNode *lval) = 0;

    // Sentencias
    virtual int visit(AssignStm *stm) = 0;
    virtual int visit(PrintfStm *stm) = 0;
    virtual int visit(CallStm *stm) = 0;
    virtual int visit(ReturnStm *stm) = 0;
    virtual int visit(IfStm *stm) = 0;
    virtual int visit(WhileStm *stm) = 0;
    virtual int visit(DoWhileStm *stm) = 0;
    virtual int visit(ForStm *stm) = 0;

    // Expresiones
    virtual int visit(BinaryExp *exp) = 0;
    virtual int visit(UnaryExp *exp) = 0;
    virtual int visit(NumberExp *exp) = 0;
    virtual int visit(BoolExp *exp) = 0;
    virtual int visit(StringLiteralExp *exp) = 0;
    virtual int visit(LambdaDecExp *lambda) = 0;
    virtual int visit(CallExp *exp) = 0;
};

// =============================================================================
// TypeCheckerVisitor — Análisis Semántico y Verificación de Ámbitos
// =============================================================================
class TypeCheckerVisitor : public Visitor
{
public:
    // Estructuras de datos auxiliares para el análisis
    std::unordered_map<std::string, int> funcontador; // función -> espacio en frame (parámetros + locales)
    std::unordered_map<std::string, int> funAridad;    // función -> número de argumentos esperados
    std::unordered_map<std::string, StructDec*> structsDefinidos; // nombre -> puntero al nodo del struct

    int locales = 0;                   // Contador acumulado de variables locales en el scope actual
    Environment<int> entorno;          // Tabla de símbolos con soporte para múltiples niveles (Scopes)
    std::string funcionActual;         // Metadato del contexto actual para trazas de error

    // Punto de entrada del Analizador Semántico
    int TypeChecker(Program *program);

    // Reescrituras obligatorias de la Interfaz (Análisis Léxico/Semántico)
    int visit(Program *p) override;
    int visit(StructDec *sd) override;
    int visit(FunDec *fd) override;
    int visit(VarDec *vd) override;
    int visit(VarNode *vn) override;
    int visit(InitValueNode *ivn) override;
    int visit(TypeNode *tn) override;
    int visit(Body *body) override;
    int visit(LValueNode *lval) override;

    int visit(AssignStm *stm) override;
    int visit(PrintfStm *stm) override;
    int visit(CallStm *stm) override;
    int visit(ReturnStm *stm) override;
    int visit(IfStm *stm) override;
    int visit(WhileStm *stm) override;
    int visit(DoWhileStm *stm) override;
    int visit(ForStm *stm) override;

    int visit(BinaryExp *exp) override;
    int visit(UnaryExp *exp) override;
    int visit(NumberExp *exp) override;
    int visit(BoolExp *exp) override;
    int visit(StringLiteralExp *exp) override;
    int visit(LambdaDecExp *lambda) override;
    int visit(CallExp *exp) override;
};

// =============================================================================
// GenCodeVisitor — Generador de Código Máquina x86-64 (Sintaxis AT&T)
// =============================================================================
class GenCodeVisitor : public Visitor
{
private:
    std::ostream &out; // Stream de salida de datos (Archivo objetivo .s)

public:
    TypeCheckerVisitor tipos; // Instancia interna del analizador para extraer metadata

    std::unordered_map<std::string, int> funcontador;   // Copia local de los tamaños de frame calculados
    std::unordered_map<std::string, int> memoria;       // Variable local -> Offset de bytes relativos a (%rbp)
    std::unordered_map<std::string, bool> memoriaGlobal; // Set dinámico de variables alojadas en .data
    std::unordered_set<std::string> stringVars;          // Variables declaradas con tipo string

    int offset = -8;             // Próximo slot disponible en bytes para variables de 64 bits
    int labelcont = 0;           // Generador incremental de etiquetas únicas para saltos condicionales
    int suReorders = 0;          // Contador de reordenamientos Sethi-Ullman aplicados
    bool entornoFuncion = false; // Flag de contexto para decidir direccionamiento (%rip) vs (%rbp)
    std::string nombreFuncion;   // Identificador de la rutina en procesamiento para los epílogos

    // Constructor vinculando el stream de salida
    GenCodeVisitor(std::ostream &out) : out(out) {}

    // Helper — emite código que deja la DIRECCIÓN del lvalue en %rax (sin deref)
    void genLValueAddr(LValueNode *lval);

    // Sethi-Ullman — calcula cuántos registros necesita un subárbol (peso)
    int sethiUllmanLabel(Exp *exp);

    // Punto de entrada para la fase de Síntesis
    int generar(Program *program);

    // Reescrituras obligatorias para la generación de código AT&T x86-64
    int visit(Program *p) override;
    int visit(StructDec *sd) override;
    int visit(FunDec *fd) override;
    int visit(VarDec *vd) override;
    int visit(VarNode *vn) override;
    int visit(InitValueNode *ivn) override;
    int visit(TypeNode *tn) override;
    int visit(Body *body) override;
    int visit(LValueNode *lval) override;

    int visit(AssignStm *stm) override;
    int visit(PrintfStm *stm) override;
    int visit(CallStm *stm) override;
    int visit(ReturnStm *stm) override;
    int visit(IfStm *stm) override;
    int visit(WhileStm *stm) override;
    int visit(DoWhileStm *stm) override;
    int visit(ForStm *stm) override;

    int visit(BinaryExp *exp) override;
    int visit(UnaryExp *exp) override;
    int visit(NumberExp *exp) override;
    int visit(BoolExp *exp) override;
    int visit(StringLiteralExp *exp) override;
    int visit(LambdaDecExp *lambda) override;
    int visit(CallExp *exp) override;
};

#endif // VISITOR_H