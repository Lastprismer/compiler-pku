#pragma once

#include <cstring>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include "ir_ast.h"

using namespace std;

namespace ir {
/* core.cpp */
string sysy2ir(const char* input, const char* output, bool output2file);
}  // namespace ir