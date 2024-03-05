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
build/bin/toycc "$src" "$bc"
if [ $? -ne 0 ]; then
  echo "error in running toyc frontend"
  exit 1
fi

# compile bytecode to executable
clang -mllvm -opaque-pointers -o "$exec" "$bc" -ltoyc
if [ $? -ne 0 ]; then
  echo "error in compiling bytecode"
  exit 1
fi

# run the executable
./"$exec"
if [ $? -ne 0 ]; then
  echo "error in running executable file"
  exit 1
fi
