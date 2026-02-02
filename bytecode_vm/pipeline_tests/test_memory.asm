; Test STORE and LOAD
PUSH 123  ; Value to store
STORE 0     ; Store 123 at address 0

LOAD 0      ; Load from address 0, expecting 123 on the stack

HALT
