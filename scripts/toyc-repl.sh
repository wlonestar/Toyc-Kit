#!/bin/bash

RED="\033[1;31m"
RET="\033[0m"

# find the shell script its absolute path
script_dir="$(cd "$(dirname "$0")" && pwd)"
TOYCREPL="${script_dir}/../build/bin/toyc-repl"

$TOYCREPL
