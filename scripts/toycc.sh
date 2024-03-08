#!/bin/bash

TOYCC='build/bin/toycc'
CLANG='clang-16'
RED='\033[1;31m'
RET='\033[0m'

# check if correct number of arguments are provided
if [ "$#" -lt 1 ]; then
  echo -e "${RED}Usage: $0 <src_file> <bytecode_file> <executable_file>${RET}"
  exit 1
fi

# assign arguments to variables
src="$1"
bc="${2:-a.ll}"
exec="${3:-a.exe}"

# build bytecode from source file
$TOYCC "$src" "$bc"
if [ $? -ne 0 ]; then
  echo -e "${RED}error in running toyc frontend${RET}"
  exit 1
fi

# compile bytecode to executable
$CLANG -o "$exec" "$bc" -ltoyc
if [ $? -ne 0 ]; then
  echo -e "${RED}error in compiling bytecode${RET}"
  exit 1
fi

# run the executable
./"$exec"
