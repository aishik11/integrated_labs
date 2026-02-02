.global _start

.section .text
_start:
  xor %rcx, %rcx        # Counter = 0
loop_start:
  inc %rcx              # Increment counter
  cmp $5, %rcx          # Compare counter to 5
  jne loop_start        # Jump if not equal
  
  mov $60, %rax         # exit
  xor %rdi, %rdi
  syscall
