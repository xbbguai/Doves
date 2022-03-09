#include "Expression.h"
#include "BuiltInFuncs.h"
#include "parser.h"
#include <math.h>

Expression::Expression(Parser& _parser, HashTable<Var> &_localVarTable)
			:parser(_parser), localVarTable(_localVarTable)
{
}

Expression::~Expression()
{
}

Var Expression::Calculate()
{   
	return EA(RA(RB(RC(I()))));
}

Var Expression::EA(Var&& EA_in)
{
	Var result;
	if (parser.IsKeywordOf(OP_DOT))
	{
		parser.ReadInOneWord();
		Var r(GetMemberValue(std::move(EA_in)));
		return EA(std::move(r));
	}
	else if (parser.IsKeywordOf(OP_LEFTSQBRACKET))
	{
		Var r(GetIndexedElement(std::move(EA_in)));
		result.ReferTo(&r, parser);
		return EA(std::move(result));
	}
	else
		result.MoveFrom(std::move(EA_in));
	return result;
}

Var Expression::RA(Var&& RA_in)
{
    if (parser.IsKeywordOf(OP_XOR))
	{
      	parser.ReadInOneWord();
        long&& left = RA_in.GetInteger();       //left and right are rvalues to avoid copying of function result.
        long&& right = RB(RC(I())).GetInteger();

		return RA(Var(left ^ right));
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RA_in));
		return result;
	}
}

Var Expression::RB(Var&& RB_in)
{
	if (parser.IsKeywordOf(OP_OR))
	{
      	parser.ReadInOneWord();
		if (RB_in.GetVarType() == VarType::Boolean && RB_in.GetBool() == true)
			shortCircuit = true;
        Var Right = RC(I());
		shortCircuit = false;
        if (RB_in.GetVarType() == VarType::Boolean || Right.GetVarType() == VarType::Boolean)
        {
            bool&& left = RB_in.GetBool();
            bool&& right = Right.GetBool();

            return RB(Var(left || right));
        }
        else
        {
            long&& left = RB_in.GetInteger();
            long&& right = Right.GetInteger();

            return RB(Var(left | right));
        }
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RB_in));
		return result;
	}
}

Var Expression::RC(Var&& RC_in)
{
	if (parser.IsKeywordOf(OP_AND))
	{
      	parser.ReadInOneWord();
		if (RC_in.GetVarType() == VarType::Boolean && RC_in.GetBool() == false)
			shortCircuit = true;
        Var Right = I();
		shortCircuit = false;
        if (RC_in.GetVarType() == VarType::Boolean || Right.GetVarType() == VarType::Boolean)
        {
            bool&& left = RC_in.GetBool();
            bool&& right = Right.GetBool();

            return RC(Var(left && right));
        }
        else
        {
            long&& left = RC_in.GetInteger();
            long&& right = Right.GetInteger();

            return RC(Var(left & right));
        }
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RC_in));
		return result;
	}
}

Var Expression::I()
{
	if (parser.IsKeywordOf(OP_NOT))
	{
      	parser.ReadInOneWord();
        Var Right = I();
		long&& right = Right.GetInteger();
        if (Right.GetVarType() == VarType::Boolean)
    		return Var(right == 0);
        else
            return Var(~right);
	}
	else
		return RE(RF(RG(RH(RI(P())))));
}

Var Expression::RE(Var&& RE_in)
{
	if (parser.IsKeywordOf(OP_EQUAL)) 
	{
      	parser.ReadInOneWord();
		Var KResult = RF(RG(RH(RI(P()))));
		bool bResult;
		if (RE_in.GetVarType() == VarType::String && KResult.GetVarType() == VarType::String)
		{
			const wchar_t* left = RE_in.GetString();
			const wchar_t* right = KResult.GetString();
			bResult = (std::wcscmp(left, right) == 0);
		}
		else if (RE_in.GetVarType() == VarType::String || KResult.GetVarType() == VarType::String)
			throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
		else
		{
			double&& left = RE_in.GetReal();
			double&& right = KResult.GetReal();
			bResult = (left == right);
		}

		return RE(Var(bResult));
	}
	else if (parser.IsKeywordOf(OP_IS))
	{
		bool opNot = false;
		parser.ReadInOneWord();
		if (parser.IsKeywordOf(OP_NOT))
		{
			parser.ReadInOneWord();
			opNot = true;
		}
		bool bResult;
		if (parser.wordType == WordReader::WordType::Identifier)
		{
			//May be structure
			int* idx = parser.structureNameIndex.Find(parser.strWord, parser.currentModule, parser.lenString);
			if (idx == nullptr)
				throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
			if (RE_in.GetVarType() == VarType::Structure)
				bResult = opNot ? (RE_in.GetStructureMember(-1).GetInteger() != *idx) : (RE_in.GetStructureMember(-1).GetInteger() == *idx);
			else
				bResult = opNot ? true : false;
		}
		else
		{
			//Must be built-in type
			VarType vt = VarType::ConvertWordReaderDataType(parser.keywordIdx);
			if (vt == VarType::NotAType)
                throw Var(Exception::ERROR_CODE::UNDEFINED_TYPE);	//This will throw error if anything read in is not a data type
			bResult = opNot ? (RE_in.GetVarType() != vt) : (RE_in.GetVarType() == vt);
		}
		parser.ReadInOneWord();
		return RE(Var(bResult));
	}
	else if (parser.IsKeywordOf(OP_NOTEQUAL))
	{
      	parser.ReadInOneWord();
		Var KResult = RF(RG(RH(RI(P()))));
		bool bResult;
		if (RE_in.GetVarType() == VarType::String && KResult.GetVarType() == VarType::String)
		{
			const wchar_t* left = RE_in.GetString();
			const wchar_t* right = KResult.GetString();
			bResult = (std::wcscmp(left, right) != 0);
		}
		else if (RE_in.GetVarType() == VarType::String || KResult.GetVarType() == VarType::String)
			throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
		else
		{
			double&& left = RE_in.GetReal();
			double&& right = KResult.GetReal();
			bResult = (left != right);
		}

		return RE(Var(bResult));
	}
	else if (parser.IsKeywordOf(OP_GREATER))
	{
      	parser.ReadInOneWord();
		Var KResult = RF(RG(RH(RI(P()))));
		bool bResult;
		if (RE_in.GetVarType() == VarType::String && KResult.GetVarType() == VarType::String)
		{
			const wchar_t* left = RE_in.GetString();
			const wchar_t* right = KResult.GetString();
			bResult = (std::wcscmp(left, right) > 0);
		}
		else if (RE_in.GetVarType() == VarType::String || KResult.GetVarType() == VarType::String)
			throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
		else
		{
			double&& left = RE_in.GetReal();
			double&& right = KResult.GetReal();
			bResult = (left > right);
		}

		return RE(Var(bResult));
	}
	else if (parser.IsKeywordOf(OP_LESS))
	{
      	parser.ReadInOneWord();
		Var KResult = RF(RG(RH(RI(P()))));
		bool bResult;
		if (RE_in.GetVarType() == VarType::String && KResult.GetVarType() == VarType::String)
		{
			const wchar_t* left = RE_in.GetString();
			const wchar_t* right = KResult.GetString();
			bResult = (std::wcscmp(left, right) < 0);
		}
		else if (RE_in.GetVarType() == VarType::String || KResult.GetVarType() == VarType::String)
			throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
		else
		{
			double&& left = RE_in.GetReal();
			double&& right = KResult.GetReal();
			bResult = (left < right);
		}

		return RE(Var(bResult));
	}
	else if (parser.IsKeywordOf(OP_GREATEREQUAL))
	{
      	parser.ReadInOneWord();
		Var KResult = RF(RG(RH(RI(P()))));
		bool bResult;
		if (RE_in.GetVarType() == VarType::String && KResult.GetVarType() == VarType::String)
		{
			const wchar_t* left = RE_in.GetString();
			const wchar_t* right = KResult.GetString();
			bResult = (std::wcscmp(left, right) >= 0);
		}
		else if (RE_in.GetVarType() == VarType::String || KResult.GetVarType() == VarType::String)
			throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
		else
		{
			double&& left = RE_in.GetReal();
			double&& right = KResult.GetReal();
			bResult = (left >= right);
		}

		return RE(Var(bResult));
	}
	else if (parser.IsKeywordOf(OP_LESSEQUAL))
	{
      	parser.ReadInOneWord();
		Var KResult = RF(RG(RH(RI(P()))));
		bool bResult;
		if (RE_in.GetVarType() == VarType::String && KResult.GetVarType() == VarType::String)
		{
			const wchar_t* left = RE_in.GetString();
			const wchar_t* right = KResult.GetString();
			bResult = (std::wcscmp(left, right) <= 0);
		}
		else if (RE_in.GetVarType() == VarType::String || KResult.GetVarType() == VarType::String)
			throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
		else
		{
			double&& left = RE_in.GetReal();
			double&& right = KResult.GetReal();
			bResult = (left <= right);
		}

		return RE(Var(bResult));
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RE_in));
		return result;
	}
}

Var Expression::RF(Var&& RF_in)
{
	if (parser.IsKeywordOf(OP_PLUS))
	{
       	parser.ReadInOneWord();
	
    	Var Right = RG(RH(RI(P())));		
		if (RF_in.GetVarType() == VarType::String && Right.GetVarType() == VarType::String)
		{
			wchar_t* left = RF_in.GetString();
			wchar_t* right = Right.GetString();
            size_t sizeLeft = std::wcslen(left);
			size_t sizeRight = std::wcslen(right);
            wchar_t* cat = new wchar_t[sizeLeft + sizeRight + 1];
            std::wcscpy(cat, left);
            std::wcscpy(cat + sizeLeft, right);
			Var result;
            result.EmplaceString(cat);
	 		return RF(std::move(result));
		}
		else
		{
			double &&left = RF_in.GetReal();
			double &&right = Right.GetReal();
			return RF(Var(left + right));
		}
	}
	else if (parser.IsKeywordOf(OP_MINUS))
	{
       	parser.ReadInOneWord();

		double &&left = RF_in.GetReal();
		double &&right = RG(RH(RI(P()))).GetReal();

		return RF(Var(left - right));
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RF_in));
		return result;
	}
}

Var Expression::RG(Var&& RG_in)
{
	if (parser.IsKeywordOf(OP_MOD))
	{
    	parser.ReadInOneWord();
		long&& left = RG_in.GetInteger();
		long&& right = RH(RI(P())).GetInteger();

		return RG(Var(left % right));
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RG_in));
		return result;
	}
}

Var Expression::RH(Var&& RH_in)
{
	if (parser.IsKeywordOf(OP_INT_DIVIDE))
	{
        parser.ReadInOneWord();
		long&& left = RH_in.GetInteger();
		long&& right = RI(P()).GetInteger();
		if (right == 0)
			throw Var(Exception::ERROR_CODE::DIVIDE_BY_ZEOR);
    	
		return RH(Var(left / right));
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RH_in));
		return result;
	}
}

Var Expression::RI(Var&& RI_in)
{
	if (parser.IsKeywordOf(OP_MULTIPLY))
	{
    	parser.ReadInOneWord();
		double&& left = RI_in.GetReal();
		double&& right = P().GetReal();

		return RI(Var(left * right));
	}
	else if (parser.IsKeywordOf(OP_DIVIDE))
	{
    	parser.ReadInOneWord();
		double&& left = RI_in.GetReal();
		double&& right = P().GetReal();

		return RI(Var(left / right));
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RI_in));
		return result;
	}
}

Var Expression::P()
{
	if (parser.IsKeywordOf(OP_PLUS))
	{
    	parser.ReadInOneWord();
		return RK(R());
//		return P();
	}
	else if (parser.IsKeywordOf(OP_MINUS))
	{
    	parser.ReadInOneWord();
		Var result;
//        result.SetReal(-P().GetReal());
		result.SetReal(-RK(R()).GetReal());
		return result;
	}
	else
		return RK(R());
}

Var Expression::RK(Var&& RK_in)
{
	if (parser.IsKeywordOf(OP_POWER))
	{
    	parser.ReadInOneWord();
		double&& left = RK_in.GetReal();
		double&& right = R().GetReal();

		return RK(Var(pow(left, right)));
	}
	else
	{
		Var result;
		result.MoveFrom(std::move(RK_in));
		return result;
	}
}

Var Expression::R()
{
	Var result;
	if (parser.IsKeywordOf(OP_LEFTBRACKET))
	{
		parser.ReadInOneWord();
		result.MoveFrom(EA(RA(RB(RC(I())))));
		if (parser.wordType == WordReader::WordType::Keyword && parser.keywordIdx != OP_RIGHTBRACKET) 
			throw Var(Exception::ERROR_CODE::RIGHT_BRACE_EXPECTED);
    	parser.ReadInOneWord();
	}
	else
		result.MoveFrom(EA(GetValue(false)));

	//Check result reference. If reference is not set, throw error.
	if (result.IsReference() && !result.HasReferedTo())
		result.Clear();
	return result;
}

Var* Expression::EAL(Var* EAL_in)
{
	Var* var;
	if (parser.IsKeywordOf(OP_DOT))
	{
		parser.ReadInOneWord();
		var = (Var *)(GetMemberValue(std::move(*EAL_in), true).GetPointer());
		if (var == nullptr)
			return var;
		else
			return EAL(var);
	}
	else if (parser.IsKeywordOf(OP_LEFTSQBRACKET))
	{
		var = (Var *)(GetIndexedElement(std::move(*EAL_in), true).GetPointer());
		if (var == nullptr)
			return var;
		else
			return EAL(var);
	}
	else
	{
		return EAL_in;	
	}
}

Var* Expression::GetLeftValue()
{
	Var* var;
	if (parser.IsKeywordOf(OP_LEFTBRACKET))
	{
		parser.ReadInOneWord();
		var = EAL(GetLeftValue());
		if (parser.wordType == WordReader::WordType::Keyword && parser.keywordIdx != OP_RIGHTBRACKET) 
			throw Var(Exception::ERROR_CODE::RIGHT_BRACE_EXPECTED);
    	parser.ReadInOneWord();
	}
	else
	{
		var = EAL((Var *)(GetValue(true).GetPointer()));
	}

	return var;	
}

int Expression::FindMemberFunction(long &sdTableIdx)
{
	wchar_t* temp = new wchar_t[parser.lenString + 20];
	std::wcsncpy(temp, parser.strWord, parser.lenString);
	int* nameIdx;
	while (true)
	{
		//If it is a member function of a structure, the function name become FunctionName#{Index}
		temp[parser.lenString] = L'#';
		int count = parser.IntToWCS(sdTableIdx, temp + parser.lenString + 1);
		nameIdx = parser.functionNameIndex.Find(temp, parser.structureTable[sdTableIdx].moduleIdx, parser.lenString + count + 1);
		if (nameIdx == nullptr && parser.structureTable[sdTableIdx].parentStructIndex != -1)
		{
			//There is a parent. I should try parent.
			sdTableIdx = parser.structureTable[sdTableIdx].parentStructIndex;
		}
		else
			break;
	}
	delete[] temp;
	if (nameIdx == nullptr)
		return -1;
	else
		return *nameIdx;
}

//Visit a member of a structure (Dot operator), or call a built-in function with the first param being the value at left side of the 'Dot'.
//Params:
//  asLeft: If set true, will return a C pointer to the variable when the variable is a left value, and nullptr when
//			the result comes from an expression calculation or function call (which is right value).
//			The pointer is stored in data.pointer.
//			Meanwhile, when asLeft is true, GetMemberValue will create a new Null var if the variable is not found in any table.
//Return:
//  When asLeft is false:
//    A reference var if it is a variable.
//    A value var if the result is from a function call or an expression evaluation.
//  When asLeft is true:
//    A pointer to the var which stored in data.pointer if it is a variable.
//    nullptr if the result is from a function call or an expression evaluation.
Var Expression::GetMemberValue(Var&& S_in, bool asLeft /* = false*/)
{
	Var result;	
	if (parser.wordType == WordReader::WordType::Keyword && (parser.keywordIdx & KT_BUILTINFUNC) == KT_BUILTINFUNC)
	{
		//Calling a built-in function with the grammar of value.function() and value be the first param of the function.
		int funcIdx = parser.keywordIdx - KT_BUILTINFUNC;
		HashTable<Var>&& paramTable = PrepareParamTable(lastValueGetInGlobalTable, &S_in);
		Var&& r = (builtInFunctions[funcIdx])(paramTable, parser);
		if (!asLeft)
			result.MoveFrom(std::move(r));
	}
	else if (S_in.GetVarType().IsStructure())	//Structures have dot operator
	{
		if (S_in.IsReference() && !S_in.HasReferedTo())
			throw Var(Exception::ERROR_CODE::USING_NOT_INITIALIZED_REF);
		long sdTableIdx = S_in.GetStructureMember(-1).GetInteger();	//Use -1 to get the structure table index.
		//Now, find the member variable name from structure definitions
		int* nameIdx = parser.structureTable[sdTableIdx].memberNames.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);
		if (nameIdx != nullptr)
		{
			//Yes, member variable.
			if (asLeft)
				result.SetPointer((void *)&S_in.GetStructureMember(*nameIdx));
			else
			{
				Var* pvar = &S_in.GetStructureMember(*nameIdx);
				//When I need an r-value but the result is a not initialized reference, just return a Null var.
				if (!pvar->IsReference() || pvar->HasReferedTo())
					result.ReferTo(pvar, parser);	//No, it's set reference.
				//or, result is a Null var.
			}
		}
		else
		{
			//Should try member function.
			int idx = FindMemberFunction(sdTableIdx);
			if (idx == -1)
				throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
			HashTable<Var> paramTable = PrepareParamTable(lastValueGetInGlobalTable, &S_in, &parser.functionTable[idx]);

			//Call the user function
/*			if (asLeft)
				parser.Execute(idx, paramTable);	//Ignore the result
			else
				result = parser.Execute(idx, paramTable);
*/
//			result = tempVars.emplace_back(parser.Execute(idx, paramTable));
			Var varR = parser.Execute(idx, paramTable);
			Var* var = &tempVars.emplace_back(varR);
			if (asLeft)
				result.SetPointer(var);
			else
				result.ReferTo(var, parser);
//				result = varR;
		}
	}
	else
		throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
	parser.ReadInOneWord();
	return result;
}

//Get a member from an indexable var, eg. an array.
//Params:
//  asLeft: If set true, will return a C pointer to the variable when the variable is a left value.
//			The pointer is stored in data.pointer.
//Return:
//  When asLeft is false:
//    A reference var.
//  When asLeft is true:
//    A pointer to the var which stored in data.pointer.
Var Expression::GetIndexedElement(Var&& T_in, bool asLeft /* = false*/)
{
	Var result;
	if (T_in.GetVarType().IsIndexable())
	{
		//Array or memory, there is a '[' following the array or memory name...
		//And the '[' has been 
		parser.ReadInOneWord();
		Expression expBracket(parser, localVarTable);
		Var indexResult = expBracket.Calculate();
		//index boundary check
		long&& index = indexResult.GetInteger();
		if (index < 0 || index >= T_in.GetDataSize())
			throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
		else if (T_in.GetVarType() == VarType::Array)
		{
			if (asLeft)
				result.SetPointer((void *)&T_in.GetElementAt(index));
			else
			{
//				result.ReferTo(&T_in.GetElementAt(index), parser);
				Var* pvar = &T_in.GetElementAt(index);
				//When I need an r-value but the result is a not initialized reference, just return a Null var.
				if (!pvar->IsReference() || pvar->HasReferedTo())
					result.ReferTo(pvar, parser);	//No, it's set reference.
				//or, result is a Null var.
			}
		}
		else if (T_in.GetVarType() == VarType::Memory)
		{
			long res = T_in.GetMemoryAt(index);
			result.SetInteger(res);					//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!PROBLEM!!!!!!!!!!!!!!!!!!!!!!!
			if (asLeft)
				throw Var(Exception::ERROR_CODE::MUST_BE_REFERENCE);
		}
		//...structure not implemented.

		if (parser.wordType != WordReader::WordType::Keyword || parser.keywordIdx != OP_RIGHTSQBRACKET)
			throw Var(Exception::ERROR_CODE::RIGHT_SQBRACKET_EXPECTED);
		parser.ReadInOneWord();
	}
	else
		throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
	return result;
}

//Params:
//  asLeft: If set true, will return a C pointer to the variable when the variable is a left value, and nullptr when
//			the result comes from an expression calculation or function call (which is right value).
//			The pointer is stored in data.pointer.
//			Meanwhile, when asLeft is true, GetValue will create a new Null var if the variable is not found in any table.
//Return:
//  When asLeft is false:
//    A reference var if it is a variable.
//    A value var if the result is from a function call or an expression evaluation.
//  When asLeft is true:
//    A pointer to the var which stored in data.pointer if it is a variable.
//    nullptr if the result is from a function call or an expression evaluation.
Var Expression::GetValue(bool asLeft /* = false*/)
{
	Var result;
	Var* var { nullptr };
	lastValueGetInGlobalTable = false;
	if (parser.wordType == WordReader::WordType::Double)			//Literal, double
		result.SetReal(parser.fWord);
    else if (parser.wordType == WordReader::WordType::Integer)		//Literal, integer
        result.SetInteger(parser.iWord);
    else if (parser.wordType == WordReader::WordType::Boolean)		//Literal, boolean
        result.SetBool(parser.bWord);
    else if (parser.wordType == WordReader::WordType::Character)	//Literal, char
        result.SetChar(parser.cWord);
	else if (parser.wordType == WordReader::WordType::String)		//Literal, string
        result.SetString(parser.strWord, parser.lenString);
	else if (parser.IsKeywordOf(OP_IS))	//Special 'IS' is used in select-case. From this on, not a literal
	{
		Var* is = localVarTable.Find(L"IS", MODULE_IRRELEVANT, 2);
		if (is == nullptr)
			throw Var(Exception::ERROR_CODE::CASE_WITHOUT_SELECT);
		result.ResetAndReferTo(is);
	}
	else if (parser.IsKeywordOf(DT_NULL))
	{
		//Just return result, which is Null.
		//Do not delete this branch to avoid 'NULL' being considered as syntax error.
	}
	else if (parser.IsKeywordOf(OP_LEFTSQBRACKET))	//An indexable object. e.g. an array
	{
		//Now, build the array.
		int index = 0;
		int arraySize = 10;	//Dynamic size
		if (asLeft)
			var = &localVarTable.InsertAnonymous(result);
		else
			var = &tempVars.emplace_back(result);
		var->InitArray(arraySize);
		while (true)
		{
			parser.ReadInOneWord();
			if (parser.IsEndOfStatement() || parser.IsKeywordOf(OP_RIGHTSQBRACKET))
				break;

			Expression expr(parser, localVarTable);
			if (asLeft)
			{
				Var* varLeft = expr.GetLeftValue();
				if (varLeft == nullptr)
					throw Var(Exception::ERROR_CODE::RVALUE_NOT_REFERENCABLE);
				var->GetElementAt(index++).ReferTo(varLeft, parser);
			}
			else
			{
				var->GetElementAt(index++) = expr.Calculate();
			}
			if (!parser.IsKeywordOf(OP_COMMA))
				break;
			if (index >= arraySize)
			{
				arraySize += 10;
				var->ResizeArray(arraySize, parser.structureTable);
			}
		}
		if (!parser.IsKeywordOf(OP_RIGHTSQBRACKET))
			throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
		if (arraySize != index)	//Should adjust the final size of the array
			var->ResizeArray(index, parser.structureTable);
		if (!asLeft)
		{
			result.ReferTo(var, parser);
		}
	}
	else if (parser.wordType == WordReader::WordType::Keyword)
    {
		//Maybe a built-in function
		if ((parser.keywordIdx & KT_BUILTINFUNC) == KT_BUILTINFUNC)
		{
			int funcIdx = parser.keywordIdx - KT_BUILTINFUNC;
			HashTable<Var>&& paramTable = PrepareParamTable(lastValueGetInGlobalTable);
			result.MoveFrom((builtInFunctions[funcIdx])(paramTable, parser));
		}
		else
			throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
	}
	else //Var or user function
	{
		//Try looking it up in local var table first.
		var = localVarTable.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);	//In the local variable table?
		//Not found? Try 'self' and find in the member variables.
		if (var == nullptr)
		{
			Var* vSelf { nullptr };	//To store the 'self' for further use.
			//Maybe a member of 'self'
			vSelf = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
			if (vSelf != nullptr)
			{
				var = vSelf->FindStructureMember(parser.structureTable, parser.strWord, parser.lenString);
				if (var == nullptr)
				{
					//Not a member variable, try member function.
					long sdTableIdx = vSelf->GetStructureMember(-1).GetInteger();	//Use -1 to get the structure table index.
					int idx = FindMemberFunction(sdTableIdx);
					if (idx != -1)
					{
						//Yes, call the member function.
						HashTable<Var> paramTable = PrepareParamTable(lastValueGetInGlobalTable, vSelf, &parser.functionTable[idx]);
						result = parser.Execute(idx, paramTable);
						goto resultGet;
					}
				}
			}
		}
		//Still not found? Try global var table.
		int moduleIdx = parser.currentModule;
		if (var == nullptr)
		{
			var = parser.globalVarTable.Find(parser.strWord, moduleIdx, parser.lenString);	//In the global variable table?
			if (var != nullptr && var->GetVarType() == VarType::InternalModuleIdx)
			{
				//It's a prefix which indicates that the following function/variable is from another module.
				moduleIdx = var->GetInteger();
				parser.ReadInOneWord();
				//There must be a dot
				if (!parser.IsKeywordOf(OP_DOT))
					throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
				parser.ReadInOneWord();
				//Followed by an identifier
				if (parser.wordType != WordReader::WordType::Identifier)
					throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
				//At this stage, if it is a variable, this variable could only be in global variable table.
				var = parser.globalVarTable.Find(parser.strWord, moduleIdx, parser.lenString);	
			}
			if (var != nullptr)
				lastValueGetInGlobalTable = true;
		}

		if (var == nullptr)	//Not in any variable table.
		{
			//Try user function.
			int* idx = parser.functionNameIndex.Find(parser.strWord, moduleIdx, parser.lenString);
			if (idx == nullptr)
			{
				if (asLeft)
				{
					var = &localVarTable.Insert(parser.strWord, MODULE_IRRELEVANT, parser.lenString, Var());
					result.ResetAndReferTo(var);
				}
				else
				{
					throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
				}
			}
			else
			{
				HashTable<Var> paramTable = PrepareParamTable(lastValueGetInGlobalTable, nullptr, &parser.functionTable[*idx]);

				//Call the user function
//				result = parser.Execute(*idx, paramTable);
				Var varR = parser.Execute(*idx, paramTable);
				var = &tempVars.emplace_back(varR);
				//result = *var;
				result.ResetAndReferTo(var);
			}
		}
		else if (!asLeft)	// var found in any one of the variable table.
		{
				//If it is a variable, return the reference to this variable.
				//It's important that the returned Var is a reference.
				//It is used when there is a BYREF directive in the function param declaration.
//				result.ResetAndReferTo(var);
				//When I need an r-value but the result is a not initialized reference, just return a Null var.
				if (!var->IsReference() || var->HasReferedTo())
					result.ResetAndReferTo(var);	//No, it's set reference.
				else	//or, result is a Null var.
					result.Clear();
		}
	}
resultGet:	
	//Next word
	parser.ReadInOneWord();
	if (asLeft)
	{
		result.Clear();
		result.SetPointer((void*)var);
	}
	return result;
}

HashTable<Var> Expression::PrepareParamTable(bool firstParamValueInGlobal, Var* firstParamValue/* = nullptr */, FunctionDefinition* pfdef /* = nullptr */)
{
    HashTable<Var> paramTable;

    //Make sure the function param table starts with a '('
    parser.ReadInOneWord();
    if (parser.wordType != WordReader::WordType::Keyword || parser.keywordIdx != OP_LEFTBRACKET)
        throw Var(Exception::ERROR_CODE::LEFT_BRACE_EXPECTED);
    
    //Default param name of xx, xx = 00, 01, 02, ..., 99. which is the index of a param.
    int paramIndex = 0;
    wchar_t params[3] {L'0', L'0', 0};

	if (firstParamValue)
	{
		if (pfdef)
		{
			//User function.
			int* idx = pfdef->paramIndexes.Find(L"self", MODULE_IRRELEVANT, 4);
			if (idx == nullptr)
				paramTable.Insert(params, MODULE_IRRELEVANT, *firstParamValue);
			else
			{
				//Verify param data type
				VarType paramType = pfdef->paramTypes[*idx];
				if (!paramType.IsCompatible(firstParamValue->GetVarType()))
					throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
				//For structures, there is the demand for polymorphism. So that it doesn't need to check the structure type.
				Var& byref = paramTable.Insert(L"self", MODULE_IRRELEVANT, Var());
				byref.ReferTo(firstParamValue, parser);
				firstParamValue->IncreaseReferCount();	//Can also call byref.IncreaseReferCount();
			}
		}
		else
		{
			//built-in function. always refer to all params.
			Var& byref = paramTable.Insert(params, MODULE_IRRELEVANT, Var());
			byref.ReferTo(firstParamValue, parser);
//			firstParamValue->IncreaseReferCount();	//Can alse call byref.IncreaseReferCount();
		}
		paramIndex++;
	}

    parser.ReadInOneWord();
    if (!parser.IsKeywordOf(OP_RIGHTBRACKET))
    {
        while (true)
        {
            params[1] = paramIndex % 10 + L'0';
            params[0] = paramIndex / 10 + L'0';

            //Parse a param value
			Expression paramExp(parser, localVarTable);
			Var&& result =  paramExp.Calculate();
			//When the Expression.Calculate returns, wordReader will have read a new word.

            //If there is a ':' after the value expression, it means this value belongs to a specific named param.
            if (parser.IsKeywordOf(OP_COLON))
            {
                //The word is ':'. Get the param name. The param name should be an identifier.
                parser.ReadInOneWord();
                if (parser.wordType == WordReader::WordType::Identifier)
				{
					if (pfdef)
					{
						int* idx = pfdef->paramIndexes.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);
						if (idx == nullptr)
							throw Var(Exception::ERROR_CODE::PARAM_NOT_DEFINED);
						//Verify param data type
						VarType paramType = pfdef->paramTypes[*idx];
						if (!paramType.IsCompatible(result.GetVarType()))
							throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
						//For structures, the structure index should be the same.
						if (paramType.IsStructure() && !paramType.IsReference())
						{
							Var structIdx = result.GetStructureMember(-1);
							if (result.GetVarType().IsRedimable())
							{
								Var temp;
								temp.InitStructure(parser.structureTable, pfdef->structParamIndex[*idx]);
								temp = result;
								result.Clear();
								result = temp;
							}
							else if (structIdx.GetInteger() != pfdef->structParamIndex[*idx])
								throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
						}
						if (paramType.IsReference())
						{
							//This parameter is 'byref'.
							//Should make sure that the 'result' is referable. That is, result should refer to a variable.
							//If 'result' is not a reference, that will mean 'result' is a calculation result and is an r-value,
							//which is not refereable.
							if (result.IsReference() && !result.IsConstant())
							{
								Var& byref = paramTable.Insert(parser.strWord, MODULE_IRRELEVANT, parser.lenString, Var());
								if (pfdef->structParamIndex[*idx] != -1)
								{
									byref.SetVarType(VarType::Structure | VarType::ReferenceFlag, pfdef->structParamIndex[*idx]);
								}
								byref.ReferTo(&result, parser);
								result.IncreaseReferCount();	//Can also call byref.IncreaseReferCount()
							}
							else
								throw Var(Exception::ERROR_CODE::RVALUE_NOT_REFERENCABLE);
						}
						else
							paramTable.Insert(parser.strWord, MODULE_IRRELEVANT, parser.lenString, result);	//This will cause to make a copy of 'result' and insert the copy.
					}
					else
                    	paramTable.Insert(parser.strWord, MODULE_IRRELEVANT, parser.lenString, result);
				}
                else
                    throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
                parser.ReadInOneWord();
            }
            else
			{
				wchar_t* paramName;
				if (pfdef && (paramName = pfdef->paramIndexes.ReverseFind(paramIndex)) != nullptr)
				{
					//Verify param data type
					VarType paramType = pfdef->paramTypes[paramIndex];
					if (!paramType.IsCompatible(result.GetVarType()))
						throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
					//For structures, the structure index should be the same.
					if (paramType.IsStructure() && !paramType.IsReference())
					{
						Var structIdx = result.GetStructureMember(-1);
						if (result.GetVarType().IsRedimable())
						{
							Var temp;
							temp.InitStructure(parser.structureTable, pfdef->structParamIndex[paramIndex]);
							temp = result;
							result.Clear();
							result = temp;
						}
						else if (structIdx.GetInteger() != pfdef->structParamIndex[paramIndex])
							throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
					}
					if (paramType.IsReference())
					{
						//This parameter is 'byref'.
						//Should make sure that the 'result' is referable. That is, result should refer to a variable.
						//If 'result' is not a reference, that will mean 'result' is a calculation result and is an r-value,
						//which is not refereable.
						if (result.IsReference() && !result.IsConstant())
						{
							Var& byref = paramTable.Insert(paramName, MODULE_IRRELEVANT, Var());
							int idx = paramTable.GetSize() - 1;
							if (pfdef->structParamIndex[idx] != -1)
							{
								byref.SetVarType(VarType::Structure | VarType::ReferenceFlag, pfdef->structParamIndex[idx]);
							}
							byref.ReferTo(&result, parser);
							result.IncreaseReferCount();	//Can also call byref.IncreaseReferCount()
						}
						else
							throw Var(Exception::ERROR_CODE::RVALUE_NOT_REFERENCABLE);
					}
					else
						paramTable.Insert(paramName, MODULE_IRRELEVANT, result);	//This will cause to make a copy of 'result' and insert the copy.
				}
				else
				{
					if (result.IsReference() && !result.IsConstant())
					{
						Var& byref = paramTable.Insert(params, MODULE_IRRELEVANT, Var());
						byref.ReferTo(&result, parser);
						result.IncreaseReferCount();	//Can also call byref.IncreaseReferCount()
					}
					else
						paramTable.Insert(params, MODULE_IRRELEVANT, result);  //Or, use the default param name.
				}
			}

            //If next word is not a ',', break.
            if (!parser.IsKeywordOf(OP_COMMA))
                break;
            else
                parser.ReadInOneWord();

            //Increase paramIndex so that the next default param name changes.
            paramIndex++;
            if (paramIndex == 100)
                throw Var(Exception::ERROR_CODE::TOO_MANY_PARAMS);
			
			firstParamValue = nullptr;	//Disable it.
        }
    }
    //At the end, there should be ')'
    if (parser.wordType != WordReader::WordType::Keyword || parser.keywordIdx != OP_RIGHTBRACKET)
        throw Var(Exception::ERROR_CODE::RIGHT_BRACE_EXPECTED);

	//And, the paramTable should contain a pointer to current localVarTable, which belongs to its caller.
	//This caller localVarTable/globalVarTable is used to store thread object pointer.
	//So that when you start a thread inside a function, system makes sure the thread exits when exiting the function,
	//or the whole program crashes.
	Var fvt;	//fvt = father variable table.
	if (firstParamValueInGlobal)
		fvt.SetPointer((void*)&parser.globalVarTable);
	else
		fvt.SetPointer((void*)&localVarTable);
	paramTable.Insert(L"#fvt", MODULE_IRRELEVANT, fvt);

    return paramTable;
}
