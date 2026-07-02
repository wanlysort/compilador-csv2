// =============================================================================
// token.cpp — Implementación de la clase Token para el Subconjunto C#
// =============================================================================

#include "token.h"

// -----------------------------------------------------------------------------
// Constructores
// -----------------------------------------------------------------------------

Token::Token(Type type)
    : type(type), text("") {}

Token::Token(Type type, char c)
    : type(type), text(std::string(1, c)) {}

Token::Token(Type type, const std::string& source, int first, int last)
    : type(type), text(source.substr(first, last)) {}

// -----------------------------------------------------------------------------
// typeName — Devuelve el nombre legible de un tipo de token
// Ampliamente usado por el Parser para emitir mensajes de error claros.
// -----------------------------------------------------------------------------

std::string Token::typeName(Type t) {
    switch (t) {
        // Operadores aritméticos
        case PLUS:      return "'+'";
        case MINUS:     return "'-'";
        case MUL:       return "'*'";
        case DIV:       return "'/'";
        case INC:       return "'++'";
        case DEC:       return "'--'";
        
        // Delimitadores y Puntuación
        case LPAREN:    return "'('";
        case RPAREN:    return "')'";
        case LBRACE:    return "'{'";
        case RBRACE:    return "'}'";
        case LBRACKET:  return "'['";
        case RBRACKET:  return "']'";
        case SEMICOL:   return "';'";
        case COMA:      return "','";
        case DOT:       return "'.'";
         
        // Asignación y Operadores de Memoria/Especiales
        case ASSIGN:    return "'='";
        case AMPERSAND: return "'&'";
        case ARROW:     return "'=>'";
        
        // Literales
        case NUM:            return "número entero";
        case TRUE:           return "'true'";
        case FALSE:          return "'false'";
        case STRING_LITERAL: return "cadena de caracteres (string literal)";
        
        // Identificadores
        case ID:             return "identificador";
        
        // Palabras reservadas (Tipos de datos)
        case STRUCT:         return "'struct'";
        case INT_TYPE:       return "'int'";
        case STRING_TYPE:    return "'string'";
        case BOOL_TYPE:      return "'bool'";
        
        // Estructuras de control
        case IF:             return "'if'";
        case ELSE:           return "'else'";
        case WHILE:          return "'while'";
        case DO:             return "'do'";
        case FOR:            return "'for'";
        case SWITCH:         return "'switch'";
        case CASE:           return "'case'";
        case DEFAULT:        return "'default'";
        case BREAK:          return "'break'";
        
        // Funciones y Salida
        case PRINTF:         return "'printf'";
        case RETURN:         return "'return'";
        
        // Operadores relacionales y lógicos
        case LT_OP:          return "'<'";
        case LE_OP:          return "'<='";
        case GT_OP:          return "'>'";
        case GE_OP:          return "'>='";
        case EQ_OP:          return "'=='";
        case NE_OP:          return "'!='";
        case AND_OP:         return "'&&'";
        case OR_OP:          return "'||'";
        case NOT_OP:         return "'!'";
        
        // Especiales
        case ERR:            return "<error léxico>";
        case END:            return "fin de entrada (EOF)";
        
        default:             return "<tipo desconocido>";
    }
}

// -----------------------------------------------------------------------------
// Operador << para Token por referencia (Útil para depuración del Scanner)
// -----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& outs, const Token& tok) {
    switch (tok.type) {
        // Operadores aritméticos
        case Token::PLUS:      outs << "TOKEN(PLUS, \""      << tok.text << "\")"; break;
        case Token::MINUS:     outs << "TOKEN(MINUS, \""     << tok.text << "\")"; break;
        case Token::MUL:       outs << "TOKEN(MUL, \""       << tok.text << "\")"; break;
        case Token::DIV:       outs << "TOKEN(DIV, \""       << tok.text << "\")"; break;
        case Token::INC:       outs << "TOKEN(INC, \""       << tok.text << "\")"; break;
        case Token::DEC:       outs << "TOKEN(DEC, \""       << tok.text << "\")"; break;
        
        // Delimitadores y Puntuación
        case Token::LPAREN:    outs << "TOKEN(LPAREN, \""    << tok.text << "\")"; break;
        case Token::RPAREN:    outs << "TOKEN(RPAREN, \""    << tok.text << "\")"; break;
        case Token::LBRACE:    outs << "TOKEN(LBRACE, \""    << tok.text << "\")"; break;
        case Token::RBRACE:    outs << "TOKEN(RBRACE, \""    << tok.text << "\")"; break;
        case Token::LBRACKET:  outs << "TOKEN(LBRACKET, \""  << tok.text << "\")"; break;
        case Token::RBRACKET:  outs << "TOKEN(RBRACKET, \""  << tok.text << "\")"; break;
        case Token::SEMICOL:   outs << "TOKEN(SEMICOL, \""   << tok.text << "\")"; break;
        case Token::COMA:      outs << "TOKEN(COMA, \""      << tok.text << "\")"; break;
        case Token::DOT:       outs << "TOKEN(DOT, \""       << tok.text << "\")"; break;
        
        // Asignación y Operadores Especiales
        case Token::ASSIGN:    outs << "TOKEN(ASSIGN, \""    << tok.text << "\")"; break;
        case Token::AMPERSAND: outs << "TOKEN(AMPERSAND, \"" << tok.text << "\")"; break;
        case Token::ARROW:     outs << "TOKEN(ARROW, \""     << tok.text << "\")"; break;
        
        // Literales
        case Token::NUM:            outs << "TOKEN(NUM, \""            << tok.text << "\")"; break;
        case Token::TRUE:           outs << "TOKEN(TRUE, \""           << tok.text << "\")"; break;
        case Token::FALSE:          outs << "TOKEN(FALSE, \""          << tok.text << "\")"; break;
        case Token::STRING_LITERAL: outs << "TOKEN(STRING_LITERAL, \"" << tok.text << "\")"; break;
        
        // Identificadores
        case Token::ID:             outs << "TOKEN(ID, \""             << tok.text << "\")"; break;
        
        // Palabras reservadas (Tipos de datos)
        case Token::STRUCT:         outs << "TOKEN(STRUCT, \""         << tok.text << "\")"; break;
        case Token::INT_TYPE:       outs << "TOKEN(INT_TYPE, \""       << tok.text << "\")"; break;
        case Token::STRING_TYPE:    outs << "TOKEN(STRING_TYPE, \""    << tok.text << "\")"; break;
        case Token::BOOL_TYPE:      outs << "TOKEN(BOOL_TYPE, \""      << tok.text << "\")"; break;
        
        // Estructuras de control
        case Token::IF:             outs << "TOKEN(IF, \""             << tok.text << "\")"; break;
        case Token::ELSE:           outs << "TOKEN(ELSE, \""           << tok.text << "\")"; break;
        case Token::WHILE:          outs << "TOKEN(WHILE, \""          << tok.text << "\")"; break;
        case Token::DO:             outs << "TOKEN(DO, \""             << tok.text << "\")"; break;
        case Token::FOR:            outs << "TOKEN(FOR, \""            << tok.text << "\")"; break;
        case Token::SWITCH:         outs << "TOKEN(SWITCH, \""         << tok.text << "\")"; break;
        case Token::CASE:           outs << "TOKEN(CASE, \""           << tok.text << "\")"; break;
        case Token::DEFAULT:        outs << "TOKEN(DEFAULT, \""        << tok.text << "\")"; break;
        case Token::BREAK:          outs << "TOKEN(BREAK, \""          << tok.text << "\")"; break;
        
        // Funciones y Salida
        case Token::PRINTF:         outs << "TOKEN(PRINTF, \""         << tok.text << "\")"; break;
        case Token::RETURN:         outs << "TOKEN(RETURN, \""         << tok.text << "\")"; break;
        
        // Operadores relacionales y lógicos
        case Token::LT_OP:          outs << "TOKEN(LT_OP, \""          << tok.text << "\")"; break;
        case Token::LE_OP:          outs << "TOKEN(LE_OP, \""          << tok.text << "\")"; break;
        case Token::GT_OP:          outs << "TOKEN(GT_OP, \""          << tok.text << "\")"; break;
        case Token::GE_OP:          outs << "TOKEN(GE_OP, \""          << tok.text << "\")"; break;
        case Token::EQ_OP:          outs << "TOKEN(EQ_OP, \""          << tok.text << "\")"; break;
        case Token::NE_OP:          outs << "TOKEN(NE_OP, \""          << tok.text << "\")"; break;
        case Token::AND_OP:         outs << "TOKEN(AND_OP, \""         << tok.text << "\")"; break;
        case Token::OR_OP:          outs << "TOKEN(OR_OP, \""          << tok.text << "\")"; break;
        case Token::NOT_OP:         outs << "TOKEN(NOT_OP, \""         << tok.text << "\")"; break;
        
        // Especiales
        case Token::ERR:            outs << "TOKEN(ERR, \""            << tok.text << "\")"; break;
        case Token::END:            outs << "TOKEN(END)";                                    break;
    }
    return outs;
}

// -----------------------------------------------------------------------------
// Operador << para Token por puntero
// -----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& outs, const Token* tok) {
    if (!tok) return outs << "TOKEN(NULL)";
    return outs << *tok;
}