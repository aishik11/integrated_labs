; Test multiplication
PUSH 10
PUSH 5
MUL
; Expected result: 50

; Test division
PUSH 100
PUSH 10
DIV
; Expected result: 10

; Test comparison (true)
PUSH 5
PUSH 10
CMP
; Expected result: 1 (true)

; Test comparison (false)
PUSH 10
PUSH 5
CMP
; Expected result: 0 (false)

HALT
