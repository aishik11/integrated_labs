#!/bin/bash

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
ASSEMBLER="$SCRIPT_DIR/../../build/assembler"
TEST_DIR="$SCRIPT_DIR"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

passed=0
failed=0

for test_file in $(find $TEST_DIR -name "*.asm"); do
    test_name=$(basename $test_file .asm)
    expected_file="$TEST_DIR/$test_name.bin.expected"
    output_file="$TEST_DIR/$test_name.bin"

    echo "Running test: $test_name"

    # Run the assembler
    $ASSEMBLER $test_file $output_file
    if [ $? -ne 0 ]; then
        echo -e "${RED}FAIL:${NC} Assembler returned a non-zero exit code for $test_name."
        failed=$((failed + 1))
        continue
    fi

    # Check if expected file exists
    if [ ! -f $expected_file ]; then
        echo -e "${RED}FAIL:${NC} Expected output file $expected_file not found for $test_name."
        failed=$((failed + 1))
        continue
    fi

    # Compare the output with the expected file
    if diff -q $output_file $expected_file > /dev/null; then
        echo -e "${GREEN}PASS:${NC} $test_name"
        passed=$((passed + 1))
    else
        echo -e "${RED}FAIL:${NC} $test_name"
        echo "Generated output:"
        xxd $output_file
        echo "Expected output:"
        xxd $expected_file
        failed=$((failed + 1))
    fi

    # Clean up the generated binary
    rm $output_file
done

echo
echo "--------------------"
echo "Test Summary:"
echo -e "${GREEN}Passed: $passed${NC}"
echo -e "${RED}Failed: $failed${NC}"
echo "--------------------"

if [ $failed -ne 0 ]; then
    exit 1
fi
