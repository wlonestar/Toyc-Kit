#!/bin/bash

clang="clang-16"
RED="\033[1;31m"
RET="\033[0m"

# find the shell script its absolute path
script_dir="$(cd "$(dirname "$0")" && pwd)"
TOYCC="${script_dir}/../build/bin/toycc"
export toycc=$TOYCC

# check if correct number of arguments are provided
if [ "$#" -lt 1 ]; then
  echo -e "${RED}Usage: $0 <src_file> <bytecode_file> <executable_file>${RET}"
  exit 1
fi

# assign arguments to variables
src="$1"
bc="${2:-a.ll}"
exec="${3:-a.exe}"

# 1. build bytecode from source file
$toycc "$src" "$bc"
if [ $? -ne 0 ]; then
  echo -e "${RED}error in running toyc frontend${RET}"
  exit 1
fi

# 2. compile bytecode to executable
$clang -o "$exec" "$bc" -ltoyc
if [ $? -ne 0 ]; then
  echo -e "${RED}error in compiling bytecode${RET}"
  exit 1
fi

# 3. run the executable
./"$exec"
