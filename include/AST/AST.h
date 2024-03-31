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
  virtual auto GetType() const -> std::string = 0;
  virtual auto Assignable() const -> bool = 0;
  virtual auto IsConstant() const -> bool = 0;
  virtual auto Accept(ASTVisitor &visitor) -> llvm::Value * = 0;
  virtual void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
                    const std::string &_p = "") = 0;
};

struct Literal : public Expr {};

struct IntegerLiteral : public Literal {
  int64_t value_;
  std::string type_;

  IntegerLiteral(int64_t value, std::string type)
      : value_(value), type_(std::move(type)) {}

  auto GetType() const -> std::string override { return type_; }
  auto Assignable() const -> bool override { return false; }
  auto IsConstant() const -> bool override { return true; };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct FloatingLiteral : public Literal {
  double value_;
  std::string type_;

  FloatingLiteral(double value, std::string type)
      : value_(value), type_(std::move(type)) {}

  auto GetType() const -> std::string override { return type_; }
  auto Assignable() const -> bool override { return false; }
  auto IsConstant() const -> bool override { return true; };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct CharacterLiteral : public Literal {
  int value_;
  std::string type_;

  explicit CharacterLiteral(int value) : value_(value), type_("i64") {}

  auto GetType() const -> std::string override { return "i64"; }
  auto Assignable() const -> bool override { return false; }
  auto IsConstant() const -> bool override { return true; };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct StringLiteral : public Literal {
  std::string value_;
  std::string type_;

  StringLiteral(std::string value, std::string type)
      : value_(std::move(value)), type_(std::move(type)) {}

  auto GetType() const -> std::string override { return type_; }
  auto Assignable() const -> bool override { return false; }
  auto IsConstant() const -> bool override { return true; };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct DeclRefExpr : public Expr {
  std::unique_ptr<Decl> decl_;

  explicit DeclRefExpr(std::unique_ptr<Decl> _decl) : decl_(std::move(_decl)) {}
  explicit DeclRefExpr(DeclRefExpr *expr) : decl_(std::move(expr->decl_)) {}

  auto GetType() const -> std::string override;
  auto Assignable() const -> bool override { return true; }
  auto IsConstant() const -> bool override { return false; };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct ImplicitCastExpr : public Expr {
  std::string type_;
  std::unique_ptr<Expr> expr_;

  ImplicitCastExpr(std::string type, std::unique_ptr<Expr> expr)
      : type_(std::move(type)), expr_(std::move(expr)) {}

  auto GetType() const -> std::string override { return type_; }
  auto Assignable() const -> bool override { return expr_->Assignable(); }
  auto IsConstant() const -> bool override { return expr_->IsConstant(); };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct ParenExpr : public Expr {
  std::unique_ptr<Expr> expr_;

  explicit ParenExpr(std::unique_ptr<Expr> _expr) : expr_(std::move(_expr)) {}

  auto GetType() const -> std::string override { return expr_->GetType(); }
  auto Assignable() const -> bool override { return expr_->Assignable(); }
  auto IsConstant() const -> bool override { return expr_->IsConstant(); };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct CallExpr : public Expr {
  std::unique_ptr<DeclRefExpr> callee_;
  std::vector<std::unique_ptr<Expr>> args_;

  CallExpr(std::unique_ptr<DeclRefExpr> _callee,
           std::vector<std::unique_ptr<Expr>> _args)
      : callee_(std::move(_callee)), args_(std::move(_args)) {}

  auto GetType() const -> std::string override { return callee_->GetType(); }
  auto Assignable() const -> bool override { return false; }
  auto IsConstant() const -> bool override { return false; };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

enum UnarySide {
  PREFIX,
  POSTFIX,
};

struct UnaryOperator : public Expr {
  Token op_;
  std::unique_ptr<Expr> expr_;
  std::string type_;
  UnarySide side_;

  UnaryOperator(Token _op, std::unique_ptr<Expr> _expr, std::string _type,
                UnarySide _side)
      : op_(std::move(_op)), expr_(std::move(_expr)), type_(std::move(_type)),
        side_(_side) {}

  auto GetType() const -> std::string override { return type_; }
  auto Assignable() const -> bool override { return expr_->Assignable(); }
  auto IsConstant() const -> bool override { return expr_->IsConstant(); };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct BinaryOperator : public Expr {
  Token op_;
  std::unique_ptr<Expr> left_;
  std::unique_ptr<Expr> right_;
  std::string type_;

  BinaryOperator(Token _op, std::unique_ptr<Expr> _left,
                 std::unique_ptr<Expr> _right, std::string _type)
      : op_(std::move(_op)), left_(std::move(_left)), right_(std::move(_right)),
        type_(std::move(_type)) {}

  auto GetType() const -> std::string override { return type_; }
  auto Assignable() const -> bool override { return false; }
  auto IsConstant() const -> bool override {
    return left_->IsConstant() && right_->IsConstant();
  };
  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

/* ================================== Stmt ================================== */

struct Stmt {
  virtual ~Stmt() = default;
  virtual auto Accept(ASTVisitor &visitor) -> llvm::Value * = 0;
  virtual void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
                    const std::string &_p = "") = 0;
};

struct CompoundStmt : public Stmt {
  std::vector<std::unique_ptr<Stmt>> stmts_;

  explicit CompoundStmt(std::vector<std::unique_ptr<Stmt>> &&_stmts =
                            std::vector<std::unique_ptr<Stmt>>{})
      : stmts_(std::move(_stmts)) {}

  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct ExprStmt : public Stmt {
  std::unique_ptr<Expr> expr_;

  explicit ExprStmt(std::unique_ptr<Expr> _expr) : expr_(std::move(_expr)) {}

  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct DeclStmt : public Stmt {
  std::unique_ptr<Decl> decl_;

  explicit DeclStmt(std::unique_ptr<Decl> _decl) : decl_(std::move(_decl)) {}
  explicit DeclStmt(DeclStmt *stmt) : decl_(std::move(stmt->decl_)) {}

  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct IfStmt : public Stmt {
  std::unique_ptr<Expr> cond_;
  std::unique_ptr<Stmt> then_stmt_;
  std::unique_ptr<Stmt> else_stmt_;

  IfStmt(std::unique_ptr<Expr> _cond, std::unique_ptr<Stmt> _thenStmt,
         std::unique_ptr<Stmt> _elseStmt)
      : cond_(std::move(_cond)), then_stmt_(std::move(_thenStmt)),
        else_stmt_(std::move(_elseStmt)) {}

  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct WhileStmt : public Stmt {
  std::unique_ptr<Expr> cond_;
  std::unique_ptr<Stmt> stmt_;

  WhileStmt(std::unique_ptr<Expr> _cond, std::unique_ptr<Stmt> _stmt)
      : cond_(std::move(_cond)), stmt_(std::move(_stmt)) {}

  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct ForStmt : public Stmt {
  std::unique_ptr<DeclStmt> init_;
  std::unique_ptr<Expr> cond_;
  std::unique_ptr<Expr> update_;
  std::unique_ptr<Stmt> body_;

  ForStmt(std::unique_ptr<DeclStmt> _init, std::unique_ptr<Expr> _cond,
          std::unique_ptr<Expr> _update, std::unique_ptr<Stmt> _body)
      : init_(std::move(_init)), cond_(std::move(_cond)),
        update_(std::move(_update)), body_(std::move(_body)) {}

  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct ReturnStmt : public Stmt {
  std::unique_ptr<Expr> expr_;

  explicit ReturnStmt(std::unique_ptr<Expr> _expr) : expr_(std::move(_expr)) {}

  auto Accept(ASTVisitor &visitor) -> llvm::Value * override;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

/* ================================== Decl ================================== */

struct Decl {
  virtual ~Decl() = default;
  virtual auto GetName() const -> std::string = 0;
  virtual auto GetType() const -> std::string = 0;
  virtual void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
                    const std::string &_p = "") = 0;
};

enum VarScope {
  LOCAL,
  GLOBAL,
};

struct VarDecl : public Decl {
  std::string name_;
  std::string type_;
  std::unique_ptr<Expr> init_;
  VarScope scope_;

  VarDecl(std::string _name, std::string _type,
          std::unique_ptr<Expr> _init = nullptr, VarScope _scope = LOCAL)
      : name_(std::move(_name)), type_(std::move(_type)),
        init_(std::move(_init)), scope_(_scope) {}

  explicit VarDecl(VarDecl *decl)
      : name_(decl->name_), type_(decl->type_), init_(std::move(decl->init_)),
        scope_(decl->scope_) {}

  auto GetName() const -> std::string override { return name_; }
  auto GetType() const -> std::string override { return type_; }
  auto Accept(ASTVisitor &visitor) -> llvm::Value *;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

struct ParmVarDecl : public VarDecl {
  ParmVarDecl(std::string _name, std::string _type)
      : VarDecl(std::move(_name), std::move(_type), nullptr, LOCAL) {}

  auto GetName() const -> std::string override { return name_; }
  auto GetType() const -> std::string override { return type_; }
  auto Accept(ASTVisitor &visitor) -> llvm::Type *;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

enum FuncKind {
  DECLARATION, // body is null
  DEFINITION,
  EXTERN_FUNC, // extern function
};

struct FunctionProto {
  std::string name_;
  std::string type_;
  std::vector<std::unique_ptr<ParmVarDecl>> params_;
  size_t refered_;

  FunctionProto(std::string _name, std::string _type,
                std::vector<std::unique_ptr<ParmVarDecl>> _params,
                size_t _refered)
      : name_(std::move(_name)), type_(std::move(_type)),
        params_(std::move(_params)), refered_(_refered) {}
};

struct FunctionDecl : public Decl {
  std::unique_ptr<FunctionProto> proto_;
  std::unique_ptr<Stmt> body_;
  FuncKind kind_;

  explicit FunctionDecl(std::unique_ptr<FunctionProto> _proto,
                        std::unique_ptr<Stmt> _body = nullptr,
                        FuncKind _kind = DEFINITION)
      : proto_(std::move(_proto)), body_(std::move(_body)), kind_(_kind) {}

  auto GetKind() -> FuncKind { return kind_; }

  auto GetName() const -> std::string override { return proto_->name_; }
  auto GetType() const -> std::string override { return proto_->type_; }
  auto Accept(ASTVisitor &visitor) -> llvm::Function *;
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "") override;
};

/* ========================== TranslationUnitDecl =========================== */

struct TranslationUnitDecl {
  std::vector<std::unique_ptr<Decl>> decls_;

  explicit TranslationUnitDecl(std::vector<std::unique_ptr<Decl>> _decls =
                                   std::vector<std::unique_ptr<Decl>>{})
      : decls_(std::move(_decls)) {}

  void Accept(ASTVisitor &visitor);
  void Dump(std::ostream &os = std::cerr, size_t _d = 0, Side _s = LEAF,
            const std::string &_p = "");
};

} // namespace toyc

#endif
