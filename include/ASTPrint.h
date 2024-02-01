//! AST Print

#ifndef AST_PRINT_H
#define AST_PRINT_H

#pragma once

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

#define AST_LEADER(str)                                                        \
  fmt_str("{}{}{}", std::string(_BLUE), str, std::string(_RST))
#define AST_SYNTAX(str)                                                        \
  fmt_str("{}{}{}", std::string(_BOLD_MAGENTA), str, std::string(_RST))
#define AST_TYPE(str)                                                          \
  fmt_str("{}{}{}", std::string(_GREEN), str, std::string(_RST))
#define AST_LITERAL(str)                                                       \
  fmt_str("{}{}{}", std::string(_BOLD_CYAN), str, std::string(_RST))

} // namespace toyc

#endif
