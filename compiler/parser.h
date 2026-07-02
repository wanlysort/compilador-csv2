#ifndef PARSER_H
#define PARSER_H

// =============================================================================
// parser.h — Definición del Parser (Analizador Sintáctico Descendente)
// =============================================================================
// Implementa un parser descendente recursivo (LL) con Lookahead que consume
// tokens del Scanner y construye el Árbol de Sintaxis Abstracta (AST) adaptado
// a la nueva gramática orientada a C#.
// =============================================================================

#include <string>
#include <vector>
#include "scanner.h"
#include "ast.h"
#include "token.h"

class Parser
{
private:
    Scanner *scanner; // Fuente de tokens
    Token *current;   // Token actual (lookahead)
    Token *previous;  // Token anterior (ya consumido)
    Token *next;

    // ---- Primitivas de consumo ----
    bool match(Token::Type ttype); // Consume el token si coincide
    bool check(Token::Type ttype); // Verifica sin consumir
    bool advance();                // Avanza al siguiente token
    bool isAtEnd();                // ¿Llegamos al final del archivo (END)?
    bool looksLikeVarDeclaration();

    // ---- Reporte de errores ----
    void error(const std::string &expected);

    void expect(Token::Type ttype);

    // ---- Métodos auxiliares de parsing interno estructural ----
    StructDec *parseStructDec();
    VarNode *parseVar();
    InitValueNode *parseInitValue();
    InitValueNode *parseInitList();
    TypeNode *parseType();
    LValueNode *parseLValue();
    std::vector<Exp *> parseDimAccess();

    Exp *parseAExp();   // Expresiones Lógicas (&&, ||)
    Exp *parseBExp();   // Negación Lógica (!) con agrupamiento obligado
    Exp *parseCExp();   // Comparaciones Relacionales (<, <=, ==, >, >=, !=)
    Exp *parseExp();    // Operaciones Aditivas (+, -)
    Exp *parseTerm();   // Operaciones Multiplicativas (*, /)
    Exp *parseFactor(); // Literales, Identificadores, Lambdas y Agrupamientos ( AExp )

public:
    Parser(Scanner *scanner);

    Program *parseProgram(); // Program ::= StructDecList VarDecList FunDecList
    FunDec *parseFunDec();   // FunDec  ::= [ < id > ] Type id ( [ParamDecList] ) { Body }
    Body *parseBody();       // Body    ::= (VarDec | Stmt)*
    VarDec *parseVarDec();   // VarDec  ::= Type VarList ;
    Stm *parseStm();         // Stmt    ::= Control de Flujos, Asignaciones, Retornos y Printf
};

#endif // PARSER_H