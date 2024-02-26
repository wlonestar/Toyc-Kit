//! AST

#ifndef AST_H
#define AST_H

#pragma once

#include <Token.h>

#include <llvm/IR/Value.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace toyc {

enum Side {
  INTERNAL = 1,
  LEAF = 0,
};

struct ASTVisitor;
struct TranslationUnitDecl;
struct Decl;
struct Stmt;
struct Expr;

/* =============================== Expr ==================================== */

struct Expr {
  virtual ~Expr() = default;
  virtual std::string getType() const = 0;
  virtual llvm::Value *accept(ASTVisitor &visitor) = 0;
  virtual void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
                    std::string _p = "") = 0;
};

struct Literal : public Expr {};

struct IntegerLiteral : public Literal {
  int64_t value;
  std::string type;

  IntegerLiteral(int64_t _value, std::string &&_type)
      : value(_value), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct FloatingLiteral : public Literal {
  double value;
  std::string type;

  FloatingLiteral(double _value, std::string &&_type)
      : value(_value), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct CharacterLiteral : public Literal {
  int value;
  std::string type;

  CharacterLiteral(int _value) : value(_value), type("i64") {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct StringLiteral : public Literal {
  std::string value;
  std::string type;

  StringLiteral(std::string &&_value, std::string &&_type)
      : value(std::move(_value)), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct DeclRefExpr : public Expr {
  std::string name;
  std::string type;

  DeclRefExpr(std::string &_type, std::string &_name)
      : type(_type), name(_name) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ParenExpr : public Expr {
  std::unique_ptr<Expr> expr;

  ParenExpr(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct UnaryOperator : public Expr {
  Token op;
  std::unique_ptr<Expr> right;
  std::string type;

  UnaryOperator(Token _op, std::unique_ptr<Expr> _right, std::string &&_type)
      : op(_op), right(std::move(_right)), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct BinaryOperator : public Expr {
  Token op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  std::string type;

  BinaryOperator(Token _op, std::unique_ptr<Expr> _left,
                 std::unique_ptr<Expr> _right, std::string &&_type)
      : op(_op), left(std::move(_left)), right(std::move(_right)),
        type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

/* =============================== Stmt ==================================== */

struct Stmt {
  virtual ~Stmt() = default;
  virtual llvm::Value *accept(ASTVisitor &visitor) = 0;
  virtual void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
                    std::string _p = "") = 0;
};

struct CompoundStmt : public Stmt {
  std::vector<std::unique_ptr<Stmt>> stmts;

  CompoundStmt(std::vector<std::unique_ptr<Stmt>> &&_stmts =
                   std::vector<std::unique_ptr<Stmt>>{})
      : stmts(std::move(_stmts)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ExprStmt : public Stmt {
  std::unique_ptr<Expr> expr;

  ExprStmt(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct DeclStmt : public Stmt {
  std::unique_ptr<Decl> decl;

  DeclStmt(std::unique_ptr<Decl> _decl) : decl(std::move(_decl)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ReturnStmt : public Stmt {
  std::unique_ptr<Expr> expr;

  ReturnStmt(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

/* =============================== Decl ==================================== */

struct Decl {
  virtual ~Decl() = default;
  virtual std::string getType() const = 0;
  virtual void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
                    std::string _p = "") = 0;
};

enum VarScope {
  LOCAL,
  GLOBAL,
};

struct VarDecl : public Decl {
  std::string type;
  std::string name;
  std::unique_ptr<Expr> init;
  VarScope scope;

  VarDecl(std::string &&_type, std::string &&_name, std::unique_ptr<Expr> _init,
          VarScope _scope)
      : type(std::move(_type)), name(std::move(_name)), init(std::move(_init)),
        scope(_scope) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ParamVarDecl : public VarDecl {
  ParamVarDecl(std::string &&_type, std::string &&_name)
      : VarDecl(std::move(_type), std::move(_name), nullptr, LOCAL) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct FunctionDecl : public Decl {
  std::string name;
  std::string type;
  std::vector<std::unique_ptr<Decl>> params;
  std::unique_ptr<Stmt> body;

  FunctionDecl(std::string &&_name, std::string &&_type,
               std::vector<std::unique_ptr<Decl>> &&_params,
               std::unique_ptr<Stmt> _body = nullptr)
      : name(std::move(_name)), type(std::move(_type)),
        params(std::move(_params)), body(std::move(_body)) {}

  std::string getType() const override;
  llvm::Function *accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

/* ========================================================================= */

struct TranslationUnitDecl {
  std::vector<std::unique_ptr<Decl>> decls;

  TranslationUnitDecl(std::vector<std::unique_ptr<Decl>> _decls =
                          std::vector<std::unique_ptr<Decl>>{})
      : decls(std::move(_decls)) {}

  void accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cout, size_t _d = 0, Side _s = LEAF,
            std::string _p = "");
};

} // namespace toyc

#endif
