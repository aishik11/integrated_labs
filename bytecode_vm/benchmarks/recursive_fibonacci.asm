; Recursive Fibonacci of 20

; main function
PUSH 20
CALL fib
PEEKPRINT
HALT

; fib function
; Expects n on the stack.
fib:
    DUP      ; [n, n]
    PUSH 2   ; [n, n, 2]
    CMP      ; [n, (n < 2 ? 1 : 0)]
    JNZ fib_recursive ; If n is not less than 2 (i.e., n >= 2), jump to recursive case.

    ; Base case (n < 2): returns n itself.
    RET

fib_recursive:
    ; It's a bit tricky to manage the stack for two recursive calls without a frame pointer.
    ; We will use data_memory to store intermediate values.
    ; fib(n) = fib(n-1) + fib(n-2)

    DUP      ; [n, n]
    PUSH 1
    SUB      ; [n, n-1]
    CALL fib ; [n, fib(n-1)] - computes fib(n-1)
    STORE 0  ; Store fib(n-1) in memory[0]. Stack: [n]

    PUSH 2
    SUB      ; [n-2]
    CALL fib ; [fib(n-2)] - computes fib(n-2)

    LOAD 0   ; [fib(n-2), fib(n-1)]
    ADD      ; [fib(n-1) + fib(n-2)]
    RET
