#!/usr/bin/env python3
import subprocess
import time
import csv
import os
import statistics

ASSEMBLER = "../build/assembler"
VM = "../build/bvm"
RESULTS_CSV = "results.csv"

def run_cmd(cmd, timeout=10):
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=timeout)
        if result.returncode != 0:
            return False
        return True
    except subprocess.TimeoutExpired:
        print(f"Timeout expired for command: {cmd}")
        return False

def time_execution(bin_file, timeout=10, runs=5):
    times = []
    for _ in range(runs):
        start = time.perf_counter()
        if not run_cmd(f"{VM} {bin_file}", timeout=timeout):
            return None
        end = time.perf_counter()
        times.append((end - start) * 1000)
    return statistics.median(times)

def generate_asm(name, n, template):
    asm_file = f"{name}_{n}.asm"
    with open(asm_file, "w") as f:
        f.write(template.format(n=n))
    return asm_file

# Templates
TEMPLATES = {
    "add": """
PUSH {n}
loop:
    DUP
    JZ end
    PUSH 1
    SUB
    PUSH 10
    PUSH 20
    ADD
    POP
    JMP loop
end:
    HALT
""",
    "mul": """
PUSH {n}
loop:
    DUP
    JZ end
    PUSH 1
    SUB
    PUSH 10
    PUSH 20
    MUL
    POP
    JMP loop
end:
    HALT
""",
    "push_pop": """
PUSH {n}
loop:
    DUP
    JZ end
    PUSH 1
    SUB
    PUSH 100
    POP
    JMP loop
end:
    HALT
""",
    "load_store": """
PUSH {n}
loop:
    DUP
    JZ end
    PUSH 1
    SUB
    PUSH 123
    STORE 5
    LOAD 5
    POP
    JMP loop
end:
    HALT
"""
}

RECURSIVE_FIB_TEMPLATE = """
PUSH {n}
CALL fib
HALT

fib:
    DUP
    PUSH 2
    CMP
    JZ fib_recursive
    RET
fib_recursive:
    DUP
    PUSH 1
    SUB
    CALL fib
    STORE 0
    PUSH 2
    SUB
    CALL fib
    LOAD 0
    ADD
    RET
"""

RECURSIVE_FACT_TEMPLATE = """
PUSH {n}
CALL fact
HALT

fact:
    DUP
    PUSH 2
    CMP
    JZ fact_recursive
    PUSH 1
    RET
fact_recursive:
    DUP
    PUSH 1
    SUB
    CALL fact
    MUL
    RET
"""

def main():
    results = []
    
    # Operation benchmarks
    for op, template in TEMPLATES.items():
        print(f"Benchmarking operation: {op}")
        for exp in range(1, 7): # 10 to 1,000,000
            n = 10**exp
            asm = generate_asm(op, n, template)
            bin_f = asm.replace(".asm", ".bin")
            if run_cmd(f"{ASSEMBLER} {asm} {bin_f}"):
                t = time_execution(bin_f, timeout=15)
                if t is not None:
                    results.append({"type": "op", "name": op, "n": n, "time_ms": t})
            if os.path.exists(asm): os.remove(asm)
            if os.path.exists(bin_f): os.remove(bin_f)

    # Fibonacci benchmarks
    print("Benchmarking recursive fibonacci")
    for n in range(2, 33, 2): # 2, 4, ..., 32
        asm = generate_asm("fib", n, RECURSIVE_FIB_TEMPLATE)
        bin_f = asm.replace(".asm", ".bin")
        if run_cmd(f"{ASSEMBLER} {asm} {bin_f}"):
            t = time_execution(bin_f, timeout=30)
            if t is not None:
                results.append({"type": "fib", "name": "fib", "n": n, "time_ms": t})
        if os.path.exists(asm): os.remove(asm)
        if os.path.exists(bin_f): os.remove(bin_f)

    # Factorial benchmarks
    print("Benchmarking recursive factorial")
    for n in range(2, 41, 2): # 2, 4, ..., 40
        asm = generate_asm("fact", n, RECURSIVE_FACT_TEMPLATE)
        bin_f = asm.replace(".asm", ".bin")
        if run_cmd(f"{ASSEMBLER} {asm} {bin_f}"):
            t = time_execution(bin_f, timeout=20)
            if t is not None:
                results.append({"type": "fact", "name": "fact", "n": n, "time_ms": t})
        if os.path.exists(asm): os.remove(asm)
        if os.path.exists(bin_f): os.remove(bin_f)

    # Write results to CSV
    with open(RESULTS_CSV, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["type", "name", "n", "time_ms"])
        writer.writeheader()
        writer.writerows(results)
    
    print(f"Results written to {RESULTS_CSV}")

if __name__ == "__main__":
    main()
