.data
print_val_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text

.globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  subq $0, %rsp
  movq $0, %rax
  jmp .end_main
.end_main:
  leave
  ret

.section .note.GNU-stack,"",@progbits
