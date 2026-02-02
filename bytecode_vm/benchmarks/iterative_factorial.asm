; Iterative Factorial of 20
; Expected result: 2432902008176640000

PUSH 20   ; N
STORE 0   ; Store N at memory address 0

PUSH 1    ; ACC (Result)
STORE 1   ; Store ACC at memory address 1

factorial_loop:
    LOAD 0    ; Load N
    PUSH 1
    SUB       ; N = N - 1. (Value is N-1)
    DUP       ; DUP N-1 (make copy for JZ)
    JZ factorial_exit ; If N-1 == 0, exit. (JZ consumes the DUPed N-1)

    LOAD 0    ; Load N (current N)
    LOAD 1    ; Load ACC (current ACC)
    MUL       ; ACC * N
    STORE 1   ; Store new ACC at memory address 1

    LOAD 0    ; Load N
    PUSH 1
    SUB       ; N = N - 1
    STORE 0   ; Store new N at memory address 0
    JMP factorial_loop

factorial_exit:
    LOAD 1    ; Load final ACC (result)
    PEEKPRINT
    HALT
