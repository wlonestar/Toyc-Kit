//! AST

#ifndef AST_H
#define AST_H

#pragma once

#include <Token.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace toyc {

enum Side {
  INTERNAL = 1,
  LEAF = 0,
};

class Expr {
public:
  virtual ~Expr() = default;

  virtual std::string getType() const = 0;
  virtual void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") = 0;
};

class Literal : public Expr {};

using integer_t = std::variant<int, unsigned int, long, unsigned long,
                               long long, unsigned long long>;

class IntegerLiteral : public Literal {
private:
  integer_t value;
  std::string type;

public:
  IntegerLiteral(integer_t _value, std::string &&_type)
      : value(_value), type(std::move(_type)) {}

  std::string getType() const;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

using floating_t = std::variant<float, double, long double>;

class FloatingLiteral : public Literal {
private:
  floating_t value;
  std::string type;

public:
  FloatingLiteral(floating_t _value, std::string &&_type)
      : value(_value), type(std::move(_type)) {}

  std::string getType() const;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class CharacterLiteral : public Literal {
private:
  int value;

public:
  CharacterLiteral(int _value) : value(_value) {}

  std::string getType() const;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class StringLiteral : public Literal {
private:
  std::string value;
  std::string type;

public:
  StringLiteral(std::string &&_value, std::string &&_type)
      : value(std::move(_value)), type(std::move(_type)) {}

  std::string getType() const;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class ParenExpr : public Expr {
private:
  std::unique_ptr<Expr> expr;

public:
  ParenExpr(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  std::string getType() const;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class UnaryOperator : public Expr {
private:
  Token op;
  std::unique_ptr<Expr> right;

public:
  UnaryOperator(Token _op, std::unique_ptr<Expr> _right)
      : op(_op), right(std::move(_right)) {}

  std::string getType() const;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

class BinaryOperator : public Expr {
private:
  Token op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

public:
  BinaryOperator(Token _op, std::unique_ptr<Expr> _left,
                 std::unique_ptr<Expr> _right)
      : op(_op), left(std::move(_left)), right(std::move(_right)) {}

  std::string getType() const;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

} // namespace toyc

#endif
