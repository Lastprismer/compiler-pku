#pragma once

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include "ast.h"

using namespace std;

/* core.cpp */
string sysy2ir(const char* input,
               const char* output,
               bool output2file,
               bool output2stdout);