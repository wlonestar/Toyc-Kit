//! Parser implementation

#include <AST.h>
#include <Parser.h>
#include <Token.h>

#include <llvm/IR/Value.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>

namespace toyc {

std::map<std::string, llvm::Value *> VariableTable;

std::tuple<std::string, integer_t>
Parser::parseIntegerSuffix(std::string &value, int base) {
  try {
    if (value.ends_with("llu") || value.ends_with("llU") ||
        value.ends_with("LLu") || value.ends_with("LLU") ||
        value.ends_with("ull") || value.ends_with("uLL") ||
        value.ends_with("Ull") || value.ends_with("ULL")) {
      return std::make_tuple("unsigned long long",
                             stoull(value, nullptr, base));
    } else if (value.ends_with("ul") || value.ends_with("uL") ||
               value.ends_with("Ul") || value.ends_with("UL") ||
               value.ends_with("lu") || value.ends_with("lU") ||
               value.ends_with("Lu") || value.ends_with("LU")) {
      return std::make_tuple("unsigned long", stoul(value, nullptr, base));
    } else if (value.ends_with("ll") || value.ends_with("LL")) {
      return std::make_tuple("long long", stoll(value, nullptr, base));
    } else if (value.ends_with("l") || value.ends_with("L")) {
      return std::make_tuple("long", stol(value, nullptr, base));
    } else if (value.ends_with("u") || value.ends_with("U")) {
      return std::make_tuple("unsigned int", static_cast<unsigned int>(
                                                 stoul(value, nullptr, base)));
    } else {
      return std::make_tuple("int", stoi(value, nullptr, base));
    }
  } catch (std::out_of_range e) {
    throwParserError(
        "integer literal is too large to be represented in integer type");
    return std::make_tuple("int", -1);
  }
}

std::tuple<std::string, floating_t>
Parser::parseFloatingSuffix(std::string &value, int base) {
  try {
    if (value.ends_with("f") || value.ends_with("F")) {
      return std::make_tuple("float", std::stof(value, nullptr));
    } else if (value.ends_with("l") || value.ends_with("L")) {
      return std::make_tuple("long double", stold(value, nullptr));
    } else {
      return std::make_tuple("double", stod(value, nullptr));
    }
  } catch (std::out_of_range e) {
    throwParserError("magnitude of floating-point constant too large");
    return std::make_tuple("double", 0.1);
  }
}

std::unique_ptr<Expr> Parser::parseIntegerLiteral() {
  auto value_str = previous().value;
  std::string type;
  integer_t value;
  if (checkHexadecimal(value_str)) {
    value_str.erase(0, 2);
    auto tuple = parseIntegerSuffix(value_str, 16);
    type = std::get<0>(tuple);
    value = std::get<1>(tuple);
  } else if (checkOctal(value_str)) {
    value_str.erase(0, 1);
    auto tuple = parseIntegerSuffix(value_str, 8);
    type = std::get<0>(tuple);
    value = std::get<1>(tuple);
  } else if (value_str.starts_with("'")) {
    /// TODO: not support escape sequence yet
    value_str.erase(0, 1); // remove leading '
    value_str.pop_back();  // remove suffix '
    int value = value_str[0];
    // std::stringstream ss(value_str);
    // int value = 0;
    // ss >> value;
    return std::make_unique<CharacterLiteral>(value);
  } else {
    auto tuple = parseIntegerSuffix(value_str, 10);
    type = std::get<0>(tuple);
    value = std::get<1>(tuple);
  }
  return std::make_unique<IntegerLiteral>(value, std::move(type));
}

std::unique_ptr<Expr> Parser::parseFloatingLiteral() {
  auto value_str = previous().value;
  auto [type, value] = parseFloatingSuffix(value_str, 10);
  return std::make_unique<FloatingLiteral>(value, std::move(type));
}

std::unique_ptr<Expr> Parser::parsePrimaryExpression() {
  if (match(INTEGER)) {
    return parseIntegerLiteral();
  }
  if (match(FLOATING)) {
    return parseFloatingLiteral();
  }
  if (match(STRING)) {
    auto value = previous().value;
    /// add a terminator '\0' size
    auto type = fstr("char[{}]", value.size() + 1);
    return std::make_unique<StringLiteral>(std::move(value), std::move(type));
  }
  if (match(IDENTIFIER)) {
    auto token = previous();
    /// TODO: find from variable table
    return std::make_unique<DeclRefExpr>("int", std::move(token.value));
  }
  if (match(LP)) {
    auto expr = parseExpression();
    consume(RP, "expected ')'");
    expr = std::make_unique<ParenExpr>(std::move(expr));
    return expr;
  }
  throwParserError("parse primary expression error");
  return nullptr;
}

std::unique_ptr<Expr> Parser::parseUnaryExpression() {
  if (match({ADD, NOT, SUB})) {
    auto op = previous();
    auto right = parseUnaryExpression();
    return std::make_unique<UnaryOperator>(op, std::move(right));
  }
  return parsePrimaryExpression();
}

std::unique_ptr<Expr> Parser::parseMultiplicativeExpression() {
  auto expr = parseUnaryExpression();
  while (match({MUL, DIV})) {
    auto op = previous();
    auto right = parseUnaryExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseAdditiveExpression() {
  auto expr = parseMultiplicativeExpression();
  while (match({ADD, SUB})) {
    auto op = previous();
    auto right = parseMultiplicativeExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseRelationalExpression() {
  auto expr = parseAdditiveExpression();
  while (match({LE_OP, GE_OP, LA, RA})) {
    auto op = previous();
    auto right = parseAdditiveExpression();

    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseEqualityExpression() {
  auto expr = parseRelationalExpression();
  while (match({EQ_OP, NE_OP})) {
    auto op = previous();
    auto right = parseRelationalExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalAndExpression() {
  auto expr = parseEqualityExpression();
  while (match(AND_OP)) {
    auto op = previous();
    auto right = parseEqualityExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOrExpression() {
  auto expr = parseLogicalAndExpression();
  while (match(OR_OP)) {
    auto op = previous();
    auto right = parseLogicalAndExpression();
    auto type = checkBinaryOperatorType(op.type, expr.get(), right.get());
    expr = std::make_unique<BinaryOperator>(op, std::move(expr),
                                            std::move(right), std::move(type));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::parseAssignmentExpression() {
  if (match(IDENTIFIER)) {
    auto token = previous();
    auto declRef = std::make_unique<DeclRefExpr>("int", std::move(token.value));
    auto op = consume(EQUAL, "expect '=' after identifier");
    auto expr = parseAssignmentExpression();
    return std::make_unique<BinaryOperator>(op, std::move(declRef),
                                            std::move(expr), "int");
  }
  return parseLogicalOrExpression();
}

std::unique_ptr<Expr> Parser::parseExpression() {
  return parseAssignmentExpression();
}

/**
 * parse Decl
 */

std::unique_ptr<Decl> Parser::parseDeclaration() {
  auto type = consume(INT, "expected typedef");
  auto id = consume(IDENTIFIER, "expected identifier");
  auto decl =
      std::make_unique<VarDecl>(std::move(type.value), std::move(id.value));
  if (match(EQUAL)) {
    auto init = parseExpression();
    decl->setInit(std::move(init));
  }
  consume(SEMI, "expected ';' after declaration");
  return decl;
}

std::unique_ptr<Decl> Parser::parse() {
  advance();
  auto decl = parseDeclaration();
  consume(_EOF, "expect end of expression");
  return decl;
}

/**
 * Constructor of Parser Error object
 */

ParserError::ParserError(size_t _line, size_t _col, std::string &_message)
    : line(_line), col(_col),
      message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                   "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                   _line, _col, _message)) {}

ParserError::ParserError(size_t _line, size_t _col, std::string &&_message)
    : line(_line), col(_col),
      message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                   "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                   _line, _col, _message)) {}

} // namespace toyc
