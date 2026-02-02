; Loop test: Count down from 5 to 0, pushing each value onto the stack
PUSH 5  ; Initial counter

loop_start:
    DUP     ; [..., N, N] - First DUP. The bottom N is for accumulation. The top N for check/decrement.
    DUP     ; [..., N, N, N] - Second DUP. The top N is for JZ.
    JZ loop_exit ; Pops top N. Stack: [..., N, N]. If N was 0, exit.

    PUSH 1  ; Stack: [..., N, 1]
    SUB     ; Stack: [..., N-1] - This is the new N for the next iteration.
    JMP loop_start

loop_exit:
    POP     ; Consume the final 0 that was left by the loop.
    HALT
