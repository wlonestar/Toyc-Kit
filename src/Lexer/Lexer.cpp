//! Toyc lexical analyzer implementation

#include <Lexer/Lexer.h>

#include <iostream>
#include <regex>

namespace toyc {

auto Lexer::Match(char expected) -> bool {
  if (IsEnd()) {
    return false;
  }
  if (input_.at(current_) != expected) {
    return false;
  }
  col_++;
  current_++;
  return true;
}

void Lexer::Forward(size_t steps) {
  current_ += steps;
  col_ += steps;
}

void Lexer::Backward(size_t steps) {
  current_ -= steps;
  col_ -= steps;
}

auto Lexer::Advance() -> char {
  if (!IsEnd()) {
    col_++;
    return input_.at(current_++);
  }
  return '\0';
}

auto Lexer::Peek() -> char {
  if (IsEnd()) {
    return '\0';
  }
  return input_.at(current_);
}

auto Lexer::PeekNext() -> char {
  if (current_ + 1 >= input_.size()) {
    return '\0';
  }
  return input_.at(current_ + 1);
}

auto Lexer::Previous() -> char {
  if (current_ - 1 < 0) {
    return '\0';
  }
  return input_.at(current_ - 1);
}

auto Lexer::MakeToken(TokenTy type) -> Token {
  return {type, input_.substr(start_, current_ - start_), line_, col_};
}

auto Lexer::MakeToken(TokenTy type, std::string value) -> Token {
  return {type, std::move(value), line_, col_};
}

void Lexer::SkipWhitespace() {
  for (;;) {
    char c = Peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
    case '\v':
    case '\f':
      Advance();
      break;
    case '\n':
      line_++;
      Advance();
      /// reset col_umn cursor
      col_ = 0;
      break;
    case '/':
      if (PeekNext() == '/') {
        while (Peek() != '\n' && !IsEnd()) {
          Advance();
        }
      } else if (PeekNext() == '*') {
        SkipMutliComment();
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

void Lexer::SkipMutliComment() {
  Advance();
  while (!(Peek() == '*' && PeekNext() == '/') && !IsEnd()) {
    if (Peek() == '\n') {
      line_++;
      col_ = 1;
    }
    Advance();
  }
  if (IsEnd()) {
    throw LexerException(line_, col_, "unterminated /* comment");
  }
  Advance();
  if (Peek() != '/') {
    throw LexerException(line_, col_, "unterminated /* comment");
  }
  Advance();
}

auto Lexer::ScanString() -> Token {
  while (Peek() != '"' && !IsEnd()) {
    if (Peek() == '\n') {
      line_++;
    }
    Advance();
  }
  if (IsEnd()) {
    throw LexerException(line_, col_, "unterminated string.");
  }
  Advance();
  /// remove '"' and '"': "asdasd" -> asdasd
  return MakeToken(STRING, input_.substr(start_ + 1, current_ - start_ - 2));
}

auto Lexer::ScanNumber() -> Token {
  /// firstly split the possible number literal from the whole input_ string
  size_t tok_size = 0;
  for (size_t i = current_ - 1; i < input_.size(); i++, tok_size++) {
    if (input_[i] == ';' || input_[i] == ',' || input_[i] == ' ' ||
        input_[i] == '\r' || input_[i] == '\t' || input_[i] == '\n' ||
        input_[i] == '\v' || input_[i] == '\f') {
      break;
    }
  }
  std::string str = input_.substr(current_ - 1, tok_size);
  std::sregex_iterator iter;
  std::sregex_iterator end;

  std::regex int_re(R"((0[xX][a-fA-F0-9]+)|([1-9][0-9]*)|(0[0-7]*))");
  std::regex float_re(
      R"(([0-9]+([eE][+-]?[0-9]+))|([0-9]*\.[0-9]+([eE][+-]?[0-9]+)?)|([0-9]+\.([eE][+-]?[0-9]+)?))");

  if (StrContains(str, '.') || StrContains(str, 'e') || StrContains(str, 'E')) {
    iter = std::sregex_iterator(str.begin(), str.end(), float_re);
    if (iter != end) {
      Forward(iter->str().size() - 1);
      if (IsAlpha(Peek())) {
        Forward(1);
        throw LexerException(line_, col_,
                             "invalid suffix on floating constant");
      }
    }
    return MakeToken(FLOATING);
  }

  iter = std::sregex_iterator(str.begin(), str.end(), int_re);
  if (iter != end) {
    Forward(iter->str().size() - 1);
    if (IsAlpha(Peek())) {
      Forward(1);
      throw LexerException(line_, col_, "invalid suffix on integer constant");
    }
  }
  return MakeToken(INTEGER);
}

auto Lexer::ScanIdentifier() -> Token {
  while (IsAlpha(Peek())) {
    Advance();
  }
  auto value = input_.substr(start_, current_ - start_);
  if (keyword_table.find(value) != keyword_table.end()) {
    return MakeToken(keyword_table[value]);
  }
  return MakeToken(IDENTIFIER);
}

void Lexer::AddInput(const std::string &input) {
  if (input.empty()) {
    input_ = input;
  } else {
    size_t before = input_.size();
    input_ = input_ + '\n' + input;
    /// reset col_umn cursor
    current_ = before + 1;
    line_++;
    col_ = 0;
  }
}

auto Lexer::GetInput() -> std::string { return input_; }

auto Lexer::ScanToken() -> Token {
  /// skip whitespace first
  SkipWhitespace();
  start_ = current_;
  if (IsEnd()) {
    return MakeToken(_EOF);
  }
  char c = Advance();
  /// Scan identifier
  if (IsLetter(c)) {
    return ScanIdentifier();
  }
  /// Scan number
  if (IsDigit(c) || c == '.') {
    return ScanNumber();
  }
  /// Scan sign
  switch (c) {
  case ';':
    return MakeToken(SEMI);
  case '{':
    return MakeToken(LC);
  case '}':
    return MakeToken(RC);
  case ',':
    return MakeToken(COMMA);
  case ':':
    return MakeToken(COLON);
  case '(':
    return MakeToken(LP);
  case ')':
    return MakeToken(RP);
  case '[':
    return MakeToken(LB);
  case ']':
    return MakeToken(RB);
  case '~':
    return MakeToken(SIM);
  case '?':
    return MakeToken(QUE);
  case '=':
    return MakeToken(Match('=') ? EQ_OP : EQUAL);
  case '.': {
    if (Match('.')) {
      if (Match('.')) {
        return MakeToken(ELLIPSIS); /* ... */
      }
      throw LexerException(line_, col_, "expected parameter declarator");
    }
    return MakeToken(DOT); /* . */
  }
  case '&': {
    if (Match('&')) {
      return MakeToken(AND_OP); /* && */
    }
    if (Match('=')) {
      return MakeToken(AND_ASSIGN); /* &= */
    }
    return MakeToken(AND); /* & */
  }
  case '!':
    return MakeToken(Match('=') ? NE_OP : NOT);
  case '-': {
    if (Match('=')) {
      return MakeToken(SUB_ASSIGN); /* -= */
    }
    if (Match('-')) {
      return MakeToken(DEC_OP); /* -- */
    }
    if (Match('>')) {
      return MakeToken(PTR_OP); /* -> */
    }
    return MakeToken(SUB); /* - */
  }
  case '+': {
    if (Match('+')) {
      return MakeToken(INC_OP); /* ++ */
    }
    if (Match('=')) {
      return MakeToken(ADD_ASSIGN); /* += */
    }
    return MakeToken(ADD); /* + */
  }
  case '*':
    return MakeToken(Match('=') ? MUL_ASSIGN : MUL);
  case '/':
    return MakeToken(Match('=') ? DIV_ASSIGN : DIV);
  case '%':
    return MakeToken(Match('=') ? MOD_ASSIGN : MOD);
  case '<': {
    if (Match('=')) {
      return MakeToken(LE_OP); /* <= */
    }
    if (Match('<')) {
      if (Match('=')) {
        return MakeToken(LEFT_ASSIGN); /* <<= */
      }
      return MakeToken(LEFT_OP); /* << */
    }
    return MakeToken(LT); /* < */
  }
  case '>': {
    if (Match('=')) {
      return MakeToken(GE_OP); /* >= */
    }
    if (Match('>')) {
      if (Match('=')) {
        return MakeToken(RIGHT_ASSIGN); /* >>= */
      }
      return MakeToken(RIGHT_OP); /* >> */
    }
    return MakeToken(GT); /* > */
  }
  case '^':
    return MakeToken(Match('=') ? XOR_ASSIGN : XOR);
  case '|': {
    if (Match('=')) {
      return MakeToken(OR_ASSIGN); /* |= */
    }
    if (Match('|')) {
      return MakeToken(OR_OP); /* || */
    }
    return MakeToken(OR); /* | */
  }
  case '"':
    return ScanString();
  }
  /// throw exception if there is an unexcepted character
  throw LexerException(line_, col_, "unexpected character.");
}

} // namespace toyc
