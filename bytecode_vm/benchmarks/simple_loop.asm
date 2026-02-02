; Simple loop benchmark
; Loops 1,000,000 times, performing an ADD in each iteration.

PUSH 1000000 ; Counter
STORE 0     ; Store counter in memory

PUSH 0      ; Accumulator for ADD
STORE 1     ; Store accumulator in memory

loop_start:
    LOAD 0      ; Load counter
    JZ loop_exit ; If counter is 0, exit

    LOAD 1      ; Load accumulator
    PUSH 1      ; Push 1 to add
    ADD         ; accumulator + 1
    STORE 1     ; Store new accumulator

    LOAD 0      ; Load counter
    PUSH 1      ; Push 1 to subtract
    SUB         ; counter - 1
    STORE 0     ; Store new counter

    JMP loop_start

loop_exit:
    HALT
