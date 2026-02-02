; Test AND
PUSH 5  ; 0101
PUSH 3  ; 0011
AND     ; Expected result: 1 (0001)

; Test OR
PUSH 5  ; 0101
PUSH 3  ; 0011
OR      ; Expected result: 7 (0111)

; Test XOR
PUSH 5  ; 0101
PUSH 3  ; 0011
XOR     ; Expected result: 6 (0110)

; Test NOT
PUSH 5  ; 0101
NOT     ; Expected result: -6 (in two's complement)

; Test SHL
PUSH 5  ; 0101
PUSH 1  ; shift amount
SHL     ; Expected result: 10 (1010)

; Test SHR
PUSH 5  ; 0101
PUSH 1  ; shift amount
SHR     ; Expected result: 2 (0010)

HALT
