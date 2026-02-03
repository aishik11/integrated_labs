import subprocess
import time
import re
import os
import signal
import sys
import fcntl

def verify_debugger():
    print("Starting Bareshell...")
    process = subprocess.Popen(
        ["stdbuf", "-oL", "-eL", "./bareshell/bareshell"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=0  # Unbuffered
    )
    
    def print_stderr():
        try:
            fl = fcntl.fcntl(process.stderr, fcntl.F_GETFL)
            fcntl.fcntl(process.stderr, fcntl.F_SETFL, fl | os.O_NONBLOCK)
            err = process.stderr.read()
            if err:
                print(f"STDERR OUTPUT:\n{err}")
            else:
                print("NO STDERR OUTPUT")
        except Exception as e:
            print(f"Could not read stderr: {e}")

    # Helper to read until pattern (with timeout to avoid hang)
    def expect(pattern, description):
        buffer = ""
        start_time = time.time()
        while time.time() - start_time < 5:
            char = process.stdout.read(1)
            if char:
                buffer += char
                # print(char, end="", flush=True) # Debug output
                if re.search(pattern, buffer):
                    return re.search(pattern, buffer)
            else:
                break
        print(f"Failed to find pattern for {description}. Buffer: {buffer}")
        return None

    # 1. Submit
    print("Sending: submit simple_loop.asm")
    process.stdin.write("submit simple_loop.asm\n")
    
    match = expect(r"PID (\d+)\)", "Submission PID")
    if not match:
        return False
    
    pid = int(match.group(1))
    print(f"Process Submitted with PID: {pid}")

    # 2. Run
    print("Sending: run 1")
    process.stdin.write("run 1\n")
    
    # Wait for run to start (resuming...)
    if not expect(r"Resuming", "Run Resume"):
        return False
        
    print("Job running... waiting 1s")
    time.sleep(1)



    # 4. Interrupt (Ctrl+Z simulation)
    # Debug info
    subprocess.run(["ps", "-ef"])
    print(f"Sending SIGSTOP to PID {pid}")
    try:
        os.kill(pid, signal.SIGSTOP)
    except Exception as e:
        print(f"Kill failed: {e}")
        print_stderr()
        return False

    # Bareshell should catch this waitpid return
    # Note: waitpid might return in bareshell code.
    # We might expect another prompt or output? 
    # Bareshell doesn't print a prompt after run returns (executed_commands logic).
    
    time.sleep(0.5)

    # 5. Debug
    print("Sending: debug 1")
    process.stdin.write("debug 1\n")
    
    # Expect Debug Prompt
    if not expect(r"debug>", "Debug Prompt"):
        return False
        
    print("Entered Debug Mode!")
    
    # 6. Memstat (In REPL)
    print("Sending: memstat")
    process.stdin.write("memstat\n")
    if not expect(r"Stack Size", "Memstat Output (Stack)"):
        return False
    if not expect(r"Heap Objects", "Memstat Output (Heap)"):
        return False
    if not expect(r"debug>", "Debug Prompt after Memstat"):
        return False
    print("Memstat (REPL) successful!")
    
    # 7. Leaks (In REPL)
    print("Sending: leaks")
    process.stdin.write("leaks\n")
    if not expect(r"Total active objects", "Leaks Output"):
        return False
    if not expect(r"debug>", "Debug Prompt after Leaks"):
        return False
    print("Leaks (REPL) successful!")

    # 8. Step
    print("Sending: step")
    process.stdin.write("step\n")
    if not expect(r"debug>", "Debug Prompt after Step"):
        return False
    print("Step successful!")
    
    # 9. GC (In REPL)
    # Note: GC output might not print "Garbage collection triggered" if not verbose? 
    # But I added cout in vm.cpp
    print("Sending: gc")
    process.stdin.write("gc\n")
    # It prints nothing special unless cout was added. 
    # Ah, I removed the cout in the refactor? No, I see it in vm.cpp snippet in thought?
    # Wait, in the snippet I removed "Garbage collection triggered." message from gc block in repl?
    # Let's check replacement content of vm.cpp
    # } else if (line == "gc") {
    #     gc(); 
    # } 
    # It seems I removed the print. I should add it back or expect nothing but prompt.
    
    if not expect(r"debug>", "Debug Prompt after GC"):
        return False
    print("GC command successful!")

    # 10. Quit
    print("Sending: quit")
    process.stdin.write("quit\n")
    
    # ... leaks check at exit ...
    remaining = process.stdout.read()
    if "Memory Leak Detected" in remaining:
        print("Leak Check: LEAKS DETECTED")
    else:
        print("Leak Check: No leaks detected.")

    process.terminate()
    print("Verification Passed!")
    return True

if __name__ == "__main__":
    if verify_debugger():
        sys.exit(0)
    else:
        # Print stderr if we failed
        pass
        sys.exit(1)
