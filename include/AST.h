//! AST

#ifndef AST_H
#define AST_H

#pragma once

#include <Token.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace toyc {

enum Side {
  INTERNAL = 1,
  LEAF = 0,
};

class Expr {
public:
  virtual ~Expr() = default;
  virtual void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") = 0;
};

class Literal : public Expr {};

class IntegerLiteral : public Literal {
private:
  int64_t value;

public:
  IntegerLiteral(int64_t _value) : value(_value) {}

  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class FloatingLiteral : public Literal {
private:
  double value;

public:
  FloatingLiteral(double _value) : value(_value) {}

  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class StringLiteral : public Literal {
private:
  std::string value;

public:
  StringLiteral(std::string &&_value) : value(std::move(_value)) {}

  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class ParenExpr : public Expr {
private:
  std::unique_ptr<Expr> expr;

public:
  ParenExpr(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

} // namespace toyc

#endif
