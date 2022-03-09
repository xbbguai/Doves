#pragma once

class Parser;
void InitializeBuiltinStructs(Parser& parser, bool chn = false);

//Helpers
void BuildStructMember(StructureDefinition& sd, const wchar_t* name, VarType varType, int arraySize = 0, int structIdx = -1);
void BuildFuncParam(FunctionDefinition& fd, const wchar_t* name, VarType paramType, int structParamIndex = -1);
