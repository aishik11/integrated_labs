#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Helper function to run a test and check its output
run_test() {
    local test_file=$1
    shift
    local expected_stack=("$@")

    echo "Running test: $test_file"

    echo "CWD: $(pwd)"
    echo "Assembler path: $(readlink -f ../build/assembler)"
    echo "Running command: ../build/assembler \"$test_file\" \"${test_file%.asm}.bin\""
    # Assemble the .asm file
    ../build/assembler "$test_file" "${test_file%.asm}.bin"

    # Run the VM and capture the output
    output=$(../build/bvm "${test_file%.asm}.bin" --verbose)

    # Extract the stack from the output
    actual_stack_str=$(echo "$output" | awk '/Stack \(top to bottom\):/{flag=1; next} flag{print}' | xargs)
    read -r -a actual_stack <<< "$actual_stack_str"


    # Check if the actual stack starts with the expected stack
    if [ "${#actual_stack[@]}" -lt "${#expected_stack[@]}" ]; then
        echo "Test failed: $test_file"
        echo "Expected stack to have at least ${#expected_stack[@]} elements, but it has ${#actual_stack[@]}."
        echo "Expected stack: ${expected_stack[*]}"
        echo "Actual stack:   ${actual_stack[*]}"
        exit 1
    fi

    for i in "${!expected_stack[@]}"; do
        if [[ "${actual_stack[i]}" != "${expected_stack[i]}" ]]; then
            echo "Test failed: $test_file"
            echo "Expected stack: ${expected_stack[*]}"
            echo "Actual stack:   ${actual_stack[*]}"
            exit 1
        fi
    done

    echo "Test passed: $test_file"
}

# Run all tests
run_test "test_add.asm" "30"
run_test "test_sub.asm" "20"
run_test "test_arithmetic.asm" "0" "1" "10" "50"
run_test "test_bitwise.asm" "2" "10" "-6" "6" "7" "1"
run_test "test_control.asm" "5" "5" "10"
run_test "test_functions.asm" "15"
run_test "test_halt.asm" "(empty)"
run_test "test_memory.asm" "123"
run_test "test_loops.asm" "0" "1" "2" "3" "4" "5"
run_test "test_factorial.asm" "120"


# Clean up the generated .bin files
echo "Cleaning up..."
rm -f *.bin

echo "All pipeline tests passed!"
