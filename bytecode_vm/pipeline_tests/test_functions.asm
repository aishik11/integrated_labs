; Test CALL and RET
PUSH 10
CALL my_function
HALT

my_function:
    PUSH 5
    ADD
    RET
