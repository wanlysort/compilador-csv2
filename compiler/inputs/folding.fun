// =====================================================================
// DEMO 1: Plegado de Constantes (Constant Folding)
//
// El compilador evalúa expresiones con literales EN TIEMPO DE COMPILACIÓN.
// En vez de generar instrucciones aritméticas, emite un solo movq $resultado.
//
// Sin optimización:   movq $10, %rax
//                     pushq %rax
//                     movq $5, %rax
//                     movq %rax, %rcx
//                     popq %rax
//                     addq %rcx, %rax      ← 6 instrucciones
//
// Con folding:        movq $15, %rax       ← 1 instrucción
// =====================================================================

int main() {
    int a = 10 + 5;
    int b = 20 - 8;
    int c = 3 * 7;
    int d = 100 / 4;
    printf("a",a);
    printf("b",b);
    printf("c",c);
    printf("d",d);
    return 0;
}
