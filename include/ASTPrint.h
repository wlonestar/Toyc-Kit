//! AST Print

#ifndef AST_PRINT_H
#define AST_PRINT_H

#pragma once

#include <AST.h>
#include <Util.h>

#include <string>

namespace toyc {

#define _RST "\033[0m"

#define _BLACK "\033[0;30m"
#define _RED "\033[0;31m"
#define _GREEN "\033[0;32m"
#define _YELLOW "\033[0;33m"
#define _BLUE "\033[0;34m"
#define _MAGENTA "\033[0;35m"
#define _CYAN "\033[0;36m"
#define _WHITE "\033[0;37m"

#define _BOLD_BLACK "\033[1;30m"
#define _BOLD_RED "\033[1;31m"
#define _BOLD_GREEN "\033[1;32m"
#define _BOLD_YELLOW "\033[1;33m"
#define _BOLD_BLUE "\033[1;34m"
#define _BOLD_MAGENTA "\033[1;35m"
#define _BOLD_CYAN "\033[1;36m"
#define _BOLD_WHITE "\033[1;37m"

#define AST_LEADER(__fmt__, ...) fmt::format(_BLUE __fmt__ _RST, ##__VA_ARGS__)
#define AST_STMT(__fmt__, ...)                                                 \
  fmt::format(_BOLD_MAGENTA __fmt__ _RST, ##__VA_ARGS__)
#define AST_TYPE(__fmt__, ...) fmt::format(_GREEN __fmt__ _RST, ##__VA_ARGS__)
#define AST_LITERAL(__fmt__, ...)                                              \
  fmt::format(_BOLD_CYAN __fmt__ _RST, ##__VA_ARGS__)
#define AST_DECL(__fmt__, ...)                                                 \
  fmt::format(_BOLD_GREEN __fmt__ _RST, ##__VA_ARGS__)

void printASTLeader(size_t _d, Side _s, std::string _p);

} // namespace toyc

#endif
