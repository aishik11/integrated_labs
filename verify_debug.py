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

    # 3. Interrupt (Ctrl+Z simulation)
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

    # 4. Debug
    print("Sending: debug 1")
    process.stdin.write("debug 1\n")
    
    # Expect Debug Prompt
    if not expect(r"debug>", "Debug Prompt"):
        return False
        
    print("Entered Debug Mode!")

    # 5. Step
    print("Sending: step")
    process.stdin.write("step\n")
    
    # Should execute instruction and return to prompt
    # Depending on verbosity, it prints opcode info. 
    # We expect another debug>
    if not expect(r"debug>", "Debug Prompt after Step"):
        return False
    print("Step successful!")

    # 6. Stack
    print("Sending: stack")
    process.stdin.write("stack\n")
    if not expect(r"Stack", "Stack command output"):
        return False
    if not expect(r"debug>", "Debug Prompt after Stack"):
        return False
    print("Stack command successful!")
    
    # 7. Quit
    print("Sending: quit")
    process.stdin.write("quit\n")
    
    process.terminate()
    print("Verification Passed!")
    return True

if __name__ == "__main__":
    if verify_debugger():
        sys.exit(0)
    else:
        # Print stderr if we failed
        # Since process might be running or closed, we captured stderr in Popen?
        # Popen(stderr=PIPE). But we can't read it easily without blocking unless we used communicate() or separate thread.
        # But we can try to peek or just rely on the fact that if it failed, we print "Failed..."
        # Let's verify_debugger return process object or we access it?
        # Simpler: verify_debugger prints what it sees.
        # We can try to read remaining stderr from the process if it's dead.
        pass
        sys.exit(1)
