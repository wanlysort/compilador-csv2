// =============================================================================
// scanner.cpp — Implementación del Scanner (analizador léxico)
// =============================================================================

#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include "scanner.h"
#include "token.h"

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------

Scanner::Scanner(const char *s) : input(s), first(0), current(0) {}

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------

Scanner::~Scanner() {}

// -----------------------------------------------------------------------------
// Función auxiliar
// -----------------------------------------------------------------------------

static bool is_white_space(char c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

// -----------------------------------------------------------------------------
// nextToken — produce el siguiente token de la entrada
// -----------------------------------------------------------------------------

Token *Scanner::nextToken()
{
    while (current < input.length())
    {
        if (input[current] == ' ' || input[current] == '\t' || input[current] == '\r' || input[current] == '\n')
        {
            current++;
            continue;
        }

        if (input[current] == '/' && current + 1 < input.length())
        {

            if (input[current + 1] == '/')
            {
                current += 2; 
                while (current < input.length() && input[current] != '\n')
                {
                    current++;
                }
                continue; 
            }

            if (input[current + 1] == '*')
            {
                current += 2; 
                bool cerrado = false;

                while (current + 1 < input.length())
                {
                    if (input[current] == '*' && input[current + 1] == '/')
                    {
                        current += 2; 
                        cerrado = true;
                        break;
                    }
                    current++;
                }

                if (!cerrado)
                {
                    std::cerr << "[Scanner Error] Comentario multilínea no cerrado." << std::endl;
                    current = input.length(); 
                }
                continue; 
            }
        }

        break;
    }

    // Fin de la entrada
    if (current >= input.length())
        return new Token(Token::END);

    char c = input[current];
    first = current;

    // ---- Cadenas de caracteres (String Literals para Format / printf) ----
    if (c == '"')
    {
        current++; // Consumir la comilla inicial
        while (current < input.length() && input[current] != '"')
        {
            // Soporte básico para caracteres de escape de barra invertida (\", \n)
            if (input[current] == '\\' && current + 1 < input.length())
            {
                current += 2;
            }
            else
            {
                current++;
            }
        }

        if (current >= input.length())
        {
            // Error: Cadena no cerrada al llegar al fin de archivo
            return new Token(Token::ERR, input, first, current - first);
        }

        current++;
        return new Token(Token::STRING_LITERAL, input, first, current - first);
    }

    // ---- Números enteros ----
    if (isdigit(c))
    {
        current++;
        while (current < input.length() && isdigit(input[current]))
            current++;
        return new Token(Token::NUM, input, first, current - first);
    }

    // ---- Identificadores y palabras reservadas ----
    if (isalpha(c) || c == '_')
    {
        current++;
        while (current < input.length() && (isalnum(input[current]) || input[current] == '_'))
            current++;
        std::string lexema = input.substr(first, current - first);

        // Tipos de datos básicos e instrucciones estructurales
        if (lexema == "struct")
            return new Token(Token::STRUCT, input, first, current - first);
        if (lexema == "int")
            return new Token(Token::INT_TYPE, input, first, current - first);
        if (lexema == "string")
            return new Token(Token::STRING_TYPE, input, first, current - first);
        if (lexema == "bool")
            return new Token(Token::BOOL_TYPE, input, first, current - first);

        // Control de flujo
        if (lexema == "if")
            return new Token(Token::IF, input, first, current - first);
        if (lexema == "else")
            return new Token(Token::ELSE, input, first, current - first);
        if (lexema == "while")
            return new Token(Token::WHILE, input, first, current - first);
        if (lexema == "do")
            return new Token(Token::DO, input, first, current - first);
        if (lexema == "for")
            return new Token(Token::FOR, input, first, current - first);
        if (lexema == "switch")
            return new Token(Token::SWITCH, input, first, current - first);
        if (lexema == "case")
            return new Token(Token::CASE, input, first, current - first);
        if (lexema == "default")
            return new Token(Token::DEFAULT, input, first, current - first);
        if (lexema == "break")
            return new Token(Token::BREAK, input, first, current - first);

        // Literales booleanos
        if (lexema == "true")
            return new Token(Token::TRUE, input, first, current - first);
        if (lexema == "false")
            return new Token(Token::FALSE, input, first, current - first);

        // Funciones y salida
        if (lexema == "printf")
            return new Token(Token::PRINTF, input, first, current - first);
        if (lexema == "return")
            return new Token(Token::RETURN, input, first, current - first);

        // Identificador genérico por defecto
        return new Token(Token::ID, input, first, current - first);
    }

    // ---- Operadores y Delimitadores
    if (strchr("+/-*();=<,>!&|{}[].", c))
    {
        current++; // Avanzar inmediatamente

        switch (c)
        {
        case '+':
            if (current < input.length() && input[current] == '+')
            {
                current++;
                return new Token(Token::INC, input, first, current - first);
            }
            return new Token(Token::PLUS, c);
        case '-':
            if (current < input.length() && input[current] == '-')
            {
                current++;
                return new Token(Token::DEC, input, first, current - first);
            }
            return new Token(Token::MINUS, c);
        case '/':
            return new Token(Token::DIV, c);
        case '(':
            return new Token(Token::LPAREN, c);
        case ')':
            return new Token(Token::RPAREN, c);
        case '{':
            return new Token(Token::LBRACE, c);
        case '}':
            return new Token(Token::RBRACE, c);
        case '[':
            return new Token(Token::LBRACKET, c);
        case ']':
            return new Token(Token::RBRACKET, c);
        case ';':
            return new Token(Token::SEMICOL, c);
        case ',':
            return new Token(Token::COMA, c);
        case '.':
            return new Token(Token::DOT, c);
        case '*':
            return new Token(Token::MUL, c);

        case '=':
            if (current < input.length() && input[current] == '=')
            {
                current++;
                return new Token(Token::EQ_OP, input, first, current - first);
            }
            if (current < input.length() && input[current] == '>')
            {
                current++;
                return new Token(Token::ARROW, input, first, current - first);
            }
            return new Token(Token::ASSIGN, input, first, current - first);

        case '!':
            if (current < input.length() && input[current] == '=')
            {
                current++;
                return new Token(Token::NE_OP, input, first, current - first);
            }
            return new Token(Token::NOT_OP, input, first, current - first);

        case '<':
            if (current < input.length() && input[current] == '=')
            {
                current++;
                return new Token(Token::LE_OP, input, first, current - first);
            }
            return new Token(Token::LT_OP, c);

        case '>':
            if (current < input.length() && input[current] == '=')
            {
                current++;
                return new Token(Token::GE_OP, input, first, current - first);
            }
            return new Token(Token::GT_OP, c);

        case '&':
            if (current < input.length() && input[current] == '&')
            {
                current++;
                return new Token(Token::AND_OP, input, first, current - first);
            }
            return new Token(Token::AMPERSAND, c);

        case '|':
            if (current < input.length() && input[current] == '|')
            {
                current++;
                return new Token(Token::OR_OP, input, first, current - first);
            }
            return new Token(Token::ERR, c);
        }
    }

    Token *err = new Token(Token::ERR, c);
    current++;
    return err;
}

// -----------------------------------------------------------------------------
// ejecutar_scanner — tokeniza un archivo y escribe los resultados
// -----------------------------------------------------------------------------

int ejecutar_scanner(Scanner *scanner, const std::string &InputFile)
{
    std::string outputName = InputFile;
    size_t pos = outputName.find_last_of('.');
    if (pos != std::string::npos)
        outputName = outputName.substr(0, pos);
    outputName += "_tokens.txt";

    std::ofstream outFile(outputName);
    if (!outFile.is_open())
    {
        std::cerr << "Error: no se pudo abrir el archivo de salida: " << outputName << std::endl;
        return 1;
    }

    outFile << "Scanner\n"
            << std::endl;

    Token *tok;
    while (true)
    {
        tok = scanner->nextToken();

        if (tok->type == Token::ERR)
        {
            outFile << *tok << std::endl;
            outFile << "Error léxico: carácter o cadena inválida '" << tok->text << "'\n";
            outFile << "\nScanner no exitoso\n";
            delete tok;
            outFile.close();
            return 1;
        }

        outFile << *tok << std::endl;

        if (tok->type == Token::END)
        {
            outFile << "\nScanner exitoso\n";
            delete tok;
            outFile.close();
            return 0;
        }

        delete tok;
    }
}