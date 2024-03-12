//! AST

#ifndef AST_H
#define AST_H

#pragma once

#include <Lexer/Token.h>

#include <llvm/IR/Type.h>
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
struct VarDecl;
struct Stmt;
struct Expr;

/* ================================== Expr ================================== */

struct Expr {
  virtual ~Expr() = default;
  virtual std::string getType() const = 0;
  virtual llvm::Value *accept(ASTVisitor &visitor) = 0;
  virtual void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
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
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct FloatingLiteral : public Literal {
  double value;
  std::string type;

  FloatingLiteral(double _value, std::string &&_type)
      : value(_value), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct CharacterLiteral : public Literal {
  int value;
  std::string type;

  CharacterLiteral(int _value) : value(_value), type("i64") {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct StringLiteral : public Literal {
  std::string value;
  std::string type;

  StringLiteral(std::string &&_value, std::string &&_type)
      : value(std::move(_value)), type(std::move(_type)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct DeclRefExpr : public Expr {
  std::unique_ptr<Decl> decl;

  DeclRefExpr(std::unique_ptr<Decl> _decl) : decl(std::move(_decl)) {}

  DeclRefExpr(DeclRefExpr *expr) : decl(std::move(expr->decl)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ImplicitCastExpr : public Expr {
  std::string type;
  std::unique_ptr<Expr> expr;

  ImplicitCastExpr(std::string &_type, std::unique_ptr<Expr> _expr)
      : type(_type), expr(std::move(_expr)) {}

  ImplicitCastExpr(std::string &&_type, std::unique_ptr<Expr> _expr)
      : type(std::move(_type)), expr(std::move(_expr)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct CastExpr : public Expr {
  std::string type;
  std::unique_ptr<Expr> expr;

  CastExpr(std::string &_type, std::unique_ptr<Expr> _expr)
      : type(_type), expr(std::move(_expr)) {}

  CastExpr(std::string &&_type, std::unique_ptr<Expr> _expr)
      : type(std::move(_type)), expr(std::move(_expr)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ParenExpr : public Expr {
  std::unique_ptr<Expr> expr;

  ParenExpr(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct CallExpr : public Expr {
  std::unique_ptr<DeclRefExpr> callee;
  std::vector<std::unique_ptr<Expr>> args;

  CallExpr(std::unique_ptr<DeclRefExpr> _callee,
           std::vector<std::unique_ptr<Expr>> _args)
      : callee(std::move(_callee)), args(std::move(_args)) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

enum UnarySide {
  PREFIX,
  POSTFIX,
};

struct UnaryOperator : public Expr {
  Token op;
  std::unique_ptr<Expr> expr;
  std::string type;
  UnarySide side;

  UnaryOperator(Token _op, std::unique_ptr<Expr> _expr, std::string &&_type,
                UnarySide _side)
      : op(_op), expr(std::move(_expr)), type(std::move(_type)), side(_side) {}

  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
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
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

/* ================================== Stmt ================================== */

struct Stmt {
  virtual ~Stmt() = default;
  virtual llvm::Value *accept(ASTVisitor &visitor) = 0;
  virtual void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
                    std::string _p = "") = 0;
};

struct CompoundStmt : public Stmt {
  std::vector<std::unique_ptr<Stmt>> stmts;

  CompoundStmt(std::vector<std::unique_ptr<Stmt>> &&_stmts =
                   std::vector<std::unique_ptr<Stmt>>{})
      : stmts(std::move(_stmts)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ExprStmt : public Stmt {
  std::unique_ptr<Expr> expr;

  ExprStmt(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct DeclStmt : public Stmt {
  std::unique_ptr<Decl> decl;

  DeclStmt(std::unique_ptr<Decl> _decl) : decl(std::move(_decl)) {}

  DeclStmt(DeclStmt *stmt) : decl(std::move(stmt->decl)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct IfStmt : public Stmt {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Stmt> thenStmt;
  std::unique_ptr<Stmt> elseStmt;

  IfStmt(std::unique_ptr<Expr> _cond, std::unique_ptr<Stmt> _thenStmt,
         std::unique_ptr<Stmt> _elseStmt)
      : cond(std::move(_cond)), thenStmt(std::move(_thenStmt)),
        elseStmt(std::move(_elseStmt)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct WhileStmt : public Stmt {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Stmt> stmt;

  WhileStmt(std::unique_ptr<Expr> _cond, std::unique_ptr<Stmt> _stmt)
      : cond(std::move(_cond)), stmt(std::move(_stmt)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ForStmt : public Stmt {
  std::unique_ptr<DeclStmt> init;
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> update;
  std::unique_ptr<Stmt> body;

  ForStmt(std::unique_ptr<DeclStmt> _init, std::unique_ptr<Expr> _cond,
          std::unique_ptr<Expr> _update, std::unique_ptr<Stmt> _body)
      : init(std::move(_init)), cond(std::move(_cond)),
        update(std::move(_update)), body(std::move(_body)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ReturnStmt : public Stmt {
  std::unique_ptr<Expr> expr;

  ReturnStmt(std::unique_ptr<Expr> _expr) : expr(std::move(_expr)) {}

  llvm::Value *accept(ASTVisitor &visitor) override;
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

/* ================================== Decl ================================== */

struct Decl {
  virtual ~Decl() = default;
  virtual std::string getName() const = 0;
  virtual std::string getType() const = 0;
  virtual void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
                    std::string _p = "") = 0;
};

enum VarScope {
  LOCAL,
  GLOBAL,
};

struct VarDecl : public Decl {
  std::string name;
  std::string type;
  std::unique_ptr<Expr> init;
  VarScope scope;

  VarDecl(std::string &_name, std::string &_type,
          std::unique_ptr<Expr> _init = nullptr, VarScope _scope = LOCAL)
      : name(_name), type(_type), init(std::move(_init)), scope(_scope) {}

  VarDecl(std::string &&_name, std::string &&_type,
          std::unique_ptr<Expr> _init = nullptr, VarScope _scope = LOCAL)
      : name(std::move(_name)), type(std::move(_type)), init(std::move(_init)),
        scope(_scope) {}

  std::string getName() const override;
  std::string getType() const override;
  llvm::Value *accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

struct ParmVarDecl : public VarDecl {
  ParmVarDecl(std::string &_name, std::string &_type)
      : VarDecl(_name, _type, nullptr, LOCAL) {}

  ParmVarDecl(std::string &&_name, std::string &&_type)
      : VarDecl(std::move(_name), std::move(_type), nullptr, LOCAL) {}

  std::string getName() const override;
  std::string getType() const override;
  llvm::Type *accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

enum FuncKind {
  DECLARATION, // body is null
  DEFINITION,
  EXTERN_FUNC, // extern function
};

struct FunctionProto {
  std::string name;
  std::string type;
  std::vector<std::unique_ptr<ParmVarDecl>> params;

  FunctionProto(std::string &_name, std::string &_type,
                std::vector<std::unique_ptr<ParmVarDecl>> &_params)
      : name(_name), type(_type), params(std::move(_params)) {}

  FunctionProto(std::string &&_name, std::string &&_type,
                std::vector<std::unique_ptr<ParmVarDecl>> &&_params)
      : name(std::move(_name)), type(std::move(_type)),
        params(std::move(_params)) {}
};

struct FunctionDecl : public Decl {
  std::unique_ptr<FunctionProto> proto;
  std::unique_ptr<Stmt> body;
  FuncKind kind;

  FunctionDecl(std::unique_ptr<FunctionProto> _proto,
               std::unique_ptr<Stmt> _body = nullptr,
               FuncKind _kind = DEFINITION)
      : proto(std::move(_proto)), body(std::move(_body)), kind(_kind) {}

  FuncKind getKind() { return kind; }

  std::string getName() const override;
  std::string getType() const override;
  llvm::Function *accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "") override;
};

/* ========================== TranslationUnitDecl =========================== */

struct TranslationUnitDecl {
  std::vector<std::unique_ptr<Decl>> decls;

  TranslationUnitDecl(std::vector<std::unique_ptr<Decl>> _decls =
                          std::vector<std::unique_ptr<Decl>>{})
      : decls(std::move(_decls)) {}

  void accept(ASTVisitor &visitor);
  void dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            std::string _p = "");
};

} // namespace toyc

#endif
