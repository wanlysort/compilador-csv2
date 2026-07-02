.data
print_val_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text

.globl sumar
sumar:
  pushq %rbp
  movq %rsp, %rbp
  subq $16, %rsp
  movq %rdi, -8(%rbp)
  movq %rsi, -16(%rbp)
  leaq -8(%rbp), %rax
  pushq %rax
  leaq -16(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
  jmp .end_sumar
.end_sumar:
  leave
  ret

.globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  subq $0, %rsp
  movq $10, %rax
  movq %rax, %rdi
  movq $20, %rax
  movq %rax, %rsi
  call sumar
  movq %rax, -8(%rbp)
  movq $0, %rax
  jmp .end_main
.end_main:
  leave
  ret

.section .note.GNU-stack,"",@progbits
