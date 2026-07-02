.data
print_val_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text

.globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  subq $0, %rsp
  movq $15, %rax
  movq %rax, -8(%rbp)
  leaq -8(%rbp), %rax
  pushq %rax
  movq $18, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setge %al
  movzbq %al, %rax
  cmpq $0, %rax
  je if_next_0_0
  jmp endif_0
if_next_0_0:
  leaq -8(%rbp), %rax
  pushq %rax
  movq $14, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setge %al
  movzbq %al, %rax
  cmpq $0, %rax
  je if_next_0_1
  jmp endif_0
if_next_0_1:
endif_0:
  movq $0, %rax
  jmp .end_main
.end_main:
  leave
  ret

.section .note.GNU-stack,"",@progbits
