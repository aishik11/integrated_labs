.global _start

.section .text
_start:
  mov $0x12345678, %rax # Load a distinct value into RAX
  nop                   # Breakpoint here
  mov $60, %rax         # syscall number for exit
  xor %rdi, %rdi        # exit code 0
  syscall