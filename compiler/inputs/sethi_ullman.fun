// =====================================================================
// DEMO 3: Algoritmo de Sethi-Ullman
//
// Calcula el "peso" de cada subárbol para evaluar primero el lado
// más complejo, minimizando el uso del stack.
//
// Expresión: a + (b + (c + 1))
//
// Árbol de pesos (label):
//      +          label(a)=1, label(b+(c+1))=2
//     / \         → derecha más pesada → evaluar derecha PRIMERO
//    a   +        label(b)=1, label(c+1)=1 → igual → izq primero
//       / \
//      b   +      label(c)=1, label(1)=0  → izq primero (estándar)
//         / \
//        c   1
//
// Sin SU:  eval a → push, eval (b+(c+1)) → pop   (stack profundo)
// Con SU:  eval (b+(c+1)) → push, eval a → pop   (reutiliza %rax)
// =====================================================================

int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int resultado = a + (b + (c + 1));
    printf("resultado",resultado);
    return 0;
}
