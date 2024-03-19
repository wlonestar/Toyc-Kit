//! toyc semantic analysis

#ifndef SEMA_H
#define SEMA_H

#pragma once

#include <AST/AST.h>
#include <Lexer/Token.h>

#include <memory>
#include <string>

namespace toyc {

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

class Sema {
public:
  Sema() = default;

public:
  bool checkHexadecimal(std::string value);
  bool checkOctal(std::string value);

  std::string checkUnaryOperatorType(ExprPtr &right, TokenTy type);
  std::string checkBinaryOperatorType(ExprPtr &lhs, ExprPtr &rhs, TokenTy type);
  std::string checkShiftOperatorType(ExprPtr &lhs, ExprPtr &rhs, TokenTy type);
};

} // namespace toyc

#endif
