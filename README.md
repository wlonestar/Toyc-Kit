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

Before using Toyc, make sure you have Clang, LLVM (version >= 15, recommended version 16), and Boost installed. On Debian or Ubuntu, you can install them with:

```
sudo apt install clang-16 llvm-16 llvm-16-dev libboost1.81-all-dev
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

3. (**Optional**) Compile essential runtime (if you want to see some print out):

```
make runtime
```

You need have enough permission.

### Usage

To compile examples, use the `scripts/toycc.sh` script:

```
./scripts/toycc.sh example/simple.toyc
```

If compiled in **Release** mode, bytecode and executable files (a.ll and a.exe) will be generated. In **Debug** mode, colored AST tree dump and generated IR will be shown.

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
- **REPL**: Expanding project to include an interpreter for toyc along with a Read-Eval-Print Loop (REPL).
