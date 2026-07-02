// =====================================================================
// DEMO 2: Optimización de Mirilla / Peephole (Patrón 3)
//
// El codegen base genera para leer una variable local:
//   leaq -8(%rbp), %rax     ← calcular dirección
//   movq (%rax), %rax       ← desreferenciar
//
// El peephole detecta este par adyacente y lo colapsa en:
//   movq -8(%rbp), %rax     ← lectura directa (1 instrucción menos)
//
// Este patrón dispara CADA VEZ que se lee una variable local.
// =====================================================================

int doble(int x) {
    int resultado = x + x;
    return resultado;
}

int main() {
    int n = 7;
    int r = doble(n);
    printf("r",r);
    return 0;
}
