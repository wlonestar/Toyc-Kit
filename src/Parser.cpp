//! Parser implementation

#include <Parser.h>
#include <Token.h>
#include <memory>

namespace toyc {

std::unique_ptr<Expr> Parser::parsePrimaryExpression() {
  if (match(INTEGER)) {
    return std::make_unique<IntegerLiteral>(stoi(previous().value));
  }
  if (match(FLOATING)) {
    return std::make_unique<FloatingLiteral>(stod(previous().value));
  }
  if (match(STRING)) {
    return std::make_unique<StringLiteral>(previous().value);
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

std::unique_ptr<Expr> Parser::parseExpression() {
  return parsePrimaryExpression();
}

/**
 * Constructor of Parser Error object
 */

ParserError::ParserError(size_t _line, size_t _col, std::string &_message)
    : line(_line), col(_col),
      message(fmt_str("\033[1;37mline:{}:col:{}:\033[0m "
                      "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                      _line, _col, _message)) {}

ParserError::ParserError(size_t _line, size_t _col, std::string &&_message)
    : line(_line), col(_col),
      message(fmt_str("\033[1;37mline:{}:col:{}:\033[0m "
                      "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                      _line, _col, _message)) {}

} // namespace toyc
