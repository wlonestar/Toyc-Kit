# Toyc - A C Subset Compiler, Interpreter and REPL

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## Overview

Toyc is a project focused on simplifying the complexities of the C programming language while providing a platform for learning about compiler construction principles and delving into LLVM Intermediate Representation (**LLVM IR**). It offers a compiler, an interpreter and a REPL tailored to the needs of learners and enthusiasts in the field.

## Features

- **Simplified C Subset**: Toyc presents a subset of the C language, making it more approachable for beginners while still covering essential concepts.
- **Hands-on Learning**: The project emphasizes hand-written lexical and syntax analysis, promoting a deeper understanding of compiler internals.
- **Focus on Simplicity**: With simplicity as a core tenet, toyc's design aims to be transparent and easy yo comprehend, making it an ideal tool for educational purposes.
- **Incremental Implementation**: Although currently covering only a small portion of the C language syntax, the project is open to expansion and improvement by contributors.

## Components

- **Compiler**: Converts toyc source code into LLVM IR, facilitating exploration of compilation stages and LLVM's capabilities.
- **Interpreter**: Executes toyc source code line by line directly, offering immediate feedback and enabling experimentation with language features.
- **REPL**: An interactive programming environment.

## Getting Started

### Prerequisites

Before using Toyc, make sure you have the following dependencies installed:

- CMake
- Ninja
- Clang (version >= 15)
- LLVM (version >= 15, recommended version 16)
- libzstd-dev
- libfmt-dev
- libreadline-dev

You can install them on Debian or Ubuntu using the following command:

```
sudo apt install cmake ninja-build clang-16 libclang-16-dev llvm-16 llvm-16-dev libzstd-dev libfmt-dev libreadline-dev
```

### Installation

1. Clone the repository:

```
git clone https:://github.com/wlonestar/toyc.git
```

2. Compile the frontend:

```
make TYPE=Release 
# (Optional) Debug mode
# make
```

1. Compile essential runtime:

```
make install
```

You need to have enough permission.

### Usage

#### Compiler

To compile examples, use the provided `toycc.sh` script:

```
./scripts/toycc.sh examples/simple.toyc
```

This command will compile the specified example source file(`simple.toyc`) using the script.

If compiled in **Release** mode, bytecode and executable files (`a.ll` and `a.exe`) will be generated. In **Debug** mode, colored AST tree dump and generated IR will be shown.

![](https://image-1305118058.cos.ap-nanjing.myqcloud.com/image/Snipaste_2024-03-07_21-04-24.jpg)

**Running the Frontend Separately**

If you want to run the frontend separately, you can use the `toycc` binary directly. Before it you need to set environment variable `toycc`.

```
export toycc=build/bin/toycc
build/bin/toycc <source_file> <bytecode_file>
```

#### Interpreter

To use Interpreter, use the provided `toyci.sh` script:

```
./scripts/toyci.sh examples/repl_test.toyc
```

The Interpreter allows you to execute Toyc source code line by line, providing immediate feedback. This is particularly useful for experimenting with language features and exploring code behavior interactively.

#### REPL

To launch the REPL, execute the following command:

````
./script/toyc-repl.sh
````

The REPL (Read-Eval-Print Loop) provides an interactive programming environment where you can enter Toyc expressions or statements, which are then immediately evaluated, and the results are displayed back to you. This is a convenient way to test code snippets, explore language features, and prototype algorithms interactively.

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

- **Optimization**: Implement optimization passes to improve generated code efficiency.

## References

1. Robert Nystrom. *Crafting Interpreters*. Genever Benning (July 28, 2021). from [https://craftinginterpreters.com/](https://craftinginterpreters.com/)

2. LLVM Tutorial. (2023) *My First Language Frontend with LLVM Tutorial*. from [https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)

3. Mukul Rathi. (2019). *bolt*. Github. from [https://github.com/mukul-rathi/bolt](https://github.com/mukul-rathi/bolt)
