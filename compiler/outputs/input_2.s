.data
print_val_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text

.globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  subq $0, %rsp
  movq $10, %rax
  movq %rax, -8(%rbp)
  movq $5, %rax
  movq %rax, -16(%rbp)
  leaq -8(%rbp), %rax
  pushq %rax
  leaq -16(%rbp), %rax
  pushq %rax
  movq $2, %rax
  movq %rax, %rcx
  popq %rax
  imulq %rcx, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
  movq %rax, -24(%rbp)
  leaq -24(%rbp), %rax
  jmp .end_main
.end_main:
  leave
  ret

.section .note.GNU-stack,"",@progbits
