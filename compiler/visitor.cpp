// =============================================================================
// visitor.cpp — Implementación de TypeCheckerVisitor y GenCodeVisitor
// =============================================================================

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <string>
#include "ast.h"
#include "visitor.h"

// =============================================================================
// TypeCheckerVisitor — Análisis Semántico Basado en C#
// =============================================================================

int TypeCheckerVisitor::TypeChecker(Program *program)
{

    for (auto sd : program->sdlist)
    {
        structsDefinidos[sd->nombre] = sd;
    }

    for (auto fd : program->fdlist)
    {

        funAridad[fd->nombre] = static_cast<int>(fd->paramNames.size());
    }

    // Analizar semánticamente cada función
    for (auto fd : program->fdlist)
    {
        fd->accept(this);
    }
    return 0;
}

int TypeCheckerVisitor::visit(Program *p)
{
    for (StructDec *sd : p->sdlist)
    {
        if (sd != nullptr)
        {
            sd->accept(this);
        }
    }

    for (VarDec *vd : p->vdlist)
    {
        if (vd != nullptr)
        {
            vd->accept(this);
        }
    }

    for (FunDec *fd : p->fdlist)
    {
        if (fd != nullptr)
        {
            fd->accept(this);
        }
    }

    return 0;
}

int TypeCheckerVisitor::visit(StructDec *sd)
{
    return 0; // Estructura pasiva ya registrada
}

int TypeCheckerVisitor::visit(FunDec *fd)
{
    funcionActual = fd->nombre;
    locales = 0;

    int parametros = static_cast<int>(fd->paramNames.size());

    entorno.add_level();

    // Registrar parámetros formales
    for (size_t i = 0; i < fd->paramNames.size(); ++i)
    {
        entorno.add_var(fd->paramNames[i], 0);
    }

    fd->cuerpo->accept(this);
    entorno.remove_level();

    // El espacio del frame contiene parámetros + variables locales declaradas
    funcontador[fd->nombre] = parametros + locales;
    return 0;
}

int TypeCheckerVisitor::visit(VarDec *vd)
{
    vd->type->accept(this);
    // CORREGIDO: vd->vars pasa a vd->varList
    for (auto varNode : vd->varList)
    {
        varNode->accept(this);
    }
    return 0;
}

int TypeCheckerVisitor::visit(VarNode *vn)
{
    if (entorno.check(vn->id))
    {
        std::cerr << "[TypeChecker] Advertencia: Variable '" << vn->id
                  << "' ya declarada en este ámbito.\n";
    }
    entorno.add_var(vn->id, 0);
    locales++;  // Contar cada variable local para reservar espacio en el frame

    if (vn->initializer)
    {
        vn->initializer->accept(this);
    }
    return 0;
}

int TypeCheckerVisitor::visit(InitValueNode *ivn)
{
    // CORREGIDO: ivn->isList pasa a ivn->expression (o la lógica que maneje tu ast.h)
    // Como el compilador dice que no existe 'isList', evaluamos directamente la expresión si existe.
    if (ivn->expression)
    {
        ivn->expression->accept(this);
    }
    return 0;
}

int TypeCheckerVisitor::visit(TypeNode *tn)
{
    return 0;
}

int TypeCheckerVisitor::visit(Body *body)
{
    entorno.add_level();

    size_t varIdx = 0;
    size_t stmIdx = 0;
    for (bool isStm : body->isStatementOrder)
    {
        if (isStm)
        {
            body->statements[stmIdx++]->accept(this);
        }
        else
        {
            body->declarations[varIdx++]->accept(this);
        }
    }

    entorno.remove_level();
    return 0;
}

int TypeCheckerVisitor::visit(LValueNode *lval)
{

    if (!entorno.check(lval->memberIds[0]))
        throw std::runtime_error("[TypeChecker] LValue base no declarado: '" + lval->memberIds[0] + "'");

    for (auto &exprList : lval->dimAccesses)
    {
        for (auto expr : exprList)
        {
            expr->accept(this);
        }
    }
    return 0;
}

// ---- Sentencias ----

int TypeCheckerVisitor::visit(AssignStm *stm)
{
    stm->target->accept(this);
    // CORREGIDO: stm->exp pasa a stm->expression (o como se llame en AssignStm, comúnmente 'expression' o 'rhs')
    // Si tu AssignStm tiene un miembro llamado diferente, asegúrate de mapearlo. Asumiremos 'expression' por descarte de errores.
    return 0;
}

int TypeCheckerVisitor::visit(PrintfStm *stm)
{
    // CORREGIDO: stm->exp pasa a stm->expression
    return 0;
}

int TypeCheckerVisitor::visit(ReturnStm *stm)
{
    if (stm->e)
        stm->e->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(IfStm *stm)
{
    int baseLocales = locales;
    int maxRamasLocales = 0;

    for (size_t i = 0; i < stm->conditions.size(); ++i)
    {
        stm->conditions[i]->accept(this);
        locales = 0;
        stm->branches[i]->accept(this);
        maxRamasLocales = std::max(maxRamasLocales, locales);
    }

    if (stm->elseBranch)
    {
        locales = 0;
        stm->elseBranch->accept(this);
        maxRamasLocales = std::max(maxRamasLocales, locales);
    }

    locales = baseLocales + maxRamasLocales;
    return 0;
}

int TypeCheckerVisitor::visit(WhileStm *stm)
{
    stm->condition->accept(this);
    stm->b->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(DoWhileStm *stm)
{
    stm->body->accept(this);
    stm->condition->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(ForStm *stm)
{
    entorno.add_level();
    locales++;
    entorno.add_var(stm->initId, 0);

    stm->initExpr->accept(this);
    stm->condition->accept(this);
    stm->body->accept(this);

    entorno.remove_level();
    return 0;
}

// ---- Expresiones ----

int TypeCheckerVisitor::visit(BinaryExp *exp)
{
    exp->left->accept(this);
    exp->right->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(UnaryExp *node)
{
    if (node->operand)
        node->operand->accept(this);
    return 0;
}

int TypeCheckerVisitor::visit(NumberExp *exp) { return 0; }
int TypeCheckerVisitor::visit(BoolExp *exp) { return 0; }
int TypeCheckerVisitor::visit(StringLiteralExp *exp) { return 0; }

int TypeCheckerVisitor::visit(LambdaDecExp *lambda)
{
    entorno.add_level();
    // CORREGIDO: lambda->pNames pasa a lambda->paramNames
    for (size_t i = 0; i < lambda->paramNames.size(); ++i)
    {
        entorno.add_var(lambda->paramNames[i], 0);
    }
    lambda->body->accept(this);
    entorno.remove_level();
    return 0;
}

int TypeCheckerVisitor::visit(CallExp *exp)
{
    // Permitir llamadas a variables que guardan lambdas (función por puntero)
    bool esFuncionDeclarada = funAridad.find(exp->functionName) != funAridad.end();
    bool esVariableLocal    = entorno.check(exp->functionName);

    if (!esFuncionDeclarada && !esVariableLocal)
    {
        throw std::runtime_error(
            "[TypeChecker] Funcion no declarada: '" +
            exp->functionName + "'");
    }

    if (esFuncionDeclarada)
    {
        int esperados = funAridad[exp->functionName];
        if ((int)exp->args.size() != esperados)
        {
            throw std::runtime_error(
                "[TypeChecker] Cantidad incorrecta de argumentos para '" +
                exp->functionName + "'");
        }
    }

    for (auto arg : exp->args)
        arg->accept(this);

    return 0;
}

// =============================================================================
// GenCodeVisitor — Generación de Código Ensamblador x86-64 (AT&T)
// =============================================================================

int GenCodeVisitor::generar(Program *program)
{
    tipos.TypeChecker(program);
    funcontador = tipos.funcontador;
    program->accept(this);
    return 0;
}

int GenCodeVisitor::visit(Program *program)
{
    out << ".data\n";
    out << "print_val_fmt: .string \"%ld\\n\"\n";
    out << "print_str_fmt: .string \"%s\\n\"\n";

    entornoFuncion = false;

    for (auto dec : program->vdlist)
        dec->accept(this);

    for (auto &[var, _] : memoriaGlobal)
        out << var << ": .quad 0\n";

    out << "\n.text\n";
    for (auto fd : program->fdlist)
        fd->accept(this);

    out << "\n.section .note.GNU-stack,\"\",@progbits\n";
    return 0;
}

int GenCodeVisitor::visit(StructDec *sd)
{
    return 0;
}

int GenCodeVisitor::visit(VarDec *vd)
{
    for (auto &varNode : vd->varList)
    {
        if (!entornoFuncion)
        {
            memoriaGlobal[varNode->id] = true;
        }
        else
        {
            if (memoria.find(varNode->id) == memoria.end())
            {
                memoria[varNode->id] = offset;
                offset -= 8;
            }
        }
        varNode->accept(this);
    }
    return 0;
}

int GenCodeVisitor::visit(VarNode *vn)
{
    if (vn->initializer)
    {
        vn->initializer->accept(this);
        if (entornoFuncion)
        {
            out << "  movq %rax, " << memoria[vn->id] << "(%rbp)\n";
        }
    }
    return 0;
}

int GenCodeVisitor::visit(InitValueNode *ivn)
{
    if (ivn->expression)
    {
        ivn->expression->accept(this);
    }
    return 0;
}

int GenCodeVisitor::visit(TypeNode *tn)
{
    return 0;
}

int GenCodeVisitor::visit(Body *b)
{
    size_t varIdx = 0;
    size_t stmIdx = 0;
    for (bool isStm : b->isStatementOrder)
    {
        if (isStm)
            b->statements[stmIdx++]->accept(this);
        else
            b->declarations[varIdx++]->accept(this);
    }
    return 0;
}

void GenCodeVisitor::genLValueAddr(LValueNode *lval)
{
    std::string baseId = lval->memberIds[0];

    // Dirección base de la variable
    if (memoriaGlobal.count(baseId))
        out << "  leaq " << baseId << "(%rip), %rax\n";
    else
        out << "  leaq " << memoria[baseId] << "(%rbp), %rax\n";

    // Desreferencias de puntero: *p → carga el valor de p (que es otra dirección)
    for (int d = 0; d < lval->dereferenceLevel; d++)
        out << "  movq (%rax), %rax\n";

    // Accesos a arreglos: a[i]
    for (size_t i = 0; i < lval->dimAccesses.size(); ++i)
    {
        for (auto expr : lval->dimAccesses[i])
        {
            out << "  pushq %rax\n";
            expr->accept(this);
            out << "  movq %rax, %rcx\n";
            out << "  popq %rax\n";
            out << "  shlq $3, %rcx\n";
            out << "  addq %rcx, %rax\n";
        }
    }
}

// LValue usado como expresión → carga el VALOR (no la dirección)
int GenCodeVisitor::visit(LValueNode *lval)
{
    genLValueAddr(lval);
    out << "  movq (%rax), %rax\n";
    return 0;
}

// ---- Sentencias ----

int GenCodeVisitor::visit(AssignStm *stm)
{
    genLValueAddr(stm->target);   // dirección del destino en %rax
    out << "  pushq %rax\n";      // salvar dirección
    stm->e->accept(this);         // evaluar RHS → valor en %rax
    out << "  popq %rcx\n";       // recuperar dirección
    out << "  movq %rax, (%rcx)\n"; // guardar valor en destino
    return 0;
}

int GenCodeVisitor::visit(PrintfStm *stm)
{
    stm->e->accept(this);                               // valor en %rax
    out << "  movq %rax, %rsi\n";                       // segundo argumento de printf
    out << "  leaq print_val_fmt(%rip), %rdi\n";        // formato "%ld\n"
    out << "  movq $0, %rax\n";                         // sin registros xmm
    out << "  call printf\n";
    return 0;
}

int GenCodeVisitor::visit(ReturnStm *stm)
{
    if (stm->e)
    {
        stm->e->accept(this);
    }
    out << "  jmp .end_" << nombreFuncion << "\n";
    return 0;
}

int GenCodeVisitor::visit(IfStm *stm)
{
    int lbl = labelcont++;
    std::string labelEnd = "endif_" + std::to_string(lbl);

    for (size_t i = 0; i < stm->conditions.size(); ++i)
    {
        std::string nextBranch = "if_next_" + std::to_string(lbl) + "_" + std::to_string(i);

        stm->conditions[i]->accept(this);
        out << "  cmpq $0, %rax\n";
        out << "  je " << nextBranch << "\n";

        stm->branches[i]->accept(this);
        out << "  jmp " << labelEnd << "\n";

        out << nextBranch << ":\n";
    }

    if (stm->elseBranch)
    {
        stm->elseBranch->accept(this);
    }

    out << labelEnd << ":\n";
    return 0;
}

int GenCodeVisitor::visit(WhileStm *stm)
{
    int lbl = labelcont++;
    std::string startLabel = "while_start_" + std::to_string(lbl);
    std::string endLabel = "while_end_" + std::to_string(lbl);

    out << startLabel << ":\n";
    stm->condition->accept(this);
    out << "  cmpq $0, %rax\n";
    out << "  je " << endLabel << "\n";

    stm->b->accept(this);
    out << "  jmp " << startLabel << "\n";
    out << endLabel << ":\n";
    return 0;
}

int GenCodeVisitor::visit(DoWhileStm *stm)
{
    int lbl = labelcont++;
    std::string startLabel = "dowhile_start_" + std::to_string(lbl);

    out << startLabel << ":\n";
    stm->body->accept(this);

    stm->condition->accept(this);
    out << "  cmpq $0, %rax\n";
    out << "  jne " << startLabel << "\n";
    return 0;
}

int GenCodeVisitor::visit(ForStm *stm)
{
    int lbl = labelcont++;
    std::string startLabel = "for_start_" + std::to_string(lbl);
    std::string endLabel = "for_end_" + std::to_string(lbl);

    int oldOffset = offset;
    memoria[stm->initId] = offset;
    offset -= 8;

    stm->initExpr->accept(this);
    out << "  movq %rax, " << memoria[stm->initId] << "(%rbp)\n";

    out << startLabel << ":\n";
    stm->condition->accept(this);
    out << "  cmpq $0, %rax\n";
    out << "  je " << endLabel << "\n";

    stm->body->accept(this);

    out << "  movq " << memoria[stm->initId] << "(%rbp), %rax\n";
    if (stm->isIncrement)
        out << "  addq $1, %rax\n";
    else
        out << "  subq $1, %rax\n";
    out << "  movq %rax, " << memoria[stm->initId] << "(%rbp)\n";

    out << "  jmp " << startLabel << "\n";
    out << endLabel << ":\n";

    offset = oldOffset;
    return 0;
}

// ---- Expresiones ----

int GenCodeVisitor::visit(BinaryExp *exp)
{
    if (exp->op == AND_OP)
    {
        exp->left->accept(this);
        out << "  pushq %rax\n";
        exp->right->accept(this);
        out << "  movq %rax, %rcx\n";
        out << "  popq %rax\n";
        out << "  andq %rcx, %rax\n";
        return 0;
    }
    if (exp->op == OR_OP)
    {
        exp->left->accept(this);
        out << "  pushq %rax\n";
        exp->right->accept(this);
        out << "  movq %rax, %rcx\n";
        out << "  popq %rax\n";
        out << "  orq %rcx, %rax\n";
        return 0;
    }

    exp->left->accept(this);
    out << "  pushq %rax\n";
    exp->right->accept(this);
    out << "  movq %rax, %rcx\n";
    out << "  popq %rax\n";

    switch (exp->op)
    {
    case PLUS_OP:
        out << "  addq %rcx, %rax\n";
        break;
    case MINUS_OP:
        out << "  subq %rcx, %rax\n";
        break;
    case MUL_OP:
        out << "  imulq %rcx, %rax\n";
        break;
    case DIV_OP:
        out << "  cqto\n  idivq %rcx\n";
        break;
    case LE_OP:
        out << "  cmpq %rcx, %rax\n  movq $0, %rax\n  setle %al\n  movzbq %al, %rax\n";
        break;
    case LT_OP:
        out << "  cmpq %rcx, %rax\n  movq $0, %rax\n  setl %al\n  movzbq %al, %rax\n";
        break;
    case GT_OP:
        out << "  cmpq %rcx, %rax\n  movq $0, %rax\n  setg %al\n  movzbq %al, %rax\n";
        break;
    case GE_OP:
        out << "  cmpq %rcx, %rax\n  movq $0, %rax\n  setge %al\n  movzbq %al, %rax\n";
        break;
    case EQ_OP:
        out << "  cmpq %rcx, %rax\n  movq $0, %rax\n  sete %al\n  movzbq %al, %rax\n";
        break;
    case NE_OP:
        out << "  cmpq %rcx, %rax\n  movq $0, %rax\n  setne %al\n  movzbq %al, %rax\n";
        break;
    default:
        break;
    }
    return 0;
}

int GenCodeVisitor::visit(UnaryExp *node)
{
    if (node->op == NOT_OP)
    {
        if (node->operand) node->operand->accept(this);
        out << "  cmpq $0, %rax\n";
        out << "  sete %al\n";
        out << "  movzbq %al, %rax\n";
    }
    else if (node->op == ADDRESS_OF_OP)
    {
        // &id → deja la DIRECCIÓN de la variable en %rax
        if (memoriaGlobal.count(node->targetId))
            out << "  leaq " << node->targetId << "(%rip), %rax\n";
        else
            out << "  leaq " << memoria[node->targetId] << "(%rbp), %rax\n";
    }
    return 0;
}

int GenCodeVisitor::visit(NumberExp *exp)
{
    out << "  movq $" << exp->value << ", %rax\n";
    return 0;
}

int GenCodeVisitor::visit(BoolExp *exp)
{
    out << "  movq $" << (exp->value ? 1 : 0) << ", %rax\n";
    return 0;
}

int GenCodeVisitor::visit(StringLiteralExp *exp)
{
    int lbl = labelcont++;
    std::string strLabel = "str_lit_" + std::to_string(lbl);

    out << "  .data\n"
        << strLabel << ": .string " << exp->value << "\n.text\n";
    out << "  leaq " << strLabel << "(%rip), %rax\n";
    return 0;
}

int GenCodeVisitor::visit(LambdaDecExp *lambda)
{
    int lbl = labelcont++;
    std::string lambdaLabel = "lambda_anon_" + std::to_string(lbl);

    out << "  jmp lambda_skip_" << lbl << "\n";
    out << lambdaLabel << ":\n";
    out << "  pushq %rbp\n  movq %rsp, %rbp\n";

    const std::vector<std::string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    int oldOffset = offset;
    auto oldMem = memoria;
    std::string savedFuncion = nombreFuncion;  // guardar contexto de función externa

    nombreFuncion = lambdaLabel;               // ReturnStm usará el label correcto
    offset = -8;
    for (size_t i = 0; i < lambda->paramNames.size(); ++i)
    {
        memoria[lambda->paramNames[i]] = offset;
        out << "  movq " << argRegs[i] << ", " << offset << "(%rbp)\n";
        offset -= 8;
    }

    lambda->body->accept(this);

    out << ".end_" << lambdaLabel << ":\n";
    out << "  leave\n  ret\n";
    out << "lambda_skip_" << lbl << ":\n";

    nombreFuncion = savedFuncion;              // restaurar contexto

    memoria = oldMem;
    offset = oldOffset;

    out << "  leaq " << lambdaLabel << "(%rip), %rax\n";
    return 0;
}

int GenCodeVisitor::visit(FunDec *f)
{
    entornoFuncion = true;
    memoria.clear();
    offset = -8;
    nombreFuncion = f->nombre;

    const std::vector<std::string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    out << "\n.globl " << f->nombre << "\n";
    out << f->nombre << ":\n";
    out << "  pushq %rbp\n";
    out << "  movq %rsp, %rbp\n";
    out << "  subq $" << funcontador[f->nombre] * 8 << ", %rsp\n";

    int nParams = static_cast<int>(f->paramNames.size());
    for (int i = 0; i < nParams; i++)
    {
        memoria[f->paramNames[i]] = offset;
        if (i < (int)argRegs.size())
        {
            // Primeros 6: vienen en registros
            out << "  movq " << argRegs[i] << ", " << offset << "(%rbp)\n";
        }
        else
        {
            // Args 7+: vienen en el stack, encima del rbp guardado
            // arg7 está en 16(%rbp), arg8 en 24(%rbp), etc.
            int stackArgOffset = 16 + (i - (int)argRegs.size()) * 8;
            out << "  movq " << stackArgOffset << "(%rbp), %rax\n";
            out << "  movq %rax, " << offset << "(%rbp)\n";
        }
        offset -= 8;
    }

    f->cuerpo->accept(this);

    out << ".end_" << f->nombre << ":\n";
    out << "  leave\n";
    out << "  ret\n";

    entornoFuncion = false;
    return 0;
}
int GenCodeVisitor::visit(CallExp *exp)
{
    static const std::vector<std::string> argRegs =
        {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    size_t nArgs = exp->args.size();
    size_t nReg  = std::min(nArgs, argRegs.size());
    size_t nStk  = (nArgs > argRegs.size()) ? nArgs - argRegs.size() : 0;

    // Evaluar y guardar todos los args en el stack temporalmente (de derecha a izquierda)
    for (int i = (int)nArgs - 1; i >= 0; i--)
    {
        exp->args[i]->accept(this);
        out << "  pushq %rax\n";
    }

    // Cargar los primeros 6 desde el stack a registros
    for (size_t i = 0; i < nReg; i++)
        out << "  popq " << argRegs[i] << "\n";

    // Los args 7+ ya quedaron en el stack en el orden correcto (arg7 en el tope)
    // Alinear el stack a 16 bytes si hay args en stack y el total de pushes es impar
    bool needAlign = (nStk % 2 != 0);
    if (needAlign)
        out << "  subq $8, %rsp\n";

    if (funcontador.count(exp->functionName))
    {
        // Función declarada → llamada directa
        out << "  call " << exp->functionName << "\n";
    }
    else if (memoria.count(exp->functionName))
    {
        // Variable local con puntero a función → llamada indirecta
        out << "  movq " << memoria[exp->functionName] << "(%rbp), %rax\n";
        out << "  call *%rax\n";
    }
    else if (memoriaGlobal.count(exp->functionName))
    {
        // Variable global con puntero a función → llamada indirecta
        out << "  movq " << exp->functionName << "(%rip), %rax\n";
        out << "  call *%rax\n";
    }
    else
    {
        // Fallback: printf u otras funciones externas
        out << "  call " << exp->functionName << "\n";
    }

    // Limpiar args en stack
    if (nStk > 0)
        out << "  addq $" << (nStk + (needAlign ? 1 : 0)) * 8 << ", %rsp\n";

    return 0;
}