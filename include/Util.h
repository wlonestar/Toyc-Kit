#ifndef UTIL_H
#define UTIL_H

#pragma once

#include <fmt/format.h>
#include <iostream>

namespace toyc {

#define fmt_str(__fmt__, ...) fmt::format(__fmt__, ##__VA_ARGS__)

#ifdef DEBUG
#define debug(__fmt__, ...)                                                    \
  std::clog << fmt_str("\033[1;34m{}:{} [debug] " __fmt__ "\033[0m\n",         \
                       __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(__fmt__, ...) ((void)0)
#endif

} // namespace toyc

#endif
