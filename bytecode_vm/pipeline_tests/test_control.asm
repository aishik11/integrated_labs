; Test JMP
PUSH 10
JMP end_jmp
PUSH 20 ; This should be skipped
end_jmp:

; Test JZ (jump if zero)
PUSH 0
JZ jump_target_jz
PUSH 10 ; This should be skipped
jump_target_jz:
PUSH 5

; Test JNZ (jump if not zero)
PUSH 1
JNZ jump_target_jnz
PUSH 10 ; This should be skipped
jump_target_jnz:
PUSH 5

HALT
