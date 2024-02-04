//! toyc Token

#ifndef TOKEN_H
#define TOKEN_H

#pragma once

#include <Util.h>

#include <map>
#include <string>

namespace toyc {

enum TokenType {
  // keywords
  AUTO,          // auto
  BREAK,         // break
  CASE,          // case
  CHAR,          // char
  CONST,         // const
  CONTINUE,      // continue
  DEFAULT,       // default
  DO,            // do
  DOUBLE,        // double
  ELSE,          // else
  ENUM,          // enum
  EXTERN,        // extern
  FLOAT,         // float
  FOR,           // for
  GOTO,          // goto
  IF,            // if
  INLINE,        // inline
  INT,           // int
  LONG,          // long
  REGISTER,      // register
  RESTRICT,      // restrict
  RETURN,        // return
  SHORT,         // short
  SIGNED,        // signed
  SIZEOF,        // sizeof
  STATIC,        // static
  STRUCT,        // struct
  SWITCH,        // switch
  TYPEDEF,       // typedef
  UNION,         // union
  UNSIGNED,      // unsigned
  VOID,          // void
  VOLATILE,      // volatile
  WHILE,         // while
  ALIGNAS,       // _Alignas
  ALIGNOF,       // _Alignof
  ATOMIC,        // _Atomic
  BOOL,          // _Bool
  COMPLEX,       // _Complex
  GENERIC,       // _Generic
  IMAGINARY,     // _Imaginary
  NORETURN,      // _Noreturn
  STATIC_ASSERT, // _Static_assert
  THREAD_LOCAL,  // _Thread_local
  FUNC_NAME,     // __func__

  IDENTIFIER,           // identifier
  TYPEDEF_NAME,         // typedef_name
  ENUMERATION_CONSTANT, // enumeration_constant
  INTEGER,              // integer
  FLOATING,             // floating
  STRING,               // string

  // sign
  ELLIPSIS,     // ...
  RIGHT_ASSIGN, // >>=
  LEFT_ASSIGN,  // <<=
  ADD_ASSIGN,   // +=
  SUB_ASSIGN,   // -=
  MUL_ASSIGN,   // *=
  DIV_ASSIGN,   // /=
  MOD_ASSIGN,   // %=
  AND_ASSIGN,   // &=
  XOR_ASSIGN,   // ^=
  OR_ASSIGN,    // |=
  RIGHT_OP,     // >>
  LEFT_OP,      // <<
  INC_OP,       // ++
  DEC_OP,       // --
  PTR_OP,       // ->
  AND_OP,       // &&
  OR_OP,        // ||
  LE_OP,        // <=
  GE_OP,        // >=
  EQ_OP,        // ==
  NE_OP,        // !=
  SEMI,         // ;
  LC,           // { | <%
  RC,           // } | %>
  COMMA,        // ,
  COLON,        // :
  EQUAL,        // =
  LP,           // (
  RP,           // )
  LB,           // [ | <:
  RB,           // ] | :>
  DOT,          // .
  AND,          // &
  NOT,          // !
  SIM,          // ~
  SUB,          // -
  ADD,          // +
  MUL,          // *
  DIV,          // /
  MOD,          // %
  LA,           // <
  RA,           // >
  XOR,          // ^
  OR,           // |
  QUE,          // ?

  EMPTY,
  ERROR, // error
  _EOF,  // end of file
};

#define _stringify_(name) #name
#define _map_(name)                                                            \
  { name, _stringify_(name) }

static std::map<TokenType, std::string> TokenTypeTable = {
    // keywords
    _map_(AUTO),          // auto
    _map_(BREAK),         // break
    _map_(CASE),          // case
    _map_(CHAR),          // char
    _map_(CONST),         // const
    _map_(CONTINUE),      // continue
    _map_(DEFAULT),       // default
    _map_(DO),            // do
    _map_(DOUBLE),        // double
    _map_(ELSE),          // else
    _map_(ENUM),          // enum
    _map_(EXTERN),        // extern
    _map_(FLOAT),         // float
    _map_(FOR),           // for
    _map_(GOTO),          // goto
    _map_(IF),            // if
    _map_(INLINE),        // inline
    _map_(INT),           // int
    _map_(LONG),          // long
    _map_(REGISTER),      // register
    _map_(RESTRICT),      // restrict
    _map_(RETURN),        // return
    _map_(SHORT),         // short
    _map_(SIGNED),        // signed
    _map_(SIZEOF),        // sizeof
    _map_(STATIC),        // static
    _map_(STRUCT),        // struct
    _map_(SWITCH),        // switch
    _map_(TYPEDEF),       // typedef
    _map_(UNION),         // union
    _map_(UNSIGNED),      // unsigned
    _map_(VOID),          // void
    _map_(VOLATILE),      // volatile
    _map_(WHILE),         // while
    _map_(ALIGNAS),       // _Alignas
    _map_(ALIGNOF),       // _Alignof
    _map_(ATOMIC),        // _Atomic
    _map_(BOOL),          // _Bool
    _map_(COMPLEX),       // _Complex
    _map_(GENERIC),       // _Generic
    _map_(IMAGINARY),     // _Imaginary
    _map_(NORETURN),      // _Noreturn
    _map_(STATIC_ASSERT), // _Static_assert
    _map_(THREAD_LOCAL),  // _Thread_local
    _map_(FUNC_NAME),     // __func__

    _map_(IDENTIFIER),           // identifier
    _map_(TYPEDEF_NAME),         // typedef_name
    _map_(ENUMERATION_CONSTANT), // enumeration_constant
    _map_(INTEGER),              // integer
    _map_(FLOATING),             // floating
    _map_(STRING),               // string

    // sign
    _map_(ELLIPSIS),     // ...
    _map_(RIGHT_ASSIGN), // >>=
    _map_(LEFT_ASSIGN),  // <<=
    _map_(ADD_ASSIGN),   // +=
    _map_(SUB_ASSIGN),   // -=
    _map_(MUL_ASSIGN),   // *=
    _map_(DIV_ASSIGN),   // /=
    _map_(MOD_ASSIGN),   // %=
    _map_(AND_ASSIGN),   // &=
    _map_(XOR_ASSIGN),   // ^=
    _map_(OR_ASSIGN),    // |=
    _map_(RIGHT_OP),     // >>
    _map_(LEFT_OP),      // <<
    _map_(INC_OP),       // ++
    _map_(DEC_OP),       // --
    _map_(PTR_OP),       // ->
    _map_(AND_OP),       // &&
    _map_(OR_OP),        // ||
    _map_(LE_OP),        // <=
    _map_(GE_OP),        // >=
    _map_(EQ_OP),        // ==
    _map_(NE_OP),        // !=
    _map_(SEMI),         // ;
    _map_(LC),           // { | <%
    _map_(RC),           // } | %>
    _map_(COMMA),        // ,
    _map_(COLON),        // :
    _map_(EQUAL),        // =
    _map_(LP),           // (
    _map_(RP),           // )
    _map_(LB),           // [ | <:
    _map_(RB),           // ] | :>
    _map_(DOT),          // .
    _map_(AND),          // &
    _map_(NOT),          // !
    _map_(SIM),          // ~
    _map_(SUB),          // -
    _map_(ADD),          // +
    _map_(MUL),          // *
    _map_(DIV),          // /
    _map_(MOD),          // %
    _map_(LA),           // <
    _map_(RA),           // >
    _map_(XOR),          // ^
    _map_(OR),           // |
    _map_(QUE),          // ?

    _map_(ERROR), // error
    _map_(_EOF),  // end of file
};

static std::map<std::string, TokenType> KeywordTable = {
    {"auto", AUTO},                    // auto
    {"break", BREAK},                  // break
    {"case", CASE},                    // case
    {"char", CHAR},                    // char
    {"const", CONST},                  // const
    {"continue", CONTINUE},            // continue
    {"default", DEFAULT},              // default
    {"do", DO},                        // do
    {"double", DOUBLE},                // double
    {"else", ELSE},                    // else
    {"enum", ENUM},                    // enum
    {"extern", EXTERN},                // extern
    {"float", FLOAT},                  // float
    {"for", FOR},                      // for
    {"goto", GOTO},                    // goto
    {"if", IF},                        // if
    {"inline", INLINE},                // inline
    {"int", INT},                      // int
    {"long", LONG},                    // long
    {"register", REGISTER},            // register
    {"restrict", RESTRICT},            // restrict
    {"return", RETURN},                // return
    {"short", SHORT},                  // short
    {"signed", SIGNED},                // signed
    {"sizeof", SIZEOF},                // sizeof
    {"static", STATIC},                // static
    {"struct", STRUCT},                // struct
    {"switch", SWITCH},                // switch
    {"typedef", TYPEDEF},              // typedef
    {"union", UNION},                  // union
    {"unsigned", UNSIGNED},            // unsigned
    {"void", VOID},                    // void
    {"volatile", VOLATILE},            // volatile
    {"while", WHILE},                  // while
    {"_Alignas", ALIGNAS},             // _Alignas
    {"_Alignod", ALIGNOF},             // _Alignof
    {"_Atomic", ATOMIC},               // _Atomic
    {"_Bool", BOOL},                   // _Bool
    {"_Complex", COMPLEX},             // _Complex
    {"_Generic", GENERIC},             // _Generic
    {"_Imaginary", IMAGINARY},         // _Imaginary
    {"_Noreturn", NORETURN},           // _Noreturn
    {"_Static_assert", STATIC_ASSERT}, // _Static_assert
    {"_Thread_local", THREAD_LOCAL},   // _Thread_local
    {"__func__", FUNC_NAME},           // __func__
};

class Token {
public:
  TokenType type;
  std::string value;
  size_t line, col;

  Token() : type(EMPTY), value(""), line(0), col(0) {}
  Token(TokenType _type, std::string &&_value, size_t _line = 1,
        size_t _col = 1)
      : type(_type), value(std::move(_value)), line(_line), col(_col) {}

  std::string toString() const {
    return fstr("Token({}:{} {} -> '{}')", line, col, TokenTypeTable[type],
                value);
  }
};

} // namespace toyc

#endif
