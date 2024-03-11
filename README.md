# Toyc - A Compiler Frontend for a C Language Subset

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## Overview

Toyc is a compiler frontend for a subset of the C language. It's designed for learning compilation principles and understanding LLVM Intermediate Representation (**IR**). The project includes hand-written lexical and syntax analysis, with a focus on simplicity and understanding. Currently, only a small portion of the C language syntax is implemented.

## Features

- **Lexical Analysis**: Hand-written lexer with regex used for integer and floating-point number parsing.
- **Syntax Parsing**: Single-pass compilation with nested lexical analysis.
- **Semantic Analysis**: Combined with syntax parsing, includes type checking, implicit casting, and AST tree construction.
- **Code Generation**: Generates LLVM IR.
- **Error Handling**: Basic error handling; exits on error.
- **Optimization**: Not implemented yet.

## Getting Started

### Prerequisites

Before using Toyc, make sure you have **CMake**, **Ninja**, **Clang**, **LLVM** (version >= 15, recommended version 16), and other dependencies installed. On Debian or Ubuntu, you can install them with:

```
sudo apt install \
  cmake ninja-build \
  clang-16 libclang-16-dev llvm-16 llvm-16-dev \
  libzstd-dev libfmt-dev libreadline-dev
```

### Installation

1. Clone the repository:

```
git clone https:://github.com/wlonestar/toyc.git
```

2. Compile the frontend:

```
make TYPE=Release # or just type make
```

3. Compile essential runtime (if you want to see some print out):

```
make install
```

You need have enough permission.

### Usage

To compile examples, use the provided `toycc.sh` script:

```
./scripts/toycc.sh examples/simple.toyc
```

This command will compile the specified example source file(`simple.toyc`) using the script.

If compiled in **Release** mode, bytecode and executable files (a.ll and a.exe) will be generated. In **Debug** mode, colored AST tree dump and generated IR will be shown.

![](https://image-1305118058.cos.ap-nanjing.myqcloud.com/image/Snipaste_2024-03-07_21-04-24.jpg)

### Running the Frontend Separately

If you want to run the frontend separately, you can use the `toycc` binary directly. Before it you need to set environment variable `toycc`.

```
export toycc=build/bin/toycc
build/bin/toycc <source_file> <bytecode_file>
```

## Running Tests

### Unit Tests

To run unit tests, execute the following command:

```bash
make test-all
```

This command will run all unit tests included in the project and report any failures.

### End-to-End Tests

To run end-to-end tests, execute the following command:

```bash
./script/run_e2e_test.sh
```

These tests cover the entire compilation process, from source code to generated executable, and ensure that the compiler frontend behaves as expected in real-world scenarios.

## Contributing

Contributions to Toyc are welcome from everyone. Whether you're a beginner or an experienced developer, your input is valuable.

Here's how you can contribute:

- **Bug Reports**: Open an issue on GitHub with detailed steps to reproduce the problem.
- **Feature Requests**: Create a new issue to suggest new features or improvements.
- **Code Contributions**: Fork the repository and submit a pull request with your changes. Please follow our contribution guidelines.

Thank you for considering contributing to Toyc!

## To-Do (Future Plans)

Here are some ideas for future improvements and features:

- **REPL**: Expanding project to include an interpreter for toyc along with a Read-Eval-Print Loop (REPL).
- **Optimization**: Implement optimization passes to improve generated code efficiency.

## References

1. Robert Nystrom. *Crafting Interpreters*. Genever Benning (July 28, 2021). from [https://craftinginterpreters.com/](https://craftinginterpreters.com/)

2. LLVM Tutorial. (2023) *My First Language Frontend with LLVM Tutorial*. from [https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)

3. Mukul Rathi. (2019). *bolt*. Github. from [https://github.com/mukul-rathi/bolt](https://github.com/mukul-rathi/bolt)
