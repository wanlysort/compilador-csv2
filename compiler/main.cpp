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
#include <sstream>
#include <string>
#include <vector>
#include <regex>

#include "ast.h"
#include "ast_printer.h"
#include "parser.h"
#include "scanner.h"
#include "visitor.h"

// =============================================================================
// Peephole Optimizer — pasa sobre el ensamblador línea a línea eliminando
// patrones redundantes.
// =============================================================================
std::string peepholeOptimize(const std::string &src)
{
    std::istringstream stream(src);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line))
        lines.push_back(line);

    int removed = 0;
    bool changed = true;
    while (changed)
    {
        changed = false;
        std::vector<std::string> out;
        for (size_t i = 0; i < lines.size(); ++i)
        {
            const std::string &cur  = lines[i];
            const std::string &next = (i + 1 < lines.size()) ? lines[i + 1] : "";

            // Patrón 1: pushq %rax ; popq %rax  → eliminar ambas
            if (cur.find("pushq %rax") != std::string::npos &&
                next.find("popq %rax") != std::string::npos)
            {
                ++i; // saltar las dos líneas
                removed += 2;
                changed = true;
                continue;
            }

            // Patrón 2: movq %rax, X(%rbp) ; movq X(%rbp), %rax → solo el movq de store
            std::smatch m1, m2;
            std::regex storeRe(R"(^\s*movq %rax, (-?\d+\(%rbp\)))");
            std::regex loadRe (R"(^\s*movq (-?\d+\(%rbp\)), %rax)");
            if (std::regex_search(cur, m1, storeRe) &&
                std::regex_search(next, m2, loadRe)  &&
                m1[1] == m2[1])
            {
                out.push_back(cur); // conservar solo el store
                ++i;                // saltar el load redundante
                ++removed;
                changed = true;
                continue;
            }

            // Patrón 3: leaq N(%rbp), %rax ; movq (%rax), %rax → movq N(%rbp), %rax
            // Generado por genLValueAddr() para variables locales simples.
            // La dirección intermedia en %rax es innecesaria si se puede leer directo.
            std::smatch m3;
            std::regex leaqRbpRe(R"(^\s*leaq (-?\d+\(%rbp\)), %rax)");
            std::regex derefRaxRe(R"(^\s*movq \(%rax\), %rax)");
            if (std::regex_search(cur, m3, leaqRbpRe) &&
                std::regex_search(next, derefRaxRe))
            {
                out.push_back("  movq " + m3[1].str() + ", %rax");
                ++i;    // saltar el deref redundante
                ++removed;
                changed = true;
                continue;
            }

            out.push_back(cur);
        }
        lines = out;
    }

    if (removed > 0)
        std::cout << "[OPT] Peephole: " << removed << " instruccion(es) eliminada(s).\n";

    std::string result;
    for (auto &l : lines)
        result += l + "\n";
    return result;
}

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

    // -------------------------------------------------------------------------
    // Analisis semantico + generacion de codigo
    // -------------------------------------------------------------------------
    std::ostringstream asmBuffer;

    try {

        std::cout << "[INFO] Ejecutando analisis semantico...\n";

        GenCodeVisitor codegen(asmBuffer);

        codegen.generar(program);

        std::cout << "[OK] Analisis semantico completado.\n";
        std::cout << "[OK] Generacion de codigo x86 completada.\n\n";

    } catch (const std::exception& e) {

        std::cerr << "[ERROR] Compilacion detenida.\n";
        std::cerr << e.what() << "\n";

        delete program;
        return 1;
    }

    // -------------------------------------------------------------------------
    // Peephole Optimization
    // -------------------------------------------------------------------------
    std::string optimizedAsm = peepholeOptimize(asmBuffer.str());

    // -------------------------------------------------------------------------
    // Escribir archivo de salida
    // -------------------------------------------------------------------------
    std::ofstream outfile(outputFile);

    if (!outfile.is_open()) {
        std::cerr << "Error: no se pudo crear el archivo de salida '"
                  << outputFile << "'\n";
        delete program;
        return 1;
    }

    outfile << optimizedAsm;
    outfile.close();

    std::cout << "========================================\n";
    std::cout << "      COMPILACION EXITOSA\n";
    std::cout << "========================================\n";
    std::cout << "Archivo generado: " << outputFile << "\n";

    delete program;

    return 0;
}