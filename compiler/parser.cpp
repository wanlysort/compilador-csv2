// =============================================================================
// parser.cpp — Implementación del Parser (Analizador Sintáctico Descendente)
// =============================================================================

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "ast.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"

// =============================================================================
// Constructor
// =============================================================================

Parser::Parser(Scanner *sc)
    : scanner(sc),
      previous(nullptr),
      current(nullptr),
      next(nullptr)
{
    current = scanner->nextToken();
    next = scanner->nextToken();

    if (current == nullptr)
    {
        throw std::runtime_error(
            "Error crítico: No se pudo leer el primer token de la entrada.");
    }

    if (current->type == Token::ERR)
    {
        throw std::runtime_error(
            "Error léxico: carácter no reconocido '" + current->text + "'");
    }

    if (next != nullptr && next->type == Token::ERR)
    {
        throw std::runtime_error(
            "Error léxico: carácter no reconocido '" + next->text + "'");
    }
}

// =============================================================================
// Primitivas de Consumo de Tokens
// =============================================================================

bool Parser::isAtEnd()
{
    return current == nullptr || current->type == Token::END;
}

bool Parser::check(Token::Type ttype)
{
    if (isAtEnd())
        return false;
    return current->type == ttype;
}

bool Parser::advance()
{
    if (!isAtEnd())
    {
        previous = current;

        current = next;
        next = scanner->nextToken();

        if (current == nullptr)
        {
            throw std::runtime_error(
                "Error crítico: El Scanner retornó un token nulo.");
        }

        if (current->type == Token::ERR)
        {
            throw std::runtime_error(
                "Error léxico: carácter no reconocido '" +
                current->text + "'");
        }

        if (next != nullptr && next->type == Token::ERR)
        {
            throw std::runtime_error(
                "Error léxico: carácter no reconocido '" +
                next->text + "'");
        }

        return true;
    }

    return false;
}
bool Parser::match(Token::Type ttype)
{
    if (check(ttype))
    {
        advance();
        return true;
    }
    return false;
}

void Parser::error(const std::string &expected)
{
    std::string found;
    if (isAtEnd())
    {
        found = "fin de entrada";
    }
    else
    {
        found = Token::typeName(current->type);
        if (!current->text.empty())
            found += " '" + current->text + "'";
    }
    throw std::runtime_error("Error sintáctico: se esperaba " + expected + ", pero se encontró " + found);
}

void Parser::expect(Token::Type ttype)
{
    if (!match(ttype))
        error(Token::typeName(ttype));
}

bool Parser::looksLikeVarDeclaration()
{
    if (check(Token::INT_TYPE))
        return true;

    if (check(Token::STRING_TYPE))
        return true;

    if (check(Token::BOOL_TYPE))
        return true;

    if (current->type == Token::ID &&
        next != nullptr &&
        next->type == Token::ID)
    {
        return true;
    }

    return false;
}
// =============================================================================
// Implementación de Reglas Gramaticales
// =============================================================================

// Program ::= StructDecList VarDecList FunDecList
Program *Parser::parseProgram()
{
    Program *p = new Program();
    while (check(Token::STRUCT))
    {
        p->sdlist.push_back(parseStructDec());
    }

    // Auxiliar para discriminar si viene un VarDec (Tipo + ID) o un FunDec ([<ID>] Tipo ID '(' )
    auto isVarDecAhead = [&]()
    {
        if (check(Token::LT_OP))
            return false; // [<id>] -> FunDec
        // Si es un tipo base válido (int, string, bool, id)
        if (check(Token::INT_TYPE) || check(Token::STRING_TYPE) || check(Token::BOOL_TYPE) || check(Token::ID))
        {
            return true;
        }
        return false;
    };

    // 2. VarDecList ::= (VarDec)*
    // Si viene un tipo pero NO es una función (miramos si tiene parámetros después)
    while (false)
    {
        // Guardamos estado o una pre-lectura simple. En C#, si sigue un id y luego '(' o '<' es función.
        // Como tu gramática separa limpiamente las listas globales en bloques consecutivos:
        if (check(Token::ID))
        {
            // Si el ID es seguido por un '<' o por otro ID + '(', es una función. De lo contrario, es declaración de struct/variable.
            // Para simplificar según el flujo estándar de tus listas ordenadas estructurales:
            break;
        }
        p->vdlist.push_back(parseVarDec());
    }
    while (check(Token::INT_TYPE) ||
           check(Token::STRING_TYPE) ||
           check(Token::BOOL_TYPE) ||
           check(Token::ID))
    {
        p->fdlist.push_back(parseFunDec());
    }

    // 3. FunDecList ::= (FunDec)*
    while (check(Token::LT_OP) || check(Token::INT_TYPE) || check(Token::STRING_TYPE) || check(Token::BOOL_TYPE) || check(Token::ID))
    {
        p->fdlist.push_back(parseFunDec());
    }

    if (!isAtEnd())
    {
        error("fin de archivo o declaración estructural válida");
    }

    std::cout << "Parser exitoso" << std::endl;
    return p;
}

// StructDec ::= struct id { (Type id ;)* } ;
StructDec *Parser::parseStructDec()
{
    expect(Token::STRUCT);
    expect(Token::ID);
    std::string structName = previous->text;

    expect(Token::LBRACE); // {

    std::vector<TypeNode *> mTypes;
    std::vector<std::string> mNames;

    while (!check(Token::RBRACE) && !isAtEnd())
    {
        mTypes.push_back(parseType());
        expect(Token::ID);
        mNames.push_back(previous->text);
        expect(Token::SEMICOL);
    }
    expect(Token::RBRACE);  // }
    expect(Token::SEMICOL); // ;

    return new StructDec(structName, mTypes, mNames);
}

// FunDec ::= [ < id > ] Type id ( [ParamDecList] ) { Body }
FunDec *Parser::parseFunDec()
{

    std::string genericParam = "";
    if (match(Token::LT_OP)) // <
    {
        expect(Token::ID);
        genericParam = previous->text;
        expect(Token::GT_OP); // >
    }

    TypeNode *retType = parseType();
    expect(Token::ID);
    std::string funcName = previous->text;

    expect(Token::LPAREN);

    std::vector<TypeNode *> pTypes;
    std::vector<std::string> pNames;

    if (!check(Token::RPAREN))
    {
        pTypes.push_back(parseType());
        expect(Token::ID);
        pNames.push_back(previous->text);

        while (match(Token::COMA))
        {
            pTypes.push_back(parseType());
            expect(Token::ID);
            pNames.push_back(previous->text);
        }
    }
    expect(Token::RPAREN);

    expect(Token::LBRACE); // {
    Body *body = parseBody();
    expect(Token::RBRACE); // }

    return new FunDec(genericParam, retType, funcName, pTypes, pNames, body);
}

// VarDec ::= Type VarList ;
VarDec *Parser::parseVarDec()
{
    std::cout << "parseVarDec()" << std::endl;
    TypeNode *t = parseType();
    std::vector<VarNode *> vList;

    vList.push_back(parseVar());
    while (match(Token::COMA))
    {
        vList.push_back(parseVar());
    }

    expect(Token::SEMICOL);

    return new VarDec(t, vList);
}

// Var ::= id DimList [= InitValue]
VarNode *Parser::parseVar()
{
    std::cout << "parseVar()" << std::endl;
    expect(Token::ID);
    std::string varName = previous->text;

    // DimList ::= ([ Num ])*
    std::vector<int> dims;
    while (match(Token::LBRACKET)) // [
    {
        expect(Token::NUM);
        dims.push_back(std::stoi(previous->text));
        expect(Token::RBRACKET); // ]
    }

    InitValueNode *init = nullptr;
    if (match(Token::ASSIGN)) // =
    {
        init = parseInitValue();
    }

    return new VarNode(varName, dims, init);
}

// InitValue ::= AExp | InitList
InitValueNode *Parser::parseInitValue()
{
    std::cout << "parseInitValue()" << std::endl;
    if (check(Token::LBRACE)) // { -> Es una lista
    {
        return parseInitList();
    }
    return new InitValueNode(parseAExp());
}

// InitList ::= { InitItemList }
InitValueNode *Parser::parseInitList()
{
    expect(Token::LBRACE);
    std::vector<InitValueNode *> items;

    items.push_back(parseInitValue());
    while (match(Token::COMA))
    {
        items.push_back(parseInitValue());
    }
    expect(Token::RBRACE);
    return new InitValueNode(items);
}

// Type ::= BaseType ( * )*
TypeNode *Parser::parseType()
{
    std::string base = "";
    std::string generic = "";

    if (match(Token::INT_TYPE))
        base = "int";
    else if (match(Token::STRING_TYPE))
        base = "string";
    else if (match(Token::BOOL_TYPE))
        base = "bool";
    else if (match(Token::ID))
    {
        base = previous->text;
        // id [ < Type > ]
        if (match(Token::LT_OP)) // <
        {
            TypeNode *innerType = parseType();
            generic = innerType->baseName; // Simplificación del mapeo interno
            delete innerType;
            expect(Token::GT_OP); // >
        }
    }
    else
    {
        error("nombre de tipo base (int, string, bool o identificador)");
    }

    int ptrs = 0;
    while (match(Token::MUL)) // *
    {
        ptrs++;
    }

    return new TypeNode(base, ptrs, generic);
}

// Body ::= BlockItemList ::= (VarDec | Stmt)*
Body *Parser::parseBody()
{
    Body *b = new Body();
    std::cout << "parseBody()" << std::endl;

    while (!check(Token::RBRACE) && !isAtEnd())
    {
        std::cout << "Body token actual: "
                  << current->text
                  << std::endl;

        if (looksLikeVarDeclaration())
        {
            b->declarations.push_back(parseVarDec());
            b->isStatementOrder.push_back(false);
        }
        else
        {
            b->statements.push_back(parseStm());
            b->isStatementOrder.push_back(true);
        }
    }

    return b;
}

// Stmt ::= for | while | if | do | LValue = AExp ; | printf | return
Stm *Parser::parseStm()
{
    // ---- for ( Type id = CExp ; CExp ; id (++ | --) ) { Body } ----
    if (match(Token::FOR))
    {
        expect(Token::LPAREN);
        TypeNode *t = parseType();
        expect(Token::ID);
        std::string idInit = previous->text;
        expect(Token::ASSIGN);
        Exp *initEx = parseCExp();
        expect(Token::SEMICOL);

        Exp *condition = parseCExp();
        expect(Token::SEMICOL);

        expect(Token::ID);
        std::string idUp = previous->text;

        bool isInc = true;
        if (match(Token::INC))
            isInc = true; // ++
        else if (match(Token::DEC))
            isInc = false; // --
        else
            error("'++' o '--' en la actualización del bucle for");

        expect(Token::RPAREN);
        expect(Token::LBRACE);
        Body *b = parseBody();
        expect(Token::RBRACE);

        return new ForStm(t, idInit, initEx, condition, idUp, isInc, b);
    }

    // ---- while ( AExp ) { Body } ----
    if (match(Token::WHILE))
    {
        expect(Token::LPAREN);
        Exp *cond = parseAExp();
        expect(Token::RPAREN);
        expect(Token::LBRACE);
        Body *b = parseBody();
        expect(Token::RBRACE);
        return new WhileStm(cond, b);
    }

    // ---- if ( AExp ) { Body } (else if ( AExp ) { Body })* [else { Body }] ----
    if (match(Token::IF))
    {
        std::vector<Exp *> conds;
        std::vector<Body *> branches;
        Body *elsBr = nullptr;

        expect(Token::LPAREN);
        conds.push_back(parseAExp());
        expect(Token::RPAREN);

        expect(Token::LBRACE);
        branches.push_back(parseBody());
        expect(Token::RBRACE);

        // (else if (AExp) { Body })*
        while (match(Token::ELSE))
        {
            if (match(Token::IF))
            {
                expect(Token::LPAREN);
                conds.push_back(parseAExp());
                expect(Token::RPAREN);
                expect(Token::LBRACE);
                branches.push_back(parseBody());
                expect(Token::RBRACE);
            }
            else // Rama else final
            {
                expect(Token::LBRACE);
                elsBr = parseBody();
                expect(Token::RBRACE);
                break; // El else definitivo termina la cadena
            }
        }
        return new IfStm(conds, branches, elsBr);
    }

    // ---- do { Body } while ( AExp ) ; ----
    if (match(Token::DO))
    {
        expect(Token::LBRACE);
        Body *body = parseBody();
        expect(Token::RBRACE);
        expect(Token::WHILE);
        expect(Token::LPAREN);
        Exp *cond = parseAExp();
        expect(Token::RPAREN);
        expect(Token::SEMICOL);
        return new DoWhileStm(body, cond);
    }

    // ---- printf ( Format , AExp ) ; ----
    if (match(Token::PRINTF))
    {
        expect(Token::LPAREN);
        expect(Token::STRING_LITERAL);
        std::string formatStr = previous->text;
        expect(Token::COMA);
        Exp *e = parseAExp();
        expect(Token::RPAREN);
        expect(Token::SEMICOL);
        return new PrintfStm(formatStr, e);
    }

    // ---- return AExp ; ----
    if (match(Token::RETURN))
    {
        Exp *e = parseAExp();
        expect(Token::SEMICOL);
        return new ReturnStm(e);
    }

    // ---- LValue = AExp ; ----
    LValueNode *target = parseLValue();
    expect(Token::ASSIGN);
    Exp *e = parseAExp();
    expect(Token::SEMICOL);
    return new AssignStm(target, e);
}

// LValue ::= ( * )* id DimAccess ( . id DimAccess )*
LValueNode *Parser::parseLValue()
{
    int ptrs = 0;
    while (match(Token::MUL))
        ptrs++;

    expect(Token::ID);
    std::vector<std::string> ids = {previous->text};
    std::vector<std::vector<Exp *>> accesses;

    accesses.push_back(parseDimAccess());

    while (match(Token::DOT)) // .
    {
        expect(Token::ID);
        ids.push_back(previous->text);
        accesses.push_back(parseDimAccess());
    }

    return new LValueNode(ptrs, ids, accesses);
}
// DimAccess ::= ([ AExp ])*
std::vector<Exp *> Parser::parseDimAccess()
{
    std::vector<Exp *> dynamicAccesses;
    while (match(Token::LBRACKET)) // [
    {
        dynamicAccesses.push_back(parseAExp());
        expect(Token::RBRACKET); // ]
    }
    return dynamicAccesses;
}

// =============================================================================
// Jerarquía de Expresiones Matched con Precedencia Completa
// =============================================================================

// AExp ::= BExp [ ( && | || ) BExp ]
Exp *Parser::parseAExp()
{
    Exp *l = parseBExp();
    if (check(Token::AND_OP) || check(Token::OR_OP))
    {
        advance();
        BinaryOp op = (previous->type == Token::AND_OP) ? AND_OP : OR_OP;
        Exp *r = parseBExp();
        l = new BinaryExp(l, r, op);
    }
    return l;
}

// BExp ::= CExp | ! ( CExp )
Exp *Parser::parseBExp()
{
    if (match(Token::NOT_OP)) // !
    {
        expect(Token::LPAREN);
        Exp *operand = parseCExp();
        expect(Token::RPAREN);
        return new UnaryExp(operand, NOT_OP);
    }
    return parseCExp();
}

// CExp ::= Exp [ ( < | <= | == | > | >= | != ) Exp ]
Exp *Parser::parseCExp()
{
    Exp *l = parseExp();
    if (check(Token::LT_OP) || check(Token::LE_OP) || check(Token::EQ_OP) ||
        check(Token::GT_OP) || check(Token::GE_OP) || check(Token::NE_OP))
    {
        advance();
        Token::Type t = previous->type;
        BinaryOp op;

        if (t == Token::LT_OP)
            op = LT_OP;
        else if (t == Token::LE_OP)
            op = LE_OP;
        else if (t == Token::EQ_OP)
            op = EQ_OP;
        else if (t == Token::GT_OP)
            op = GT_OP;
        else if (t == Token::GE_OP)
            op = GE_OP;
        else if (t == Token::NE_OP)
            op = NE_OP;

        Exp *r = parseExp();
        l = new BinaryExp(l, r, op);
    }
    return l;
}

// Exp ::= Term ( ( + | - ) Term )*
Exp *Parser::parseExp()
{
    Exp *l = parseTerm();
    while (match(Token::PLUS) || match(Token::MINUS))
    {
        BinaryOp op = (previous->type == Token::PLUS) ? PLUS_OP : MINUS_OP;
        Exp *r = parseTerm();
        l = new BinaryExp(l, r, op);
    }
    return l;
}

// Term ::= Factor ( ( * | / ) Factor )*
Exp *Parser::parseTerm()
{
    Exp *l = parseFactor();
    while (match(Token::MUL) || match(Token::DIV))
    {
        BinaryOp op = (previous->type == Token::MUL) ? MUL_OP : DIV_OP;
        Exp *r = parseFactor();
        l = new BinaryExp(l, r, op);
    }
    return l;
}

// Factor ::= LValue | &id | Num | Bool | StringLiteral | LambdaDec | ( AExp )
Exp *Parser::parseFactor()
{
    if (match(Token::AMPERSAND)) // &id
    {
        expect(Token::ID);
        return new UnaryExp(previous->text, ADDRESS_OF_OP);
    }

    if (match(Token::NUM))
    {
        return new NumberExp(std::stoi(previous->text));
    }

    if (match(Token::TRUE))
        return new BoolExp(true);
    if (match(Token::FALSE))
        return new BoolExp(false);

    if (match(Token::STRING_LITERAL))
    {
        return new StringLiteralExp(previous->text);
    }

    if (check(Token::LPAREN))
    {
        advance();
        if (check(Token::RPAREN) || check(Token::INT_TYPE) || check(Token::STRING_TYPE) || check(Token::BOOL_TYPE))
        {
            std::vector<TypeNode *> pTypes;
            std::vector<std::string> pNames;

            if (!check(Token::RPAREN))
            {
                pTypes.push_back(parseType());
                expect(Token::ID);
                pNames.push_back(previous->text);
                while (match(Token::COMA))
                {
                    pTypes.push_back(parseType());
                    expect(Token::ID);
                    pNames.push_back(previous->text);
                }
            }
            expect(Token::RPAREN);
            expect(Token::ARROW);
            expect(Token::LBRACE);
            Body *lambBody = parseBody();
            expect(Token::RBRACE);
            return new LambdaDecExp(pTypes, pNames, lambBody);
        }

        Exp *e = parseAExp();
        expect(Token::RPAREN);
        return e;
    }

    LValueNode *lval = parseLValue();

    if (match(Token::LPAREN))
    {
        std::vector<Exp *> args;

        if (!check(Token::RPAREN))
        {
            args.push_back(parseAExp());

            while (match(Token::COMA))
                args.push_back(parseAExp());
        }

        expect(Token::RPAREN);

        return new CallExp(
            lval->memberIds[0],
            args);
    }

    return lval;
}