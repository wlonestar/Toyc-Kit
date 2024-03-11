#!/bin/bash

clang="clang-16"

script_dir="$(cd "$(dirname "$0")" && pwd)"
TOYCC="${script_dir}/../build/bin/toycc"
export toycc=$TOYCC

# create output dir for generated files
mkdir -p build/output

echo "Run e2e test"

total_time=0.0
idx=0
sum=$(find "test/e2e/" -name '*.toyc' | wc -l)

# Iterate over test files
for src_file in $(find "test/e2e/" -name '*.toyc'); do
  ((idx++))
  printf "    Start %d: %s\n" "$idx" "$src_file"

  filename=$(basename -- "$src_file")
  filename="${filename%.*}"
  ll_file="build/output/${filename}.ll"
  ll_expected="test/e2e/${filename}.ll.expected"
  exe_file="build/output/${filename}.exe"
  exe_expected="test/e2e/${filename}.out.expected"

  start=$(date +%s.%N)

  # 1. Run toycc with the test file
  $toycc "$src_file" "$ll_file" 2> /dev/null
  # Compare generated LLVM IR with expected LLVM IR
  diff "$ll_file" "$ll_expected" > /dev/null
  if [ $? -ne 0 ]; then
    echo "Test $filename failed: generated LLVM IR does not match expected LLVM IR"
    exit 1
  fi

  # 2. Run clang to generate executable
  $clang -o "$exe_file" "$ll_file" -ltoyc
  # Run the executable
  result=$("./${exe_file}")
  expected_result=$(cat ${exe_expected})
  # Compare execution result with expected result
  if [ "$result" != "$expected_result" ]; then
    echo "Test $filename failed: execution result does not match expected result"
    exit 1
  fi

  end=$(date +%s.%N)
  duration=$(echo "$end-$start" | bc)
  total_time=$(echo "$total_time" + "$duration" | bc)
  
  printf "%d/%d Test #%d: %-30s Passed %.2f sec\n" "$idx" "$sum" "$idx" "$src_file" "$duration"
done

printf "\n\033[0;32m100%% tests passed\033[0m\n\nTotal Test time (real) = %.2f sec" "$total_time"
