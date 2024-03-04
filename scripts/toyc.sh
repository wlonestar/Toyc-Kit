#!/bin/bash

# check if correct number of arguments are provided
if [ "$#" -ne 3 ]; then
  echo "Usage: $0 <src_file> <bytecode_file> <executable_file>"
  exit 1
fi

# assign arguments to variables
src="$1"
bc="$2"
exec="$3"

# build bytecode from source file
build/bin/toyc "$src" "$bc"
# compile bytecode to executable
clang -mllvm -opaque-pointers -o "$exec" "$bc" -ltoyc
# run the executable
./"$exec"
