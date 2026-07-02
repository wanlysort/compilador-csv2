// =============================================================================
// ast_printer.cpp — Implementación del impresor JSON del AST
// =============================================================================

#include "ast_printer.h"
#include <string>

// -----------------------------------------------------------------------------
// Helpers internos
// -----------------------------------------------------------------------------

static std::string jEsc(const std::string& s) {
    std::string r;
    for (char c : s) {
        switch (c) {
            case '"':  r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n";  break;
            case '\r': r += "\\r";  break;
            case '\t': r += "\\t";  break;
            default:   r += c;
        }
    }
    return r;
}

// Forward declarations
static void jTypeNode   (TypeNode*      tn, std::ostream& o);
static void jInitValue  (InitValueNode* iv, std::ostream& o);
static void jLValue     (LValueNode*    lv, std::ostream& o);
static void jExp        (Exp*            e, std::ostream& o);
static void jVarDec     (VarDec*        vd, std::ostream& o);
static void jStm        (Stm*            s, std::ostream& o);
static void jBody       (Body*           b, std::ostream& o);

// -----------------------------------------------------------------------------
// TypeNode
// -----------------------------------------------------------------------------
static void jTypeNode(TypeNode* tn, std::ostream& o) {
    if (!tn) { o << "null"; return; }
    o << "{\"type\":\"TypeNode\"";
    o << ",\"base\":\""    << jEsc(tn->baseName)    << "\"";
    o << ",\"pointers\":"  << tn->pointerLevel;
    if (!tn->genericType.empty())
        o << ",\"generic\":\"" << jEsc(tn->genericType) << "\"";
    o << "}";
}

// -----------------------------------------------------------------------------
// InitValue
// -----------------------------------------------------------------------------
static void jInitValue(InitValueNode* iv, std::ostream& o) {
    if (!iv) { o << "null"; return; }
    if (iv->expression) {
        o << "{\"type\":\"InitValue\",\"children\":[";
        jExp(iv->expression, o);
        o << "]}";
    } else {
        o << "{\"type\":\"InitList\",\"children\":[";
        for (size_t i = 0; i < iv->listItems.size(); i++) {
            if (i > 0) o << ",";
            jInitValue(iv->listItems[i], o);
        }
        o << "]}";
    }
}

// -----------------------------------------------------------------------------
// LValue
// -----------------------------------------------------------------------------
static void jLValue(LValueNode* lv, std::ostream& o) {
    o << "{\"type\":\"LValue\"";
    o << ",\"deref\":" << lv->dereferenceLevel;
    o << ",\"path\":[";
    for (size_t i = 0; i < lv->memberIds.size(); i++) {
        if (i > 0) o << ",";
        o << "{\"id\":\"" << jEsc(lv->memberIds[i]) << "\"";
        if (i < lv->dimAccesses.size() && !lv->dimAccesses[i].empty()) {
            o << ",\"indices\":[";
            for (size_t j = 0; j < lv->dimAccesses[i].size(); j++) {
                if (j > 0) o << ",";
                jExp(lv->dimAccesses[i][j], o);
            }
            o << "]";
        }
        o << "}";
    }
    o << "]}";
}

// -----------------------------------------------------------------------------
// Expresiones (dispatch con dynamic_cast)
// -----------------------------------------------------------------------------
static void jExp(Exp* e, std::ostream& o) {
    if (!e) { o << "null"; return; }

    if (auto* n = dynamic_cast<NumberExp*>(e)) {
        o << "{\"type\":\"NumberExp\",\"value\":" << n->value << "}";

    } else if (auto* b = dynamic_cast<BoolExp*>(e)) {
        o << "{\"type\":\"BoolExp\",\"value\":" << (b->value ? "true" : "false") << "}";

    } else if (auto* s = dynamic_cast<StringLiteralExp*>(e)) {
        o << "{\"type\":\"StringLiteralExp\",\"value\":\"" << jEsc(s->value) << "\"}";

    } else if (auto* bin = dynamic_cast<BinaryExp*>(e)) {
        o << "{\"type\":\"BinaryExp\",\"op\":\"" << jEsc(Exp::binopToChar(bin->op)) << "\"";
        o << ",\"children\":[";
        jExp(bin->left, o); o << ","; jExp(bin->right, o);
        o << "]}";

    } else if (auto* un = dynamic_cast<UnaryExp*>(e)) {
        std::string opStr = (un->op == NOT_OP) ? "!" : "&";
        o << "{\"type\":\"UnaryExp\",\"op\":\"" << opStr << "\"";
        o << ",\"children\":[";
        if (un->op == ADDRESS_OF_OP && !un->targetId.empty())
            o << "{\"type\":\"Identifier\",\"name\":\"" << jEsc(un->targetId) << "\"}";
        else
            jExp(un->operand, o);
        o << "]}";

    } else if (auto* ce = dynamic_cast<CallExp*>(e)) {
        o << "{\"type\":\"CallExp\",\"name\":\"" << jEsc(ce->functionName) << "\"";
        o << ",\"children\":[";
        for (size_t i = 0; i < ce->args.size(); i++) {
            if (i > 0) o << ",";
            jExp(ce->args[i], o);
        }
        o << "]}";

    } else if (auto* lv = dynamic_cast<LValueNode*>(e)) {
        jLValue(lv, o);

    } else if (auto* la = dynamic_cast<LambdaDecExp*>(e)) {
        o << "{\"type\":\"LambdaDecExp\",\"params\":[";
        for (size_t i = 0; i < la->paramNames.size(); i++) {
            if (i > 0) o << ",";
            o << "{\"name\":\"" << jEsc(la->paramNames[i]) << "\",\"paramType\":";
            jTypeNode(la->paramTypes[i], o);
            o << "}";
        }
        o << "],\"children\":[";
        jBody(la->body, o);
        o << "]}";

    } else {
        o << "{\"type\":\"UnknownExp\"}";
    }
}

// -----------------------------------------------------------------------------
// VarDec
// -----------------------------------------------------------------------------
static void jVarDec(VarDec* vd, std::ostream& o) {
    o << "{\"type\":\"VarDec\",\"varType\":";
    jTypeNode(vd->type, o);
    o << ",\"vars\":[";
    for (size_t i = 0; i < vd->varList.size(); i++) {
        if (i > 0) o << ",";
        VarNode* vn = vd->varList[i];
        o << "{\"type\":\"VarNode\",\"name\":\"" << jEsc(vn->id) << "\"";
        if (!vn->dimensions.empty()) {
            o << ",\"dims\":[";
            for (size_t j = 0; j < vn->dimensions.size(); j++) {
                if (j > 0) o << ",";
                o << vn->dimensions[j];
            }
            o << "]";
        }
        if (vn->initializer) {
            o << ",\"children\":[";
            jInitValue(vn->initializer, o);
            o << "]";
        }
        o << "}";
    }
    o << "]}";
}

// -----------------------------------------------------------------------------
// Sentencias
// -----------------------------------------------------------------------------
static void jStm(Stm* s, std::ostream& o) {
    if (!s) { o << "null"; return; }

    if (auto* as = dynamic_cast<AssignStm*>(s)) {
        o << "{\"type\":\"AssignStm\",\"children\":[";
        jLValue(as->target, o); o << ","; jExp(as->e, o);
        o << "]}";

    } else if (auto* pr = dynamic_cast<PrintfStm*>(s)) {
        o << "{\"type\":\"PrintfStm\",\"format\":\"" << jEsc(pr->format) << "\"";
        o << ",\"children\":["; jExp(pr->e, o); o << "]}";

    } else if (auto* ret = dynamic_cast<ReturnStm*>(s)) {
        o << "{\"type\":\"ReturnStm\",\"children\":[";
        jExp(ret->e, o);
        o << "]}";

    } else if (auto* ifs = dynamic_cast<IfStm*>(s)) {
        o << "{\"type\":\"IfStm\",\"children\":[";
        bool first = true;
        for (size_t i = 0; i < ifs->conditions.size(); i++) {
            if (!first) o << ","; first = false;
            std::string label = (i == 0) ? "IfBranch" : "ElseIfBranch";
            o << "{\"type\":\"" << label << "\",\"children\":[";
            jExp(ifs->conditions[i], o); o << ","; jBody(ifs->branches[i], o);
            o << "]}";
        }
        if (ifs->elseBranch) {
            if (!first) o << ",";
            o << "{\"type\":\"ElseBranch\",\"children\":[";
            jBody(ifs->elseBranch, o);
            o << "]}";
        }
        o << "]}";

    } else if (auto* ws = dynamic_cast<WhileStm*>(s)) {
        o << "{\"type\":\"WhileStm\",\"children\":[";
        o << "{\"type\":\"Condition\",\"children\":["; jExp(ws->condition, o); o << "]}";
        o << ",";
        jBody(ws->b, o);
        o << "]}";

    } else if (auto* dw = dynamic_cast<DoWhileStm*>(s)) {
        o << "{\"type\":\"DoWhileStm\",\"children\":[";
        jBody(dw->body, o); o << ",";
        o << "{\"type\":\"Condition\",\"children\":["; jExp(dw->condition, o); o << "]}";
        o << "]}";

    } else if (auto* fs = dynamic_cast<ForStm*>(s)) {
        o << "{\"type\":\"ForStm\"";
        o << ",\"initId\":\""   << jEsc(fs->initId)   << "\"";
        o << ",\"updateId\":\"" << jEsc(fs->updateId) << "\"";
        o << ",\"increment\":"  << (fs->isIncrement ? "true" : "false");
        o << ",\"children\":[";
        // init type
        jTypeNode(fs->initType, o); o << ",";
        // init expr
        o << "{\"type\":\"InitExpr\",\"children\":["; jExp(fs->initExpr, o); o << "]}";
        o << ",";
        // condition
        o << "{\"type\":\"Condition\",\"children\":["; jExp(fs->condition, o); o << "]}";
        o << ",";
        // body
        jBody(fs->body, o);
        o << "]}";

    } else {
        o << "{\"type\":\"UnknownStm\"}";
    }
}

// -----------------------------------------------------------------------------
// Body
// -----------------------------------------------------------------------------
static void jBody(Body* b, std::ostream& o) {
    if (!b) { o << "null"; return; }
    o << "{\"type\":\"Body\",\"children\":[";

    int decIdx = 0, stmIdx = 0;
    bool first = true;
    for (bool isStm : b->isStatementOrder) {
        if (!first) o << ","; first = false;
        if (isStm) {
            if (stmIdx < (int)b->statements.size())
                jStm(b->statements[stmIdx++], o);
            else
                o << "null";
        } else {
            if (decIdx < (int)b->declarations.size())
                jVarDec(b->declarations[decIdx++], o);
            else
                o << "null";
        }
    }

    o << "]}";
}

// -----------------------------------------------------------------------------
// Punto de entrada público
// -----------------------------------------------------------------------------
void printASTJson(Program* program, std::ostream& out) {
    out << "{\"type\":\"Program\",\"children\":[";

    // --- Structs ---
    out << "{\"type\":\"Structs\",\"children\":[";
    for (size_t i = 0; i < program->sdlist.size(); i++) {
        if (i > 0) out << ",";
        StructDec* sd = program->sdlist[i];
        out << "{\"type\":\"StructDec\",\"name\":\"" << jEsc(sd->nombre) << "\"";
        out << ",\"members\":[";
        for (size_t j = 0; j < sd->memberNames.size(); j++) {
            if (j > 0) out << ",";
            out << "{\"name\":\"" << jEsc(sd->memberNames[j]) << "\",\"memberType\":";
            jTypeNode(sd->memberTypes[j], out);
            out << "}";
        }
        out << "]}";
    }
    out << "]},";

    // --- Globals ---
    out << "{\"type\":\"Globals\",\"children\":[";
    for (size_t i = 0; i < program->vdlist.size(); i++) {
        if (i > 0) out << ",";
        jVarDec(program->vdlist[i], out);
    }
    out << "]},";

    // --- Functions ---
    out << "{\"type\":\"Functions\",\"children\":[";
    for (size_t i = 0; i < program->fdlist.size(); i++) {
        if (i > 0) out << ",";
        FunDec* fd = program->fdlist[i];
        out << "{\"type\":\"FunDec\",\"name\":\"" << jEsc(fd->nombre) << "\"";
        out << ",\"returnType\":"; jTypeNode(fd->returnType, out);
        if (!fd->genericTypeParam.empty())
            out << ",\"generic\":\"" << jEsc(fd->genericTypeParam) << "\"";
        out << ",\"params\":[";
        for (size_t j = 0; j < fd->paramNames.size(); j++) {
            if (j > 0) out << ",";
            out << "{\"name\":\"" << jEsc(fd->paramNames[j]) << "\",\"paramType\":";
            jTypeNode(fd->paramTypes[j], out);
            out << "}";
        }
        out << "],\"children\":[";
        jBody(fd->cuerpo, out);
        out << "]}";
    }
    out << "]}";

    out << "]}";
}
