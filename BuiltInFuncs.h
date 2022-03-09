#pragma once

#include "HashTable.h"
#include "var.h"
#include "WordReader.h"
#include <array>

class Parser;
typedef Var(*BuildInFunctionEntrance)(HashTable<Var>&, Parser&);
extern std::array<BuildInFunctionEntrance, BUILTINFUNC_COUNT> builtInFunctions;