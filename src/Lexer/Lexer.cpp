//! Toyc lexical analyzer implementation

#include <Lexer/Lexer.h>

#include <iostream>
#include <regex>

namespace toyc {

bool Lexer::match(char expected) {
  if (isEnd()) {
    return false;
  }
  if (input.at(current) != expected) {
    return false;
  }
  col++;
  current++;
  return true;
}

void Lexer::forward(size_t steps) {
  current += steps;
  col += steps;
}

void Lexer::backward(size_t steps) {
  current -= steps;
  col -= steps;
}

char Lexer::advance() {
  col++;
  return input.at(current++);
}

char Lexer::peek() {
  if (isEnd()) {
    return '\0';
  }
  return input.at(current);
}

char Lexer::peekNext() {
  if (current + 1 >= input.size()) {
    return '\0';
  }
  return input.at(current + 1);
}

char Lexer::previous() {
  if (current - 1 < 0) {
    return '\0';
  }
  return input.at(current - 1);
}

Token Lexer::makeToken(TokenType type) {
  return Token(type, input.substr(start, current - start), line, col);
}

Token Lexer::makeToken(TokenType type, std::string value) {
  return Token(type, std::move(value), line, col);
}

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
    throwLexerException("unterminated /* comment");
  }
  advance();
  if (peek() != '/') {
    throwLexerException("unterminated /* comment");
  }
  advance();
}

Token Lexer::scanString() {
  while (peek() != '"' && !isEnd()) {
    if (peek() == '\n') {
      line++;
    }
    advance();
  }
  if (isEnd()) {
    throwLexerException("unterminated string.");
  }
  advance();
  /// remove '"' and '"': "asdasd" -> asdasd
  return makeToken(STRING, input.substr(start + 1, current - start - 2));
}

Token Lexer::scanNumber() {
  /// firstly split the possible number literal from the whole input string
  size_t tokSize = 0;
  for (size_t i = current - 1; i < input.size(); i++, tokSize++) {
    if (input[i] == ';' || input[i] == ' ' || input[i] == '\r' ||
        input[i] == '\t' || input[i] == '\n' || input[i] == '\v' ||
        input[i] == '\f') {
      break;
    }
  }
  std::string _str = input.substr(current - 1, tokSize);
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

  if (isHexPreifx(previous(), peek())) { // {HP}
    iter = std::sregex_iterator(_str.begin(), _str.end(), float6);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float5);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float4);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), int1);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  } else if (isNonZero(previous())) { // {NZ}
    iter = std::sregex_iterator(_str.begin(), _str.end(), int2);
    if (iter != end) {
      forward(iter->str().size() - 1);
      ///---------- !!! trivial !!! ----------///
      if (peek() == '.') {
        backward(iter->str().size() - 1);
        goto DIGIT;
      }
      ///---------- !!! trivial !!! ----------///
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  } else if (isDigit(previous())) { // {D}
  DIGIT:
    iter = std::sregex_iterator(_str.begin(), _str.end(), float2);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float3);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    iter = std::sregex_iterator(_str.begin(), _str.end(), float1);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }

    if (previous() == '0') { // "0"
      iter = std::sregex_iterator(_str.begin(), _str.end(), int3);
      if (iter != end) {
        forward(iter->str().size() - 1);
        if (isAlpha(peek())) {
          forward(1);
          throwLexerException(LexerExceptionTable[INVALID_INTEGER_SUFFIX]);
        }
        return makeToken(INTEGER);
      }
    }
  } else if (previous() == '.') { // "."
    iter = std::sregex_iterator(_str.begin(), _str.end(), float22);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_FLOATING_SUFFIX]);
      }
      return makeToken(FLOATING);
    }
  } else if (isCharPrefix(previous())) {
    iter = std::sregex_iterator(_str.begin(), _str.end(), int4);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  } else if (previous() == '\'') { // "'"
    iter = std::sregex_iterator(_str.begin(), _str.end(), int44);
    if (iter != end) {
      forward(iter->str().size() - 1);
      if (isAlpha(peek())) {
        forward(1);
        throwLexerException(LexerExceptionTable[INVALID_INTEGER_SUFFIX]);
      }
      return makeToken(INTEGER);
    }
  }
  throwLexerException(LexerExceptionTable[INVALID_INTEGER_OR_FLOATING]);
  return makeToken(ERROR);
}

Token Lexer::scanIdentifier() {
  while (isAlpha(peek())) {
    advance();
  }
  auto value = input.substr(start, current - start);
  if (KeywordTable.find(value) != KeywordTable.end()) {
    return makeToken(KeywordTable[value]);
  } else {
    return makeToken(IDENTIFIER);
  }
}

void Lexer::addInput(std::string _input) {
  if (input.size() == 0) {
    input = _input;
  } else {
    size_t before = input.size();
    input = input + '\n' + _input;
    /// reset column cursor
    current = before + 1;
    line++;
    col = 0;
  }
}

std::string Lexer::getInput() { return input; }

Token Lexer::scanToken() {
  /// skip whitespace first
  skipWhitespace();
  start = current;
  if (isEnd()) {
    return makeToken(_EOF);
  }
  char c = advance();
  /// scan identifier
  if (isLetter(c)) {
    return scanIdentifier();
  }
  /// scan number
  if (isDigit(c) || c == '.' || c == '\'') {
    return scanNumber();
  }
  /// scan sign
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
  /// throw exception if there is an unexcepted character
  throwLexerException("unexpected character.");
  return Token(ERROR, "");
}

} // namespace toyc
