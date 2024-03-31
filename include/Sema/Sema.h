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
  auto CheckHexadecimal(const std::string &value) -> bool;
  auto CheckOctal(const std::string &value) -> bool;

  auto CheckUnaryOperatorType(ExprPtr &rhs, TokenTy type) -> std::string;
  auto CheckBinaryOperatorType(ExprPtr &lhs, ExprPtr &rhs, TokenTy type)
      -> std::string;
  auto CheckShiftOperatorType(ExprPtr &lhs, ExprPtr &rhs, TokenTy type)
      -> std::string;
};

} // namespace toyc

#endif
