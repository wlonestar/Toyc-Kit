// ! interpreter

#ifndef INTERPRETER_H
#define INTERPRETER_H

#pragma once

#include <Lexer.h>
#include <Token.h>
#include <Util.h>

#include <exception>
#include <iostream>
#include <string>

namespace toyc {

class Interpreter {
public:
  Interpreter() = default;

  void compile(std::string &input) {
    Lexer lexer(input);
    for (;;) {
      try {
        Token token = lexer.scanToken();
        if (token.type == _EOF) {
          break;
        }
        debug("{}", token.toString());
      } catch (LexerError e) {
        std::cerr << e.what() << "\n";
        break;
      }
    }
  }
};

} // namespace toyc

#endif
