//! toyc Token

#ifndef TOKEN_H
#define TOKEN_H

#pragma once

#include <Util.h>

#include <map>
#include <string>
#include <vector>

namespace toyc {

enum TokenTy {
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
  F64,           // f64 - replacement for double precision floating point number
  FLOAT,         // float
  FOR,           // for
  GOTO,          // goto
  I64,           // i64 - replacement for 64 bit integer
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

/**
 * @brief Mapping from enumeration to string using vector
 *
 * @notice: Do not change the order of types
 */
static std::vector<std::string> TokenTyTable = {
    // keywords
    _stringify_(AUTO),          // auto
    _stringify_(BREAK),         // break
    _stringify_(CASE),          // case
    _stringify_(CHAR),          // char
    _stringify_(CONST),         // const
    _stringify_(CONTINUE),      // continue
    _stringify_(DEFAULT),       // default
    _stringify_(DO),            // do
    _stringify_(DOUBLE),        // double
    _stringify_(ELSE),          // else
    _stringify_(ENUM),          // enum
    _stringify_(EXTERN),        // extern
    _stringify_(F64),           // f64
    _stringify_(FLOAT),         // float
    _stringify_(FOR),           // for
    _stringify_(GOTO),          // goto
    _stringify_(I64),           // i64
    _stringify_(IF),            // if
    _stringify_(INLINE),        // inline
    _stringify_(INT),           // int
    _stringify_(LONG),          // long
    _stringify_(REGISTER),      // register
    _stringify_(RESTRICT),      // restrict
    _stringify_(RETURN),        // return
    _stringify_(SHORT),         // short
    _stringify_(SIGNED),        // signed
    _stringify_(SIZEOF),        // sizeof
    _stringify_(STATIC),        // static
    _stringify_(STRUCT),        // struct
    _stringify_(SWITCH),        // switch
    _stringify_(TYPEDEF),       // typedef
    _stringify_(UNION),         // union
    _stringify_(UNSIGNED),      // unsigned
    _stringify_(VOID),          // void
    _stringify_(VOLATILE),      // volatile
    _stringify_(WHILE),         // while
    _stringify_(ALIGNAS),       // _Alignas
    _stringify_(ALIGNOF),       // _Alignof
    _stringify_(ATOMIC),        // _Atomic
    _stringify_(BOOL),          // _Bool
    _stringify_(COMPLEX),       // _Complex
    _stringify_(GENERIC),       // _Generic
    _stringify_(IMAGINARY),     // _Imaginary
    _stringify_(NORETURN),      // _Noreturn
    _stringify_(STATIC_ASSERT), // _Static_assert
    _stringify_(THREAD_LOCAL),  // _Thread_local
    _stringify_(FUNC_NAME),     // __func__

    _stringify_(IDENTIFIER),           // identifier
    _stringify_(TYPEDEF_NAME),         // typedef_name
    _stringify_(ENUMERATION_CONSTANT), // enumeration_constant
    _stringify_(INTEGER),              // integer
    _stringify_(FLOATING),             // floating
    _stringify_(STRING),               // string

    // sign
    _stringify_(ELLIPSIS),     // ...
    _stringify_(RIGHT_ASSIGN), // >>=
    _stringify_(LEFT_ASSIGN),  // <<=
    _stringify_(ADD_ASSIGN),   // +=
    _stringify_(SUB_ASSIGN),   // -=
    _stringify_(MUL_ASSIGN),   // *=
    _stringify_(DIV_ASSIGN),   // /=
    _stringify_(MOD_ASSIGN),   // %=
    _stringify_(AND_ASSIGN),   // &=
    _stringify_(XOR_ASSIGN),   // ^=
    _stringify_(OR_ASSIGN),    // |=
    _stringify_(RIGHT_OP),     // >>
    _stringify_(LEFT_OP),      // <<
    _stringify_(INC_OP),       // ++
    _stringify_(DEC_OP),       // --
    _stringify_(PTR_OP),       // ->
    _stringify_(AND_OP),       // &&
    _stringify_(OR_OP),        // ||
    _stringify_(LE_OP),        // <=
    _stringify_(GE_OP),        // >=
    _stringify_(EQ_OP),        // ==
    _stringify_(NE_OP),        // !=
    _stringify_(SEMI),         // ;
    _stringify_(LC),           // { | <%
    _stringify_(RC),           // } | %>
    _stringify_(COMMA),        // ,
    _stringify_(COLON),        // :
    _stringify_(EQUAL),        // =
    _stringify_(LP),           // (
    _stringify_(RP),           // )
    _stringify_(LB),           // [ | <:
    _stringify_(RB),           // ] | :>
    _stringify_(DOT),          // .
    _stringify_(AND),          // &
    _stringify_(NOT),          // !
    _stringify_(SIM),          // ~
    _stringify_(SUB),          // -
    _stringify_(ADD),          // +
    _stringify_(MUL),          // *
    _stringify_(DIV),          // /
    _stringify_(MOD),          // %
    _stringify_(LA),           // <
    _stringify_(RA),           // >
    _stringify_(XOR),          // ^
    _stringify_(OR),           // |
    _stringify_(QUE),          // ?

    _stringify_(ERROR), // error
    _stringify_(_EOF),  // end of file
};

/**
 * @brief Mapping from string to enumeration using map
 *
 */
static std::map<std::string, TokenTy> KeywordTable = {
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
    {"f64", F64},                      // f64
    {"float", FLOAT},                  // float
    {"for", FOR},                      // for
    {"goto", GOTO},                    // goto
    {"i64", I64},                      // i64
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
  TokenTy type;
  std::string value;
  size_t line, col;

  Token() : type(EMPTY), value(""), line(1), col(0) {}
  Token(TokenTy _type, std::string _value, size_t _line = 1, size_t _col = 0)
      : type(_type), value(_value), line(_line), col(_col) {}

  std::string toString() const {
    return fstr("token({}:{} {} -> '{}')", line, col, TokenTyTable[type],
                value);
  }
};

} // namespace toyc

#endif
