//! AST

#ifndef AST_H
#define AST_H

#pragma once

#include <Token.h>

#include <llvm/IR/Value.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace toyc {

enum Side {
  INTERNAL = 1,
  LEAF = 0,
};

class TranslationUnitDecl;
class Decl;
class Stmt;
class Expr;

/**
 * Expr
 */

class Expr {
public:
  virtual ~Expr() = default;

  virtual std::string getType() const = 0;
  virtual llvm::Value *codegen() = 0;
  virtual void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") = 0;
};

class Literal : public Expr {};

class IntegerLiteral : public Literal {
private:
  int64_t value;
  std::string type;

public:
  IntegerLiteral(int64_t _value, std::string &&_type)
      : value(_value), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class FloatingLiteral : public Literal {
private:
  double value;
  std::string type;

public:
  FloatingLiteral(double _value, std::string &&_type)
      : value(_value), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class CharacterLiteral : public Literal {
private:
  int value;

public:
  CharacterLiteral(int _value) : value(_value) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class StringLiteral : public Literal {
private:
  std::string value;
  std::string type;

public:
  StringLiteral(std::string &&_value, std::string &&_type)
      : value(std::move(_value)), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class DeclRefExpr : public Expr {
private:
  std::string type;
  std::string name;

public:
  DeclRefExpr(std::string &_type, std::string &_name)
      : type(_type), name(_name) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class ParenExpr : public Expr {
private:
  std::unique_ptr<Expr> expr;

public:
  ParenExpr(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class UnaryOperator : public Expr {
private:
  Token op;
  std::unique_ptr<Expr> right;
  std::string type;

public:
  UnaryOperator(Token _op, std::unique_ptr<Expr> _right, std::string &&_type)
      : op(_op), right(std::move(_right)), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class BinaryOperator : public Expr {
private:
  Token op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  std::string type;

public:
  BinaryOperator(Token _op, std::unique_ptr<Expr> _left,
                 std::unique_ptr<Expr> _right, std::string &&_type)
      : op(_op), left(std::move(_left)), right(std::move(_right)),
        type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

/**
 * Stmt
 */

class Stmt {
public:
  virtual ~Stmt() = default;

  virtual void codegen() = 0;
  virtual void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") = 0;
};

class CompoundStmt : public Stmt {
private:
  std::vector<std::unique_ptr<Stmt>> stmts;

public:
  CompoundStmt(std::vector<std::unique_ptr<Stmt>> &&_stmts =
                   std::vector<std::unique_ptr<Stmt>>{})
      : stmts(std::move(_stmts)) {}

  void codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class ExprStmt : public Stmt {
private:
  std::unique_ptr<Expr> expr;

public:
  ExprStmt(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  void codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class DeclStmt : public Stmt {
private:
  std::unique_ptr<Decl> decl;

public:
  DeclStmt(std::unique_ptr<Decl> _decl) : decl(std::move(_decl)) {}

  void codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class ReturnStmt : public Stmt {
private:
  std::unique_ptr<Expr> expr;

public:
  ReturnStmt(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  void codegen() override;
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

/**
 * Decl
 */

class Decl {
public:
  virtual ~Decl() = default;

  virtual std::string getType() const = 0;
  virtual void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") = 0;
};

enum VarScope {
  LOCAL,
  GLOBAL,
};

class VarDecl : public Decl {
protected:
  std::string type;
  std::string name;
  std::unique_ptr<Expr> init;
  VarScope scope;

public:
  VarDecl(std::string &&_type, std::string &&_name, std::unique_ptr<Expr> _init,
          VarScope _scope)
      : type(std::move(_type)), name(std::move(_name)), init(std::move(_init)),
        scope(_scope) {}

  std::string getType() const override;
  llvm::Value *codegen();
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class ParamVarDecl : public VarDecl {
public:
  ParamVarDecl(std::string &&_type, std::string &&_name)
      : VarDecl(std::move(_type), std::move(_name), nullptr, LOCAL) {}

  std::string getType() const override;
  llvm::Value *codegen();
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

class FunctionDecl : public Decl {
private:
  std::string name;
  std::string type;
  std::vector<std::unique_ptr<Decl>> params;
  std::unique_ptr<Stmt> body;

public:
  FunctionDecl(std::string &&_name, std::string &&_type,
               std::vector<std::unique_ptr<Decl>> &&_params,
               std::unique_ptr<Stmt> _body)
      : name(std::move(_name)), type(std::move(_type)),
        params(std::move(_params)), body(std::move(_body)) {}

  std::string getType() const override;
  llvm::Function *codegen();
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "") override;
};

/**
 * TranslationUnitDecl
 */

class TranslationUnitDecl {
private:
  std::vector<std::unique_ptr<Decl>> decls;

public:
  TranslationUnitDecl(std::vector<std::unique_ptr<Decl>> _decls =
                          std::vector<std::unique_ptr<Decl>>{})
      : decls(std::move(_decls)) {}

  void codegen();
  void dump(size_t _d = 0, Side _s = LEAF, std::string _p = "");
};

} // namespace toyc

#endif
