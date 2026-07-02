#ifndef AST_PRINTER_H
#define AST_PRINTER_H

// =============================================================================
// ast_printer.h — Impresora del AST en formato JSON
// =============================================================================
// Uso:
//   printASTJson(program, std::cout);
//
// Produce un objeto JSON con la estructura completa del AST, útil para
// la visualización en la interfaz gráfica del compilador.
// =============================================================================

#include <ostream>
#include "ast.h"

// Imprime el AST completo como JSON al stream dado.
void printASTJson(Program* program, std::ostream& out);

#endif // AST_PRINTER_H
