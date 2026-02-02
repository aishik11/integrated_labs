#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Temporary file for timing results
TIMING_FILE=$(mktemp)
trap 'rm -f $TIMING_FILE' EXIT

# Assemble all benchmark files
echo "Assembling benchmarks..."
for bench_file in *.asm; do
    ../build/assembler "$bench_file" "${bench_file%.asm}.bin"
done

# Run benchmarks
echo ""
echo "Running benchmarks..."
echo "-------------------"

run_benchmark() {
    local bin_file=$1
    echo -n "Benchmarking $bin_file..."
    
    # Run the time command and capture stderr (where time outputs)
    { time ../build/bvm "$bin_file" > /dev/null; } 2>> $TIMING_FILE
    
    echo " Done."
}

run_benchmark "simple_loop.bin"
run_benchmark "iterative_factorial.bin"
run_benchmark "recursive_fibonacci.bin"

echo ""
echo "--------------------"
echo "Benchmark Results:"
echo "--------------------"
printf "% -25s | % -10s\n" "Benchmark" "Time (real)"
printf "% -25s | % -10s\n" "-------------------------" "----------"

# Parse and print results
awk '
BEGIN { benchmark_index=0; benchmarks[0]="simple_loop"; benchmarks[1]="iterative_factorial"; benchmarks[2]="recursive_fibonacci"; }
/real/ { 
    time_val=$2; 
    gsub(/0m/, "", time_val); 
    gsub(/s/, "", time_val); 
    printf "% -25s | % -10s\n", benchmarks[benchmark_index], time_val"s"; 
    benchmark_index++; 
}
' $TIMING_FILE

echo "--------------------"


# Clean up
rm -f *.bin