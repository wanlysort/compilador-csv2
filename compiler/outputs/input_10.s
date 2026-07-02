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
  subq $8, %rsp
  movq $0, %rax
  movq %rax, -16(%rbp)
  movq $1, %rax
  movq %rax, -24(%rbp)
for_start_0:
  leaq -24(%rbp), %rax
  pushq %rax
  movq $5, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setle %al
  movzbq %al, %rax
  cmpq $0, %rax
  je for_end_0
  movq -24(%rbp), %rax
  addq $1, %rax
  movq %rax, -24(%rbp)
  jmp for_start_0
for_end_0:
  leaq -16(%rbp), %rax
  pushq %rax
  movq $10, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setg %al
  movzbq %al, %rax
  cmpq $0, %rax
  je if_next_1_0
  jmp endif_1
if_next_1_0:
endif_1:
  leaq -16(%rbp), %rax
  movq %rax, %rdi
  leaq -8(%rbp), %rax
  movq %rax, %rsi
  call sumar
  movq %rax, -24(%rbp)
  leaq -24(%rbp), %rax
  jmp .end_main
.end_main:
  leave
  ret

.section .note.GNU-stack,"",@progbits
