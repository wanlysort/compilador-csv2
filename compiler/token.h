#ifndef TOKEN_H
#define TOKEN_H

// =============================================================================
// token.h — Definición de la clase Token y sus tipos para el Subconjunto C#
// =============================================================================
// Representa la unidad léxica mínima producida por el Scanner.
// Cada Token tiene un tipo (Type) y el texto literal del lexema.
// =============================================================================

#include <ostream>
#include <string>

class Token
{
public:
    // -------------------------------------------------------------------------
    // Tipos de token reconocidos por la gramática oficial
    // -------------------------------------------------------------------------
    enum Type
    {
        // Operadores aritméticos
        PLUS,  // +
        MINUS, // -
        MUL,   // * (También actúa como operador de desreferenciación en LValue y Type)
        DIV,   // /
        INC, // ++
        DEC, // --

        // Delimitadores y Puntuación
        LPAREN,   // (
        RPAREN,   // )
        LBRACE,   // {
        RBRACE,   // }
        LBRACKET, // [
        RBRACKET, // ]
        SEMICOL,  // ;
        COMA,     // ,
        DOT,      // .

        // Operadores de asignación y especiales
        ASSIGN,    // =
        AMPERSAND, // &
        ARROW,     // =>

        // Literales
        NUM,            // Número entero literal
        TRUE,           // true
        FALSE,          // false
        STRING_LITERAL, // "cadena de caracteres"

        // Identificadores
        ID, // Identificador de usuario (variables, funciones, structs, genéricos)

        // Palabras reservadas (Tipos de datos básicos)
        STRUCT,      // struct
        INT_TYPE,    // int
        STRING_TYPE, // string
        BOOL_TYPE,   // bool

        // Control de flujo
        IF,      // if
        ELSE,    // else
        WHILE,   // while
        DO,      // do
        FOR,     // for
        SWITCH,  // switch
        CASE,    // case
        DEFAULT, // default
        BREAK,   // break

        // Funciones y Salida
        PRINTF, // printf
        RETURN, // return

        // Operadores relacionales y lógicos
        LT_OP,  // <
        LE_OP,  // <=
        GT_OP,  // >
        GE_OP,  // >=
        EQ_OP,  // ==
        NE_OP,  // !=
        AND_OP, // &&
        OR_OP,  // ||
        NOT_OP, // !

        // Especiales
        ERR, // Carácter o lexema no reconocido (error léxico)
        END  // Fin de la entrada (EOF)
    };

    // -------------------------------------------------------------------------
    // Atributos públicos
    // -------------------------------------------------------------------------
    Type type;        // Tipo del token
    std::string text; // Texto literal del lexema

    // -------------------------------------------------------------------------
    // Constructores
    // -------------------------------------------------------------------------
    Token(Type type);
    Token(Type type, char c);
    Token(Type type, const std::string &source, int first, int last);

    // -------------------------------------------------------------------------
    // Nombre legible del tipo (útil para mensajes de error del Parser)
    // -------------------------------------------------------------------------
    static std::string typeName(Type t);

    // -------------------------------------------------------------------------
    // Operadores de salida (Depuración y Escritura en archivos)
    // -------------------------------------------------------------------------
    friend std::ostream &operator<<(std::ostream &outs, const Token &tok);
    friend std::ostream &operator<<(std::ostream &outs, const Token *tok);
};

#endif // TOKEN_H