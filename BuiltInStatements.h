#pragma once

#include "HashTable.h"
#include "var.h"
#include "WordReader.h"
#include <array>

class Parser;
typedef void(*BuildInStatementEntrance)(HashTable<Var>&, Parser&);
extern std::array<BuildInStatementEntrance, BUILTINSTAMENET_COUNT> builtInStatements;

bool Helper_FindCatchSection(HashTable<Var>& localVarTable, Parser& parser, Var& toThrow);