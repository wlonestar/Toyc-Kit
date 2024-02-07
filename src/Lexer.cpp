//! Lexer implementation

#include <Lexer.h>
#include <Token.h>
#include <Util.h>

#include <iostream>
#include <regex>

namespace toyc {

void Lexer::skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
    case '\v':
    case '\f':
      advance();
      break;
    case '\n':
      line++;
      advance();
      /// reset column cursor
      col = 0;
      break;
    case '/':
      if (peekNext() == '/') {
        while (peek() != '\n' && !isEnd()) {
          advance();
        }
      } else if (peekNext() == '*') {
        skipMutliComment();
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

void Lexer::skipMutliComment() {
  advance();
  while (!(peek() == '*' && peekNext() == '/') && !isEnd()) {
    if (peek() == '\n') {
      line++;
      col = 1;
    }
    advance();
  }
  if (isEnd()) {
    throwLexerError("unterminated /* comment");
  }
  advance();
  if (peek() != '/') {
    throwLexerError("unterminated /* comment");
  }
  advance();
}

/// TODO: support (u8|u|U|L) prefix
Token Lexer::scanString() {
  while (peek() != '"' && !isEnd()) {
    if (peek() == '\n') {
      line++;
    }
    advance();
  }
  if (isEnd()) {
    throwLexerError("unterminated string.");
  }
  advance();
  /// remove '"' and '"': "asdasd" -> asdasd
  return makeToken(STRING, input.substr(start + 1, current - start - 2));
}

Token Lexer::scanNumber() {
  std::string _str = input.substr(current - 1);
  std::sregex_iterator iter, end;

  std::regex float6(R"(0[xX][a-fA-F0-9]+\.([pP][+-]?[0-9]+)(f|F|l|L)?)");
  std::regex float4(R"(0[xX][a-fA-F0-9]+([pP][+-]?[0-9]+)(f|F|l|L)?)");
  std::regex int1(
      R"(0[xX][a-fA-F0-9]+(((u|U)(ll|LL|l|L)?)|((ll|LL|l|L)(u|U)?))?)");
  std::regex float5(
      R"(0[xX][a-fA-F0-9]*\.[a-fA-F0-9]+([pP][+-]?[0-9]+)(f|F|l|L)?)");
  std::regex int2(R"([1-9][0-9]*(((u|U)(ll|LL|l|L)?)|((ll|LL|l|L)(u|U)?))?)");
  std::regex int3(R"(0[0-7]*(((u|U)(ll|LL|l|L)?)|((ll|LL|l|L)(u|U)?))?)");
  std::regex float2(R"([0-9]+\.[0-9]+([eE][+-]?[0-9]+)?(f|F|l|L)?)");
  std::regex float3(R"([0-9]+\.([eE][+-]?[0-9]+)?(f|F|l|L)?)");
  std::regex float1(R"([0-9]+([eE][+-]?[0-9]+)(f|F|l|L)?)");
  std::regex float22(R"(\.[0-9]+([eE][+-]?[0-9]+)?(f|F|l|L)?)");
  std::regex int4(
      "'([^'\\\\]|\\\\['\"?\\\\abfnrtv]|\\\\[0-7]{1,3}|\\\\x[0-9a-fA-F]+)'");
  std::regex int44(
      R"(\'([^'\\\n]|(\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+)))+\')");

  if (isHP(previous(), peek())) { // {HP}
    iter = std::sregex_iterator(_str.begin(), _str.end(), float6);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float5);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float4);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), int1);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  } else if (isNZ(previous())) { // {NZ}
    iter = std::sregex_iterator(_str.begin(), _str.end(), int2);
    if (iter != end) {
      forward(iter->str().size() - 1);
      ///---------- !!! trivial !!! ----------///
      if (peek() == '.') {
        backward(iter->str().size() - 1);
        goto DIGIT;
      }
      ///---------- !!! trivial !!! ----------///
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  } else if (isD(previous())) { // {D}
  DIGIT:
    iter = std::sregex_iterator(_str.begin(), _str.end(), float2);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float3);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float1);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    if (previous() == '0') { // "0"
      iter = std::sregex_iterator(_str.begin(), _str.end(), int3);
      if (iter != end) {
        forward(iter->str().size() - 1);
        if (isA(peek())) {
          forward(1);
          throwLexerError(LexerErrorTable[INVALID_INTEGER_SUFFIX]);
        }
        return makeToken(INTEGER);
      }
    }
  } else if (previous() == '.') { // "."
    iter = std::sregex_iterator(_str.begin(), _str.end(), float22);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }
  } else if (isCP(previous())) {
    /// TODO: failed to support all C11-style CharacterLiteral
    iter = std::sregex_iterator(_str.begin(), _str.end(), int4);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  } else if (previous() == '\'') { // "'"
    iter = std::sregex_iterator(_str.begin(), _str.end(), int44);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isA(peek())) {
        forward(1);
        throwLexerError(LexerErrorTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  }
  throwLexerError(LexerErrorTable[INVALID_INTEGER_OR_FLOATING]);
  return makeToken(ERROR);
}

Token Lexer::scanIdentifier() {
  while (isA(peek())) {
    advance();
  }
  auto value = input.substr(start, current - start);
  if (KeywordTable.find(value) != KeywordTable.end()) {
    return makeToken(KeywordTable[value]);
  } else {
    return makeToken(IDENTIFIER);
  }
}

Token Lexer::scanToken() {
  skipWhitespace();
  start = current;
  if (isEnd()) {
    return makeToken(_EOF);
  }

  char c = advance();
  if (isL(c)) {
    return scanIdentifier();
  }
  if (isD(c) || c == '.' || c == '\'') {
    return scanNumber();
  }

  switch (c) {
  case ';':
    return makeToken(SEMI);
  case '{':
    return makeToken(LC);
  case '}':
    return makeToken(RC);
  case ',':
    return makeToken(COMMA);
  case ':':
    return makeToken(COLON);
  case '(':
    return makeToken(LP);
  case ')':
    return makeToken(RP);
  case '[':
    return makeToken(LB);
  case ']':
    return makeToken(RB);
  case '~':
    return makeToken(SIM);
  case '?':
    return makeToken(QUE);
  case '=':
    return makeToken(match('=') ? EQ_OP : EQUAL);
  case '.': {
    if (match('.')) {
      if (match('.')) {
        return makeToken(ELLIPSIS);
      } else {
        return makeToken(ERROR);
      }
    } else {
      return makeToken(DOT);
    }
  }
  case '&': {
    if (match('&')) {
      return makeToken(AND_OP);
    } else if (match('=')) {
      return makeToken(AND_ASSIGN);
    } else {
      return makeToken(AND);
    }
  }
  case '!':
    return makeToken(match('=') ? NE_OP : NOT);
  case '-': {
    if (match('=')) {
      return makeToken(SUB_ASSIGN);
    } else if (match('-')) {
      return makeToken(DEC_OP);
    } else if (match('>')) {
      return makeToken(PTR_OP);
    } else {
      return makeToken(SUB);
    }
  }
  case '+': {
    if (match('+')) {
      return makeToken(INC_OP);
    } else if (match('=')) {
      return makeToken(ADD_ASSIGN);
    } else {
      return makeToken(ADD);
    }
  }
  case '*':
    return makeToken(match('=') ? MUL_ASSIGN : MUL);
  case '/':
    return makeToken(match('=') ? DIV_ASSIGN : DIV);
  case '%':
    return makeToken(match('=') ? MOD_ASSIGN : MOD);
  case '<': {
    if (match('=')) {
      return makeToken(LE_OP);
    } else if (match('<')) {
      if (match('=')) {
        return makeToken(LEFT_ASSIGN);
      } else {
        return makeToken(LEFT_OP);
      }
    } else {
      return makeToken(LA);
    }
  }
  case '>': {
    if (match('=')) {
      return makeToken(GE_OP);
    } else if (match('>')) {
      if (match('=')) {
        return makeToken(RIGHT_ASSIGN);
      } else {
        return makeToken(RIGHT_OP);
      }
    } else {
      return makeToken(RA);
    }
  }
  case '^':
    return makeToken(match('=') ? XOR_ASSIGN : XOR);
  case '|': {
    if (match('=')) {
      return makeToken(OR_ASSIGN);
    } else if (match('|')) {
      return makeToken(OR_OP);
    } else {
      return makeToken(OR);
    }
  }
  case '"':
    return scanString();
  }

  throwLexerError("unexpected character.");
  return Token(ERROR, "");
}

/**
 * Constructor of Lexer Error object
 */

LexerError::LexerError(size_t _line, size_t _col, std::string &_message)
    : line(_line), col(_col),
      message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                   "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                   _line, _col, _message)) {}

LexerError::LexerError(size_t _line, size_t _col, std::string &&_message)
    : line(_line), col(_col),
      message(fstr("\033[1;37mline:{}:col:{}:\033[0m "
                   "\033[1;31merror:\033[0m \033[1;37m{}\033[0m",
                   _line, _col, _message)) {}

} // namespace toyc
