#!/bin/bash

RED="\033[1;31m"
RET="\033[0m"

# find the shell script its absolute path
script_dir="$(cd "$(dirname "$0")" && pwd)"
TOYCI="${script_dir}/../build/bin/toyci"

# check if correct number of arguments are provided
if [ "$#" -lt 1 ]; then
  echo -e "${RED}Usage: $0 <src_file>${RET}"
  exit 1
fi

# assign arguments to variables
src="$1"

# 1. build bytecode from source file
$TOYCI "$src"
if [ $? -ne 0 ]; then
  echo -e "${RED}error in running toyc frontend${RET}"
  exit 1
fi
