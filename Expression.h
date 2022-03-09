#pragma once

#include "WordReader.h"
#include "var.h"
#include "HashTable.h"
#include "TableTypes.h"
#include <list>

class Parser;
class Expression
{
protected:
    HashTable<Var> &localVarTable;
	std::list<Var> tempVars;	//Temporary results from function call, anonymous array, etc. 
								//The result from a function call or building of an anonymous array should be stored in a safe place,
								//or the next step of an index operatoin ('[]') or dot operation ('.') will cause disastor.
								//vars in tempVars will be destroyed automatically when Expression object is destroyed.

	Parser& parser;

    HashTable<Var> PrepareParamTable(bool firstParamValueInGlobal, Var* firstParamValue = nullptr, FunctionDefinition* pfdef = nullptr);

	Var EA(Var&&);
	Var RA(Var&&);
	Var RB(Var&&);
	Var RC(Var&&);
	Var I();
	Var RE(Var&&);
	Var RF(Var&&);
	Var RG(Var&&);
	Var RH(Var&&);
	Var RI(Var&&);
	Var P();
	Var RJ(Var&&);
	Var RK(Var&&);
	Var R();
	Var GetValue(bool asLeft = false);
	Var GetMemberValue(Var&&, bool asLeft = false);
	Var GetIndexedElement(Var&&, bool asLeft = false);

	int FindMemberFunction(long &sdTableIdx);

	Var* EAL(Var*);

	bool lastValueGetInGlobalTable;
	bool shortCircuit { false };	//For "and" / "or" operations. When set true, errors will be omitted.
public:
	Expression(Parser& _parser, HashTable<Var> &_localVarTable);
	virtual ~Expression();

	//!!!! VERY IMPORTANT PART 1 !!!!!
	// The returned Var may be a referenced Var, which is, the varType is VarType::Reference.
	// When the calculation returns a variable in whatever local or global vartable, this return value will be a reference.
	// Be careful when this Var is not intend to be referenced. You may get the original Var by *returnedVar.data.referedTo
	//!!!! VERY IMPORTANT PART 2 !!!!!
	// Calculate returns with parser.wordReader read in one more extra word, the word does not belong to this expression.
	// That is because expression should read in one more extra word to determine the end of an expression.
	// Note this and do not call parser.[wordReader].ReadInOneWord() to get a new word after calling Calcuate().
	Var Calculate();
	
	Var* GetLeftValue();
};