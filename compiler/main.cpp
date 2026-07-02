// =============================================================================
// main.cpp — Punto de entrada del compilador
// =============================================================================
// Flujo de compilación:
//   1. Leer el archivo fuente (.fun)
//   2. Scanner  → token stream
//   3. Parser   → AST
//   4. TypeChecker (dentro de GenCodeVisitor::generar) → análisis semántico
//   5. GenCodeVisitor → emite código x86-64 AT&T en un archivo .s
//
// Uso:
//   ./compilador <archivo.fun>
//
// Produce:
//   <archivo>.s — ensamblador listo para ensamblar con gcc/as
// =============================================================================

#include <fstream>
#include <iostream>
#include <string>

#include "ast.h"
#include "ast_printer.h"
#include "parser.h"
#include "scanner.h"
#include "visitor.h"

int main(int argc, const char* argv[]) {

    // -------------------------------------------------------------------------
    // Validar argumentos
    // -------------------------------------------------------------------------
    if (argc < 2 || argc > 3) {
        std::cerr << "Uso: " << argv[0] << " <archivo_de_entrada> [--ast]\n";
        return 1;
    }

    bool astMode = (argc == 3 && std::string(argv[2]) == "--ast");

    // -------------------------------------------------------------------------
    // Leer archivo fuente
    // -------------------------------------------------------------------------
    std::ifstream infile(argv[1]);

    if (!infile.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo '"
                  << argv[1] << "'\n";
        return 1;
    }

    std::string input;
    std::string line;

    while (std::getline(infile, line)) {
        input += line + '\n';
    }

    infile.close();

    std::cout << "========================================\n";
    std::cout << "      INICIANDO COMPILACION\n";
    std::cout << "========================================\n";
    std::cout << "Archivo: " << argv[1] << "\n\n";

    // -------------------------------------------------------------------------
    // Analisis lexico + sintactico
    // -------------------------------------------------------------------------
    Program* program = nullptr;

    try {
        Scanner scanner(input.c_str());
        Parser parser(&scanner);

        program = parser.parseProgram();

        if (!astMode) {
            std::cout << "[OK] Analisis lexico completado.\n";
            std::cout << "[OK] Analisis sintactico completado.\n\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Compilacion detenida.\n";
        std::cerr << e.what() << "\n";
        return 1;
    }

    // -------------------------------------------------------------------------
    // Modo AST: imprimir el AST como JSON y salir
    // -------------------------------------------------------------------------
    if (astMode) {
        printASTJson(program, std::cout);
        delete program;
        return 0;
    }

    // -------------------------------------------------------------------------
    // Construir nombre del archivo de salida
    // -------------------------------------------------------------------------
    std::string inputFile(argv[1]);

    size_t dotPos = inputFile.find_last_of('.');

    std::string baseName =
        (dotPos == std::string::npos)
            ? inputFile
            : inputFile.substr(0, dotPos);

    std::string outputFile = baseName + ".s";

    std::ofstream outfile(outputFile);

    if (!outfile.is_open()) {
        std::cerr << "Error: no se pudo crear el archivo de salida '"
                  << outputFile << "'\n";

        delete program;
        return 1;
    }

    // -------------------------------------------------------------------------
    // Analisis semantico + generacion de codigo
    // -------------------------------------------------------------------------
    try {

        std::cout << "[INFO] Ejecutando analisis semantico...\n";

        GenCodeVisitor codegen(outfile);

        codegen.generar(program);

        std::cout << "[OK] Analisis semantico completado.\n";
        std::cout << "[OK] Generacion de codigo x86 completada.\n\n";

    } catch (const std::exception& e) {

        std::cerr << "[ERROR] Compilacion detenida.\n";
        std::cerr << e.what() << "\n";

        outfile.close();
        delete program;
        return 1;
    }

    outfile.close();

    std::cout << "========================================\n";
    std::cout << "      COMPILACION EXITOSA\n";
    std::cout << "========================================\n";
    std::cout << "Archivo generado: " << outputFile << "\n";

    delete program;

    return 0;
}