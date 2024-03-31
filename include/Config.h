//! project configuration

#ifndef CONFIG_H
#define CONFIG_H

#pragma once

#include <string>

namespace toyc {

#define HISTORY_FILE ".toyc-repl"

static std::string prompt = "toyc> ";
static std::string multi_prompt = "..... ";
static std::string ext = ".toyc";
static std::string script_ext = ".toycs";

} // namespace toyc

#endif
