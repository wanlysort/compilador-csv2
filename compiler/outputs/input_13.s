.data
print_val_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text

.globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  subq $0, %rsp
  jmp lambda_skip_0
lambda_anon_0:
  pushq %rbp
  movq %rsp, %rbp
  movq $10, %rax
  jmp .end_main
  leave
  ret
lambda_skip_0:
  leaq lambda_anon_0(%rip), %rax
  movq %rax, -8(%rbp)
  movq $0, %rax
  jmp .end_main
.end_main:
  leave
  ret

.section .note.GNU-stack,"",@progbits
