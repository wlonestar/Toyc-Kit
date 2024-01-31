#ifndef UTIL_H
#define UTIL_H

#pragma once

#include <fmt/format.h>

namespace toyc {

#define string_format(__fmt__, ...) fmt::format(__fmt__, ##__VA_ARGS__)

} // namespace toyc

#endif
