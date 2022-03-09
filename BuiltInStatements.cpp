#include "var.h"
#include "Exception.h"
#include "HashTable.h"
#include "WordReader.h"
#include "BuiltInStatements.h"
#include "Expression.h"
#include "parser.h"
#include "textencoding.h"
#include <array>
#include <math.h>
#include <iostream>

//public functions 
static inline void InsertIntoInternalQueue(HashTable<Var>& localVarTable, Var::QueueInfo& qi)
{
    Var* internalQueue = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (internalQueue == nullptr)
    {
        internalQueue = &localVarTable.Insert(L"#Q", MODULE_IRRELEVANT, Var());
        internalQueue->InitInternalQueue();
    }
    auto qui = internalQueue->GetInternalQueue();
    qui->push_back(qi);
}

//statement processors
//
//

void St_CONST(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    bool global = false;
    if (parser.IsKeywordOf(ST_GLOBAL))
    {
        global = true;
        parser.ReadInOneWord();
    }

    if (parser.wordType != WordReader::WordType::Identifier)
        throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);


    //Left side of '='
    Var* left { nullptr };
    if (global)
        left = parser.globalVarTable.Find(parser.strWord, 0, parser.lenString);
    else
        left = localVarTable.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);
    
    if (left != nullptr)
        throw Var(Exception::ERROR_CODE::VAR_REDFINED);

    if (global)
        left = &(parser.globalVarTable.Insert(parser.strWord, 0, parser.lenString, Var()));
    else
        left = &(localVarTable.Insert(parser.strWord, 0, parser.lenString, Var()));

    //Check '='
    parser.ReadInOneWord();
    if (!parser.IsKeywordOf(OP_EQUAL))
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);    
    //Calculate the right side of '=' and assign the value to the left side.
    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    *left = expr.Calculate();
    left->SetVarType(left->GetVarType() | VarType::ConstFlag);
}

void St_PRINT(HashTable<Var>& localVarTable, Parser& parser)
{
    bool shouldCR = true;
    parser.ReadInOneWord();
    while (!parser.IsEndOfStatement())
    {
        shouldCR = true;
        Expression expr(parser, localVarTable);
        Var result = expr.Calculate();
        switch (result.GetVarType())
        {
        case VarType::Boolean:
            std::wcout << (result.GetBool() ? L"True" : L"False");
            break;
        case VarType::Char:
            std::wcout << result.GetChar();
            break;
        case VarType::Integer:
            std::wcout << result.GetInteger();
            break;
        case VarType::Real:
            std::wcout << result.GetReal();
            break;
        case VarType::String:
            std::wcout << result.GetString();
            break;
        case VarType::Null:
            //Ignore null.
            break;
        default:
            throw Var(Exception::ERROR_CODE::NONE_PRINTABLE_DATA);
        }
        if (parser.wordType == WordReader::WordType::Keyword)
        {
            if (parser.keywordIdx == OP_COMMA)
            {
                std::wcout << L"\t";
                parser.ReadInOneWord();
                shouldCR = false;
            }
            else if (parser.keywordIdx == OP_SEMICOLON)
            {
                parser.ReadInOneWord();
                shouldCR = false;
            }
        }
    }
    if (shouldCR)
        std::wcout << std::endl;
}

void St_INPUT(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    if (parser.wordType == WordReader::WordType::String)
    {
        //If there is a prompt string
        wchar_t* buffer = new wchar_t[parser.lenString + 1];
        std::wcsncpy(buffer, parser.strWord, parser.lenString);
        buffer[parser.lenString] = 0;
        std::wcout << buffer;

        parser.ReadInOneWord();
        //Check for comma.
        if (!parser.IsKeywordOf(OP_COMMA))
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        
        parser.ReadInOneWord();
    }
    while (!parser.IsEndOfStatement())
    {
        Expression expr(parser, localVarTable);
        Var* var = expr.GetLeftValue();
        if (var == nullptr)
            throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
        std::wstring str;
        std::wcin >> str;
        wchar_t* end;
        VarType varType = var->GetVarType();
        if (parser.IsKeywordOf(ST_AS))
        {
            //Type is defined.
            parser.ReadInOneWord();
            if (parser.wordType == WordReader::WordType::Keyword && (parser.keywordIdx & KT_DATATYPE) > 0)
            {
                switch (parser.keywordIdx)
                {
                case DT_BOOLEAN:
                    varType = VarType::Boolean;
                    break;
                case DT_CHAR:
                    varType = VarType::Char;
                    break;
                case DT_INTEGER:
                    varType = VarType::Integer;
                    break;
                case DT_REAL:
                    varType = VarType::Real;
                    break;
                case DT_STRING:
                    varType = VarType::String;
                    break;
                case DT_MEMORY:
                    varType = VarType::Memory;
                    break;
                case DT_ARRAY:
                    varType = VarType::Array;
                    break;
                }
            }
            else
                throw Var(Exception::ERROR_CODE::NONE_INPUTABLE_TYPE);
            
            parser.ReadInOneWord();
        }
        switch (varType)
        {
        case VarType::Boolean:
            if (str == L"T" || str == L"TRUE" || str == L"t" || str == L"True" || str == L"true")
                var->SetBool(true);
            else if (str == L"F" || str == L"FALSE" || str == L"f" || str == L"False" || str == L"false")
                var->SetBool(false);
            else
                throw Var(Exception::ERROR_CODE::INPUT_BOOLEAN_REQUIRED);
            break;
        case VarType::Char:
            if (str.size() == 0 || str.size() > 1)
                throw Var(Exception::ERROR_CODE::INPUT_CHAR_REQUIRED);
            var->SetChar(str.at(0));
            break;
        case VarType::Integer:
            var->SetInteger(std::wcstol(str.c_str(), &end, 10));
            break;
        case VarType::Real:
            var->SetReal(std::wcstol(str.c_str(), &end, 10));
            break;
        case VarType::String:
            var->SetString(str.c_str());
            break;
        case VarType::Null:
            var->SetString(str.c_str());
            break;
        default:
            throw Var(Exception::ERROR_CODE::NONE_INPUTABLE_TYPE);
        }

        if (parser.IsKeywordOf(OP_COMMA))
            parser.ReadInOneWord();
    }
}

//St_DIM helper. Creating multi-dimesion array
void InitDimension(Var& dim, std::vector<long>& sizes, int level, VarType& varType, Parser& parser, int& structureIdx)
{
    if (level == sizes.size() - 1)
    {
        //Last layer
        if (structureIdx >= 0)
            dim.InitArray(sizes[level], parser.structureTable, structureIdx);
        else
            dim.InitArray(sizes[level], varType);
    }
    else
    {
        dim.InitArray(sizes[level], VarType::Null);
        for (long n = 0; n < sizes[level]; n++)
            InitDimension(dim.GetElementAt(n), sizes, level + 1, varType, parser, structureIdx);
    }
}

void St_DIM(HashTable<Var>& localVarTable, Parser& parser)
{
    //DIM [GLOBAL] [@(varName) | [BYREF] varName][[sizeOfArray]] [AS [moduleName.]Type]
    bool global = false;
    if (&localVarTable == &parser.globalVarTable)
        global = true;

    //Save current position. I should go back to this position for the var list.
    //But for now, I must check the [AS type] grammar.
    long posSave = parser.pos;
    long posNextSave = parser.posNext;

    parser.ReadInOneWord();
    if (parser.IsKeywordOf(ST_GLOBAL))
    {
        global = true;
        posSave = parser.pos;
        posNextSave = parser.posNext;
        parser.ReadInOneWord();
    }

    //Now, find the 'AS'
    VarType varType = VarType::Null;
    int structureIndex = -1;
    while (!parser.IsEndOfStatement())
    {
        parser.ReadInOneWord();         //Read ahead to avoid 'AS type' being written before the var list.
        if (parser.IsKeywordOf(ST_AS))
        {
            //Type is defined.
            parser.ReadInOneWord();
            if (parser.wordType == WordReader::WordType::Keyword && (parser.keywordIdx & KT_DATATYPE) > 0)
            {
                switch (parser.keywordIdx)
                {
                case DT_BOOLEAN:
                    varType = VarType::Boolean;
                    break;
                case DT_CHAR:
                    varType = VarType::Char;
                    break;
                case DT_INTEGER:
                    varType = VarType::Integer;
                    break;
                case DT_REAL:
                    varType = VarType::Real;
                    break;
                case DT_STRING:
                    varType = VarType::String;
                    break;
                case DT_MEMORY:
                    varType = VarType::Memory;
                    break;
                case DT_ARRAY:
                    varType = VarType::Array;
                    break;
                }
            }
            else if (parser.wordType == WordReader::WordType::Identifier)
            {
                int moduleIdx = parser.IdentifyModule(parser.currentModule);
                int* idx = parser.structureNameIndex.Find(parser.strWord, moduleIdx, parser.lenString);
                if (idx == nullptr) //Not in current module, try public module
                    idx = parser.structureNameIndex.Find(parser.strWord, 0, parser.lenString);
                if (idx == nullptr)
                    throw Var(Exception::ERROR_CODE::UNDEFINED_TYPE);
                structureIndex = *idx;
                varType = VarType::Structure;
            }
            else
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            
            parser.ReadInOneWord();
            break;
        }
    }
    //Here should be end of the line.
    if (!parser.IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);

    //Restore parser position to var list
    parser.pos = posSave;
    parser.posNext = posNextSave;
    parser.ReadInOneWord();

    while (true)    //The var list.
    {
        //There may be a 'BYREF'
        bool shouldByref = false;
        if (parser.IsKeywordOf(ST_BYREF))
        {
            shouldByref = true;
            parser.ReadInOneWord();
        }

        bool dimForRef = false;
        if (parser.IsKeywordOf(OP_AT))
        {
            dimForRef = true;
            parser.ReadInOneWord();
            if (!parser.IsKeywordOf(OP_LEFTBRACKET))
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            parser.ReadInOneWord();
        }

        if (parser.wordType != WordReader::WordType::Identifier)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);

        Var* var;
        if (dimForRef)
        {
            //May be a structure member
            Expression expr(parser, localVarTable);
            var = expr.GetLeftValue();
            if (var == nullptr)
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            if (!parser.IsKeywordOf(OP_RIGHTBRACKET))
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            //If the type of var is not Null, it should be the same as varType and structureIndex.
            if (var->GetVarType() != VarType::Null)
            {
                if (var->GetVarType() != varType)
                    throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
                else if (varType == VarType::Structure && var->GetReferedStructureIdx() != structureIndex)
                {
                    //Should check the inheritance relationship.
                    long idx = structureIndex;
                    while (idx != var->GetReferedStructureIdx())
                    {
                        idx = parser.structureTable[idx].parentStructIndex;
                        if (idx == -1)
                            throw Var(Exception::ERROR_CODE::CANNOT_REFER_TO_DIFFERENT_TYPE);
                    }
                }
            }
        }
        else if (global)
            var = parser.globalVarTable.Find(parser.strWord, parser.currentModule, parser.lenString);
        else
            var = localVarTable.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);

        if (!dimForRef && var != nullptr && !global)
            throw Var(Exception::ERROR_CODE::VAR_REDFINED);
        else if (dimForRef && var == nullptr)
            throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
        else if (dimForRef && !var->IsReference())
            throw Var(Exception::ERROR_CODE::NOT_A_REF_VAR);

        //Save name
        wchar_t *name = new wchar_t[parser.lenString + 1];
        std::wcsncpy(name, parser.strWord, parser.lenString);
        name[parser.lenString] = 0;
        
        //Var size;
        std::vector<long> sizes;
        parser.ReadInOneWord();
        while (parser.IsKeywordOf(OP_LEFTSQBRACKET))
        {
            parser.ReadInOneWord();
            //Should be an array
            Expression expr(parser, localVarTable);
            Var size = expr.Calculate();
            if (!size.GetVarType().IsNumeric() || size.GetInteger() <= 0)
                throw Var(Exception::ERROR_CODE::ARRAY_SIZE_REQUIRED);
            sizes.push_back(size.GetInteger());
            //Check right bracket.
            if (!parser.IsKeywordOf(OP_RIGHTSQBRACKET))
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            parser.ReadInOneWord();
        }

        VarType varTypeToSet = varType;
        if (shouldByref)
        {
            varTypeToSet.SetReference();
            if (dimForRef)
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        }

        if (dimForRef)
        {
//            Var& newVar = global ? parser.globalVarTable.InsertAnonymous(Var()) : localVarTable.InsertAnonymous(Var());
            //Dynamic memory allocation always allocates on global.
            Var& newVar = parser.globalVarTable.InsertAnonymous(Var());

            if (sizes.size() > 1)
                throw Var(Exception::ERROR_CODE::ONE_DIM_FOR_REF_ONLY);

            if (sizes.size() == 1 && sizes[0] > 0)  //Array or memory
            {
                if (structureIndex >= 0)    //So that I should Initialize each element to this structure
                    newVar.InitArray(sizes[0], parser.structureTable, structureIndex);
                else if (varTypeToSet == VarType::Memory)
                    newVar.InitMemory(sizes[0]);
                else    //So that this array is intialized with built-in var-type.
                    newVar.InitArray(sizes[0], varTypeToSet);   
            }
            else if (structureIndex >= 0) //Structure
                newVar.InitStructure(parser.structureTable, structureIndex);
            else if (varTypeToSet == VarType::Memory)
                throw Var(Exception::ERROR_CODE::MEMORY_SHOULD_BE_ARRAY);
            else
                newVar.SetVarType(varTypeToSet); //varType contains reference flag
            //If dim for a ref variable, check for compatibility
            if (!var->GetVarType().IsCompatible(newVar.GetVarType()))
                throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
//            if (var->HasReferedTo())
//                var->DecreaseReferCount();
            var->ReferTo(&newVar, parser);
//            newVar.IncreaseReferCount();    //Of course can use var->IncreaseReferCount();
        }
        else if (global && var != nullptr)
        {
            //Allow recurrence of 'dim global' the same var, but types should be checked.
            if (var->GetVarType() != varTypeToSet)
            {
                throw Var(Exception::ERROR_CODE::DIM_GLOBAL_MISMATCH);
            }
        }
        else
        {
            Var& newVar = global ? parser.globalVarTable.Insert(name, parser.currentModule, Var()) : localVarTable.Insert(name, MODULE_IRRELEVANT, Var());
            if (sizes.size() >= 1 && sizes[0] > 0)  //Array or memory
            {
                if (varTypeToSet == VarType::Memory)
                    newVar.InitMemory(sizes[0]);
                else
                    InitDimension(newVar, sizes, 0, varTypeToSet, parser, structureIndex);
            }
            else if (structureIndex >= 0) //Structure
            {
                if (varTypeToSet.IsReference())
                    newVar.SetVarType(VarType::Structure | VarType::ReferenceFlag, structureIndex);
                else
                    newVar.InitStructure(parser.structureTable, structureIndex);
            }
            else if (varTypeToSet == VarType::Memory)
                throw Var(Exception::ERROR_CODE::MEMORY_SHOULD_BE_ARRAY);
            else
                newVar.SetVarType(varTypeToSet); //varType contains reference flag
        }
        delete[] name;

        if (parser.IsKeywordOf(ST_AS) || parser.IsEndOfStatement())
            break;
        else if (!parser.IsKeywordOf(OP_COMMA))
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        else
            parser.ReadInOneWord();
    }

    parser.SkipToNextLine();
}

//REDIM can only be used to redefine the size of an array.
void St_REDIM(HashTable<Var>& localVarTable, Parser& parser)
{
    //REDIM [GLOBAL] arrayName[sizeOfArray]
    bool global = false;
    parser.ReadInOneWord();
    if (parser.IsKeywordOf(ST_GLOBAL))
    {
        global = true;
        parser.ReadInOneWord();
    }

    bool dimForRef = false;
    if (parser.IsKeywordOf(OP_AT))
    {
        dimForRef = true;
        parser.ReadInOneWord();
        if (!parser.IsKeywordOf(OP_LEFTBRACKET))
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        parser.ReadInOneWord();
    }

    Var* var;
    if (dimForRef)
    {
        //May be a structure member
        Expression expr(parser, localVarTable);
        var = expr.GetLeftValue();
        if (var == nullptr)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        if (!parser.IsKeywordOf(OP_RIGHTBRACKET))
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    }
    else if (parser.IsKeywordOf(OP_LEFTBRACKET))
    {
        parser.ReadInOneWord();
        Expression expr(parser, localVarTable);
        var = expr.GetLeftValue();
        if (!parser.IsKeywordOf(OP_RIGHTBRACKET))
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    }
    else
    {
        if (parser.wordType != WordReader::WordType::Identifier)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        if (global)
            var = parser.globalVarTable.Find(parser.strWord, parser.currentModule, parser.lenString);
        else
            var = localVarTable.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);
    }
    //Read in the new size
    Var size;
    parser.ReadInOneWord();
    if (!parser.IsKeywordOf(OP_LEFTSQBRACKET))
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    size = expr.Calculate();
    if (!size.GetVarType().IsNumeric())
        throw Var(Exception::ERROR_CODE::ARRAY_SIZE_REQUIRED);
    if (size.GetInteger() <= 0)
         throw Var(Exception::ERROR_CODE::ARRAY_SIZE_REQUIRED);
    //Check right bracket.
    if (!parser.IsKeywordOf(OP_RIGHTSQBRACKET))
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    parser.ReadInOneWord();

    if (var == nullptr)
        throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);

    if (var->GetVarType() == VarType::Array)
        var->ResizeArray(size.GetInteger(), parser.structureTable);
    else if (var->GetVarType() == VarType::Memory)
        var->ResizeMemory(size.GetInteger());
    else
        throw Var(Exception::ERROR_CODE::NOT_REDIMABLE);  //can only redim an array or a memory area.

    if (!parser.IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
}

void St_IF(HashTable<Var>& localVarTable, Parser& parser)
{
    //Next should be an expression that can return true/false.
    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    Var result = expr.Calculate();
    //Then there may be 'THEN', but is omittable.
    if (parser.IsKeywordOf(ST_THEN))
        parser.ReadInOneWord();
    if (!parser.IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    if (!result.GetBool())
    {
        //Skip to 'ENDIF' or 'ELSE'
        int countIf = 0;    //Should skip nested if.
        while (!(parser.IsKeywordOf(ST_ENDIF) || parser.IsKeywordOf(ST_ELSE)) || countIf > 0)
        {
            parser.SkipToNextLine();
            parser.ReadInOneWord();
            if (parser.IsKeywordOf(ST_IF))
                countIf++;
            if (parser.IsKeywordOf(ST_ENDIF))
            {
                countIf--;
                if (countIf >= 0)
                    parser.keywordIdx = -1; //To avoid being recognized as ENDIF of this level.
            }
            if (parser.IsKeywordOf(ST_ENDFN) || parser.wordType == WordReader::WordType::EndOfFile)
                throw Var(Exception::ERROR_CODE::IF_WITHOUT_ENDIF);
        }
    }
}

void St_ELSE(HashTable<Var>& localVarTable, Parser& parser)
{
    //Skip to 'ENDIF'
    int countIf = 0;
    while (!parser.IsKeywordOf(ST_ENDIF) || countIf > 0)
    {
        parser.SkipToNextLine();
        parser.ReadInOneWord();
        if (parser.IsKeywordOf(ST_IF))
            countIf++;
        if (parser.IsKeywordOf(ST_ENDIF))
        {
            countIf--;
            if (countIf >= 0)
                parser.keywordIdx = -1; //To avoid being recognized as ENDIF of this level.
        }
        if (parser.IsKeywordOf(ST_ENDFN) || parser.wordType == WordReader::WordType::EndOfFile)
            throw Var(Exception::ERROR_CODE::IF_WITHOUT_ENDIF);
    }
    parser.MoveToNextLine();
}

void St_ENDIF(HashTable<Var>& localVarTable, Parser& parser)
{
    //Move to next line. So that parser.execute will execute the next line.
    parser.MoveToNextLine();
}

void St_FOR(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    Var* left = parser.ParseAssignment(expr, localVarTable);
    if (left) //If it is a left value.
    {
        if (!parser.IsKeywordOf(ST_TO))
            throw Var(Exception::ERROR_CODE::FOR_WITHOUT_TO);
        parser.ReadInOneWord();
        Var&& to = expr.Calculate();
        double step = 1;
        if (parser.IsKeywordOf(ST_STEP))
        {
            parser.ReadInOneWord();
            step = expr.Calculate().GetReal();
            if (step == 0)
                throw Var(Exception::ERROR_CODE::STEP_IS_ZERO);
        }
        if (!parser.IsEndOfStatement())
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        
        Var::QueueInfo qi;
        qi.srcPosSave = parser.posNext;
        qi.controllerVar = left;
        qi.to = to.GetReal();
        qi.step = step;
        qi.type = ST_FOR;

        if (step > 0 && left->GetReal() > qi.to || step < 0 && left->GetReal() < qi.to)
        {
            //No need to start this loop.
            //Skip to ENDFOR/NEXT
            while (parser.wordType != WordReader::WordType::EndOfFile)
            {
                parser.ReadInOneWord();
                if (parser.IsKeywordOf(ST_ENDFOR))
                    return;
                parser.SkipToNextLine();
            }
        }

        InsertIntoInternalQueue(localVarTable, qi);
    }
    else
        throw Var(Exception::ERROR_CODE::FOR_WITHOUT_INDEX_VAR);
}

void St_ENDFOR(HashTable<Var>& localVarTable, Parser& parser)
{
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::NEXT_WITHOUT_FOR);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();

    int pos = static_cast<int>(qi->size());
    if (pos == 0)
        throw Var(Exception::ERROR_CODE::NEXT_WITHOUT_FOR);
    pos--;
    if (qi->at(pos).type == ST_FOR)
    {    
        double next = qi->at(pos).controllerVar->GetReal() + qi->at(pos).step;
        qi->at(pos).controllerVar->SetReal(next);
        if (qi->at(pos).step > 0 && next > qi->at(pos).to || 
            qi->at(pos).step < 0 && next < qi->at(pos).to)
        {
            //FOR loop ends here.
            if (pos == 0)
                qi->clear();
            else
                qi->resize(pos);
            parser.MoveToNextLine();
        }
        else
        {
            parser.pos = parser.posNext = qi->at(pos).srcPosSave;
        }
    }
    else if (qi->at(pos).type == ST_FOREACH)
    {
        if (qi->at(pos).index < qi->at(pos).iterateVar->GetDataSize())
        {
            switch (qi->at(pos).iterateVar->GetVarType())
            {
            case VarType::Array:
                qi->at(pos).controllerVar->ResetAndReferTo(&(qi->at(pos).iterateVar->GetElementAt(qi->at(pos).index)));
                break;
            case VarType::Structure:
                qi->at(pos).controllerVar->ResetAndReferTo(&(qi->at(pos).iterateVar->GetStructureMember(qi->at(pos).index)));
                break;
            case VarType::Memory:
                break;
            }
            qi->at(pos).index++;
            parser.MovePosition(qi->at(pos).srcPosSave);
        }
        else
        {
            //LOOP loop ends here.
            if (pos == 0)
                qi->clear();
            else
                qi->resize(pos);
        }
    }
    else
        throw Var(Exception::ERROR_CODE::DIFF_ST_CANNOT_CROSS);
}

void St_WHILE(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    Var::QueueInfo qi;
    qi.srcPosSave = parser.pos;
    qi.type = ST_WHILE;

    Expression expr(parser, localVarTable);
    Var&& result = expr.Calculate();
    if (!parser.IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    if (result.GetBool())
        InsertIntoInternalQueue(localVarTable, qi);
    else
    {
        //Skip to ENDWHILE/WEND
        while (parser.wordType != WordReader::WordType::EndOfFile)
        {
            parser.ReadInOneWord();
            if (parser.IsKeywordOf(ST_ENDWHILE))
                return;
            parser.SkipToNextLine();
        }
    }
}

void St_ENDWHILE(HashTable<Var>& localVarTable, Parser& parser)
{
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::ENDWHILE_WITHOUT_WHILE);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();

    int pos = static_cast<int>(qi->size());
    if (pos == 0)
        throw Var(Exception::ERROR_CODE::ENDWHILE_WITHOUT_WHILE);
    pos--;
    if (qi->at(pos).type != ST_WHILE)
        throw Var(Exception::ERROR_CODE::DIFF_ST_CANNOT_CROSS);

    //Save status
    size_t posSave = parser.posNext;

    parser.MovePosition(qi->at(pos).srcPosSave);
    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    Var&& result = expr.Calculate();
    if (!result.GetBool())
    {
        //WHILE loop ends here.
        if (pos == 0)
            qi->clear();
        else
            qi->resize(pos);
        parser.MovePosition(posSave);
        parser.MoveToNextLine();
    }
}

void St_LOOP(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    if (!parser.IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::EOL_EXPECTED);

    Var::QueueInfo qi;
    qi.srcPosSave = parser.posNext;
    qi.type = ST_LOOP;
    InsertIntoInternalQueue(localVarTable, qi);
}

void St_ENDLOOP(HashTable<Var>& localVarTable, Parser& parser)
{
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::ENDLOOP_WITHOUT_LOOP);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();

    int pos = static_cast<int>(qi->size());
    if (pos == 0)
        throw Var(Exception::ERROR_CODE::ENDWHILE_WITHOUT_WHILE);
    pos--;
    if (qi->at(pos).type != ST_LOOP)
        throw Var(Exception::ERROR_CODE::DIFF_ST_CANNOT_CROSS);

    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    Var&& result = expr.Calculate();
    if (result.GetBool())
    {
        //LOOP loop ends here.
        if (pos == 0)
            qi->clear();
        else
            qi->resize(pos);
    }
    else
    {
        parser.MovePosition(qi->at(pos).srcPosSave);
    }
}

void St_FOREACH(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    Var* left = expr.GetLeftValue();
    if (left)
    {
        if (!parser.IsKeywordOf(ST_IN))
            throw Var(Exception::ERROR_CODE::FOREACH_WITHOUT_IN);
        parser.ReadInOneWord();
        Var* iteratable = expr.GetLeftValue();
        if (iteratable == nullptr || !iteratable->GetVarType().IsIndexable())
            throw Var(Exception::ERROR_CODE::NOT_INDEXABLE);
        
        if (!parser.IsEndOfStatement())
            throw Var(Exception::ERROR_CODE::EOL_EXPECTED);
       
        Var::QueueInfo qi;
        qi.srcPosSave = parser.posNext;
        qi.controllerVar = left;
        qi.index = 0;
        qi.iterateVar = iteratable;
        qi.type = ST_FOREACH;

        if (qi.index < qi.iterateVar->GetDataSize())
        {
            switch (qi.iterateVar->GetVarType())
            {
            case VarType::Array:
                qi.controllerVar->ResetAndReferTo(&(qi.iterateVar->GetElementAt(qi.index)));
                break;
            case VarType::Structure:
                qi.controllerVar->ResetAndReferTo(&(qi.iterateVar->GetStructureMember(qi.index+1)));
                break;
            case VarType::Memory:
                break;
            }
            qi.index++;
            InsertIntoInternalQueue(localVarTable, qi);
        }
        else
        {
            //Skip to endforeach
            while (parser.wordType != WordReader::WordType::EndOfFile)
            {
                parser.ReadInOneWord();
                if (parser.IsKeywordOf(ST_ENDFOR))
                    return;
                parser.SkipToNextLine();
            }
        }
    }
    else
        throw Var(Exception::ERROR_CODE::FOREACH_WITHOUT_VAR);
}

void St_BREAK(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    bool breakAll = false;
    if (parser.IsKeywordOf(ST_ALL))
    {
        //Should break all
        breakAll = true;
        parser.ReadInOneWord();
    }
    //Look for the nearest loop
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::BREAK_WITHOUT_LOOP);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();

    int pos = static_cast<int>(qi->size() - 1);
    do
    {    
        int type;
        while (pos >= 0)
        {
            type = qi->at(pos).type;
            if (type == ST_FOR || type == ST_FOREACH || type == ST_WHILE || type == ST_LOOP)
                break;
            pos--;
        }
        if (pos < 0)
            throw Var(Exception::ERROR_CODE::BREAK_WITHOUT_LOOP);
        if (pos == 0)
            qi->clear();
        else
            qi->resize(pos);
        pos--;

        //look for the nearest endfor, endwhile, endloop, next, wend according to the loop type.
        while (parser.wordType != WordReader::WordType::EndOfFile)
        {
            parser.ReadInOneWord();
            if (parser.IsKeywordOf(ST_ENDFOR) && (type == ST_FOR || type == ST_FOREACH) ||
                parser.IsKeywordOf(ST_ENDWHILE) && type == ST_WHILE ||
                parser.IsKeywordOf(ST_ENDLOOP) && type == ST_LOOP)
            {
                parser.SkipToNextLine();
                break;
            }
            parser.SkipToNextLine();
        }
    } while (breakAll && pos >= 0);

//    if (!parser.IsEndOfStatement())
//        throw Var(Exception::ERROR_CODE::EOL_EXPECTED);
}

void St_SELECT(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    if (parser.IsKeywordOf(ST_CASE))    //The 'CASE' after 'SELECT' can be omitted.
        parser.ReadInOneWord();

    Expression expr(parser, localVarTable);
    Var&& result = expr.Calculate();
    if (!parser.IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::EOL_EXPECTED);
    
    Var* is = localVarTable.Find(L"IS", MODULE_IRRELEVANT, 2);
    if (is == nullptr)
        is = &localVarTable.Insert(L"IS", MODULE_IRRELEVANT, Var());
    else
        is->Clear();
    is->MoveFrom(std::move(result));

    Var::QueueInfo qi;
    qi.type = ST_SELECT;
    qi.controllerVar = is;
    qi.index = 1;   //As an indicator that this select has not matched
    InsertIntoInternalQueue(localVarTable, qi);
}

void St_ENDSELECT(HashTable<Var>& localVarTable, Parser& parser);
void St_CASE(HashTable<Var>& localVarTable, Parser& parser)
{
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::CASE_WITHOUT_SELECT);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();

    int pos = static_cast<int>(qi->size() - 1);
    if (pos < 0 || qi->at(pos).type != ST_SELECT)
        throw Var(Exception::ERROR_CODE::CASE_WITHOUT_SELECT);

    Var* is = qi->at(pos).controllerVar;
    if (qi->at(pos).index == 0)
    {
        //Skip to endselect
        is->SetVarType(VarType::Null);
        if (pos == 0)
            qi->clear();
        else
            qi->resize(pos);
        int selectCount = 1;
        while (parser.wordType != WordReader::WordType::EndOfFile)
        {
            parser.ReadInOneWord();
            if (parser.IsKeywordOf(ST_SELECT))
                selectCount++;
            else if (parser.IsKeywordOf(ST_ENDSELECT))
                selectCount--;
            if (parser.IsKeywordOf(ST_ENDSELECT) && selectCount <= 0)
            {
                return;
            }
            parser.SkipToNextLine();
        }
    }
    
    Expression expr(parser, localVarTable);

BackCase:
    bool match = false;
    parser.ReadInOneWord();
    if (parser.IsKeywordOf(ST_ELSE))
        match = true;

    while (match == false)
    {
        Var result1 = expr.Calculate();
        if (result1.GetVarType() == VarType::Boolean)
            match = result1.GetBool();
        else
        {
            if (parser.IsKeywordOf(ST_TO))
            {
                parser.ReadInOneWord();
                Var result2 = expr.Calculate();
                if (is->GetVarType().IsNumeric() && result1.GetVarType().IsNumeric() && result2.GetVarType().IsNumeric())
                {
                    if (is->GetReal() >= result1.GetReal() && is->GetReal() <= result2.GetReal())
                        match = true;
                }
                else if (is->GetVarType() == VarType::String && result1.GetVarType() == VarType::String && result2.GetVarType() == VarType::String)
                {
                    if (std::wcscmp(is->GetString(), result1.GetString()) >= 0 &&
                        std::wcscmp(is->GetString(), result2.GetString()) <= 0)
                    {
                        match = true;
                    }
                }
                else
                    throw Var(Exception::ERROR_CODE::UNSUPPROTE_TYPE_IN_CASE);
            }
            else
            {
                if (is->GetVarType().IsNumeric() && result1.GetVarType().IsNumeric())
                {
                    if (is->GetReal() == result1.GetReal())
                        match = true;
                }
                else if (is->GetVarType() == VarType::String && result1.GetVarType() == VarType::String)
                {
                    if (std::wcscmp(is->GetString(), result1.GetString()) == 0)
                        match = true;
                }
                else
                    throw Var(Exception::ERROR_CODE::UNSUPPROTE_TYPE_IN_CASE);
            }
        }
        if (match == false)
        {
            if (parser.IsKeywordOf(OP_COMMA))
                parser.ReadInOneWord();
            else
                break;
        }
    }
    if (match == true)
    {
        qi->at(pos).index = 0;
        parser.SkipToNextLine();
    }
    else
    {
        //Skip to next 'case' or 'endselect'
        int countSelect = 0;    //select-case may be nested.
        while (parser.wordType != WordReader::WordType::EndOfFile)
        {
            parser.ReadInOneWord();
            if (parser.IsKeywordOf(ST_SELECT))
                countSelect++;
            if (parser.IsKeywordOf(ST_ENDSELECT))
            {
                if (countSelect == 0)
                {
                    St_ENDSELECT(localVarTable, parser);
                    return;
                }
                else
                    countSelect--;
            }

            if (parser.IsKeywordOf(ST_CASE) && countSelect == 0)
            {
                goto BackCase;
            }
            parser.SkipToNextLine();
        }
    }
}

void St_ENDSELECT(HashTable<Var>& localVarTable, Parser& parser)
{
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::ENDSELECT_WITHOUT_SELECT);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();
    int pos = static_cast<int>(qi->size() - 1);
    if (pos < 0 || qi->at(pos).type != ST_SELECT)
        throw Var(Exception::ERROR_CODE::ENDSELECT_WITHOUT_SELECT);
    if (pos == 0)   //vector::resize(0) works but vector::size() will return a weird value and crash the program.
        qi->clear();
    else
        qi->resize(pos);
    
    Var* is = localVarTable.Find(L"IS", MODULE_IRRELEVANT, 2);
    if (is != nullptr)
        is->SetVarType(VarType::Null);
    parser.SkipToNextLine();
}

void St_TRY(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.MoveToNextLine();    
    Var::QueueInfo qi;
    qi.type = ST_TRY;
    qi.srcPosSave = parser.pos;
    qi.to = 0;  // = 0 means currently in the try block. will be set to 1 when we are inside a catch block
    InsertIntoInternalQueue(localVarTable, qi);
}

void St_CATCH(HashTable<Var>& localVarTable, Parser& parser)
{
    //If CATCH is executed, it means there is no error happened inside the try block or the error has been handled by another catch block.
    //Just skip to ENDTRY
    int countTry = 0;    //TRY may be nested
    parser.SkipToNextLine();
    while (parser.wordType != WordReader::WordType::EndOfFile)
    {
        parser.ReadInOneWord();
        if (parser.IsKeywordOf(ST_TRY))
            countTry++;
        else if (parser.IsKeywordOf(ST_ENDTRY))
        {
            if (countTry == 0)
            {
                Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
                if (queueVar == nullptr)
                    throw Var(Exception::ERROR_CODE::ENDTRY_WITHOUT_TRY);

                std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();
                int pos = static_cast<int>(qi->size()) - 1;
                if (pos < 0 || qi->at(pos).type != ST_TRY)
                    throw Var(Exception::ERROR_CODE::ENDTRY_WITHOUT_TRY);
                if (pos == 0)
                    qi->clear();
                else
                    qi->resize(pos);

                return;
            }
            countTry--;
        }
        parser.SkipToNextLine();
    }
}

bool Helper_FindCatchSection(HashTable<Var>& localVarTable, Parser& parser, Var& toThrow)
{
    //Find the appropriate catch section.
    int countTry = 0;
    while (parser.wordType != WordReader::WordType::EndOfFile)
    {
        parser.ReadInOneWord();
        if (parser.IsKeywordOf(ST_TRY))
            countTry++;
        else if (parser.IsKeywordOf(ST_ENDTRY))
        {
            if (countTry == 0)
                return false;   
           countTry--;
        }
        else if (parser.IsKeywordOf(ST_CATCH) && countTry == 0)
        {
            parser.ReadInOneWord();
            if (parser.wordType == WordReader::WordType::Identifier)
            {
                Var* catchVar = localVarTable.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);
                if (catchVar == nullptr)
                    catchVar = &localVarTable.Insert(parser.strWord, MODULE_IRRELEVANT, parser.lenString, Var());
                catchVar->Clear();
                *catchVar = toThrow;

                parser.ReadInOneWord();
                if (parser.IsKeywordOf(ST_AS))
                {
                    parser.ReadInOneWord();
                    if (parser.wordType == WordReader::WordType::Keyword)
                    {
                        //It must be one of the built-in type.
                        int varType = VarType::ConvertWordReaderDataType(parser.keywordIdx);
                        if (varType == VarType::NotAType)
                            throw Var(Exception::ERROR_CODE::UNDEFINED_TYPE);
                        if (varType == toThrow.GetVarType())
                            break;  //Break so that the next statement, which is the first statement inside this catch, will be executed
                    }
                    else if (parser.wordType == WordReader::WordType::Identifier)
                    {
                        //Maybe a structure
                        int* idx = parser.structureNameIndex.Find(parser.strWord, parser.currentModule, parser.lenString);
                        if (idx == nullptr)
                            throw Var(Exception::ERROR_CODE::UNDEFINED_TYPE);
                        //Yes its a structure. Now, check if catchVar is of this structure or is the decsendent of this structure
                        if (catchVar->GetVarType() == VarType::Structure)
                        {
                            int rvalueStructIdx = catchVar->GetStructureMember(-1).GetInteger();
                            while (rvalueStructIdx != *idx && rvalueStructIdx != -1)
                            {
                                rvalueStructIdx = parser.structureTable[rvalueStructIdx].parentStructIndex;
                            }
                            if (rvalueStructIdx == *idx)
                                break;
                        }
                    }
                    else
                        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
                }
                else if (parser.IsEndOfStatement())
                    break;
                else
                    throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            }
            else if (parser.IsEndOfStatement())
                break;
            else
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        }
        parser.SkipToNextLine();
    } 
    return true;
}

void St_THROW(HashTable<Var>& localVarTable, Parser& parser)
{
    parser.ReadInOneWord();
    Expression expr(parser, localVarTable);
    Var&& toThrow = parser.IsEndOfStatement() ? Var() : expr.Calculate();
    if (parser.IsEndOfStatement())
        throw toThrow;
    else
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
}

void St_RETRY(HashTable<Var>& localVarTable, Parser& parser)
{
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::RETRY_NOT_IN_CATCH);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();
    int pos = static_cast<int>(qi->size()) - 1;
    if (pos < 0 || qi->at(pos).type != ST_TRY || qi->at(pos).to != 1)
        throw Var(Exception::ERROR_CODE::RETRY_NOT_IN_CATCH);
    
    qi->at(pos).to = 0; //Now, not in a catch block.
    parser.MovePosition(qi->at(pos).srcPosSave);    //Go back to the try part
}

void St_ENDTRY(HashTable<Var>& localVarTable, Parser& parser)
{
    Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
    if (queueVar == nullptr)
        throw Var(Exception::ERROR_CODE::ENDTRY_WITHOUT_TRY);
    std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();
    int pos = static_cast<int>(qi->size()) - 1;
    if (pos < 0 || qi->at(pos).type != ST_TRY)
        throw Var(Exception::ERROR_CODE::ENDTRY_WITHOUT_TRY);
    if (pos == 0)
        qi->clear();
    else
        qi->resize(pos);
}

void St_IMPORT(HashTable<Var>& localVarTable, Parser& parser)
{
    bool loadDLL = false;
    parser.ReadInOneWord();
    if (parser.IsKeywordOf(ST_LIB))
    {
        loadDLL = true;
        parser.ReadInOneWord();
    }
    if (parser.wordType != WordReader::WordType::String)
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    std::wstring wName;
    for (int i = 0; i < parser.lenString; i++)
    {
        wName.push_back(parser.strWord[i]);
    }
    std::string fileName = wchar_filename_to_string(wName.c_str());

    wName = L"";
    parser.ReadInOneWord();
    if (parser.IsKeywordOf(ST_AS))
    {
        parser.ReadInOneWord();
        if (parser.wordType != WordReader::WordType::Identifier)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);

        //Module name
        int i;
        for (i = 0; i < parser.lenString; i++)
            wName.push_back(parser.strWord[i]);
        parser.ReadInOneWord();
    }

    if (!parser.IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    
    if (loadDLL)
        parser.ImportDLL(fileName, wName.c_str());
    else
        parser.ImportSource(fileName, wName.c_str());
    parser.wordType = WordReader::WordType::EndOfLine;   //Import makes next wordType to be EndOfFile. So, let's set back
}

void St_FN(HashTable<Var>& localVarTable, Parser& parser)
{
    FunctionDefinition funcDef;
    int paramIdx = 0;
    int& moduleIdx = parser.currentModule;

    //FN [StructureName.]FunctionName(Param0 AS datatype [BYREF], Param1 AS datatype [BYREF], ...) [AS datatype]
    // ....
    //ENDFN

    //For member functions of a structure, the name stored into function table will be:
    //   FunctioName{#Index}
    //in which 'Index' is the structure definition index ('{', '}' are seperators and are not included in the name).
    //And the first parameter of this kind of function is defaulty set to be 'self as StructureName byref'.

    parser.ReadInOneWord();
    //Function name should be an identifier and must be unique.
    if (parser.wordType != WordReader::WordType::Identifier)
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);

    //Temporarily save the function/structure name
    wchar_t* funcName = new wchar_t[parser.lenString + 20];
    for (int i = 0; i < parser.lenString; i++)
        funcName[i] = parser.strWord[i];
    funcName[parser.lenString] = 0;

    parser.ReadInOneWord();
    //This may be a '.'
    if (parser.IsKeywordOf(OP_DOT))
    {
        //Search the structure
        int *structIdx = parser.structureNameIndex.Find(funcName, moduleIdx, parser.lenString);
        if (structIdx == nullptr)
            throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED); //structure not found.
        //Followed with member function name.
        parser.ReadInOneWord();
        //Function name should be an identifier and must be unique.
        if (parser.wordType != WordReader::WordType::Identifier)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        //Let the function name to be FunctionName{#Index}
        int i;
        for (i = 0; i < parser.lenString; i++)
            funcName[i] = parser.strWord[i];
        funcName[i] = L'#';
        parser.IntToWCS(*structIdx, funcName + parser.lenString + 1);

        //Generate first and default parameter of 'self as StructureName byref'
        funcDef.paramIndexes.Insert(L"self", MODULE_IRRELEVANT, paramIdx++);
        funcDef.paramTypes.push_back(VarType::Structure);
        funcDef.structParamIndex.push_back(*structIdx);

        parser.ReadInOneWord();
    }
    auto find = parser.functionNameIndex.Find(parser.strWord, moduleIdx, parser.lenString);
    if (find != nullptr)
        throw Var(Exception::ERROR_CODE::CANNOT_REDEFINE_FUNC);

    //Now, build the parameter table.
    //There should be a '('
    if (!parser.IsKeywordOf(OP_LEFTBRACKET))
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);

    parser.ReadInOneWord();
    while (!parser.IsKeywordOf(OP_RIGHTBRACKET))
    {
        bool shouldByref = false;
        //There may be BYREF before a param name.
        if (parser.IsKeywordOf(ST_BYREF))
        {
            shouldByref = true;
            parser.ReadInOneWord();
        }
        //param name
        wchar_t* paramName = new wchar_t[parser.lenString + 1];
        for (int i = 0; i < parser.lenString; i++)
            paramName[i] = parser.strWord[i];
        paramName[parser.lenString] = 0;
        //param name should not be duplicated
        if (funcDef.paramIndexes.Find(paramName, MODULE_IRRELEVANT, parser.lenString) != nullptr)
            throw Var(Exception::ERROR_CODE::PARAM_NAME_SHOULD_BE_UNIQUE);

        VarType paramType = VarType::Null;
        int structIndex = -1;
        parser.ReadInOneWord();
        if (parser.IsKeywordOf(ST_AS))
        {
            parser.ReadInOneWord();
            if (parser.wordType == WordReader::WordType::Keyword && (parser.keywordIdx & KT_DATATYPE) != 0)
            {
                //Built-in data type
                paramType = VarType::ConvertWordReaderDataType(parser.keywordIdx);
            }
            else if (parser.wordType == WordReader::WordType::Identifier)
            {
                //Try structure
                int moduleToFind = parser.IdentifyModule(moduleIdx);
                paramType = VarType::Structure;
                //Find the identifier from structure name-index map
                int* idx = parser.structureNameIndex.Find(parser.strWord, moduleToFind, parser.lenString);
                if (idx == nullptr) //Try public module
                    idx = parser.structureNameIndex.Find(parser.strWord, 0, parser.lenString);
                if (idx == nullptr)
                    throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
                else
                    structIndex = *idx;
            }
            else
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            
            parser.ReadInOneWord();
        }

        if (shouldByref)
        {
            paramType = paramType | VarType::ReferenceFlag;
        }

        funcDef.paramIndexes.Insert(paramName, MODULE_IRRELEVANT, paramIdx++);
        funcDef.paramTypes.push_back(paramType);
        funcDef.structParamIndex.push_back(structIndex);
        delete[] paramName;

        if (parser.IsKeywordOf(OP_COMMA))
            parser.ReadInOneWord();
        else if (!parser.IsKeywordOf(OP_RIGHTBRACKET))
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    }

    //Now, the parameter table has been parsed and saved to funcDef.
    parser.ReadInOneWord();
    //If there is an "AS" following the function parameter table
    if (parser.IsKeywordOf(ST_AS))
    {
        parser.ReadInOneWord();

        if (parser.wordType == WordReader::WordType::Keyword && (parser.keywordIdx & KT_DATATYPE) != 0)
        {
            //The return type is a built-in type
            funcDef.returnType = VarType::ConvertWordReaderDataType(parser.keywordIdx);
        }
        else if (parser.wordType == WordReader::WordType::Identifier)
        {
            //The return type may be a structure
            int* idx = parser.structureNameIndex.Find(parser.strWord, moduleIdx, parser.lenString);
            if (idx == nullptr)
                throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
            funcDef.returnType = VarType::Structure;
            funcDef.returnStructIndex = *idx;
        }
        else
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        
        parser.ReadInOneWord();
    }

    //Here is the end of line
    if (parser.wordType != WordReader::WordType::EndOfLine)
        throw Var(Exception::ERROR_CODE::ONE_STATEMENT_ONLY);

    //Store the code start position of this function 
    funcDef.sourceCodePosition = parser.posNext;

    //Push to the table
    funcDef.moduleIdx = moduleIdx;
    parser.functionTable.push_back(funcDef);
    parser.functionNameIndex.Insert(funcName, moduleIdx, static_cast<int>(parser.functionTable.size()) - 1);
    delete[] funcName;

    //Skip to 'ENDFN' or eof
    while (parser.wordType != WordReader::WordType::EndOfFile && 
            !parser.IsKeywordOf(ST_ENDFN))
    {
        if (parser.wordType != WordReader::WordType::EndOfLine)
            parser.SkipToNextLine();
        parser.ReadInOneWord();
    }

}

void St_STRUCT(HashTable<Var>& localVarTable, Parser& parser)
{
    StructureDefinition structDef;
    int memberIdx = 0;
    int& moduleIdx = parser.currentModule;

    //STRUCT name [EXTENDS ParentStructure]
    //   memberName [AS datatype]
    //ENDSTRUCT

    parser.ReadInOneWord();
    //Structure name should be an identifier and must be unique.
    if (parser.wordType != WordReader::WordType::Identifier)
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    auto find = parser.structureNameIndex.Find(parser.strWord, moduleIdx, parser.lenString);
    if (find != nullptr)
        throw Var(Exception::ERROR_CODE::CANNOT_REDEFINE_STRUCTURE);

    //Temporarily save the structure name
    wchar_t* structName = new wchar_t[parser.lenString + 10];
    for (int i = 0; i < parser.lenString; i++)
        structName[i] = parser.strWord[i];
    structName[parser.lenString] = 0;

    parser.ReadInOneWord();
    if (parser.IsKeywordOf(ST_EXTENDS))    //This structure has a parent
    {
        parser.ReadInOneWord();
        //This should be the parent structure's name
        if (parser.wordType != WordReader::WordType::Identifier)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        //And it must have been defined.
        //Find in globalVarTable first for module name.    
        int moduleToFind = parser.IdentifyModule(moduleIdx);          
        find = parser.structureNameIndex.Find(parser.strWord, moduleToFind, parser.lenString);
        if (find == nullptr) //Try public module
            find = parser.structureNameIndex.Find(parser.strWord, 0, parser.lenString);
        if (find == nullptr)
            throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
        structDef.parentStructIndex = *find;

        parser.ReadInOneWord();
    }
    while (!parser.IsKeywordOf(ST_ENDSTRUCT))
    {
        //Skip empty lines
        if (parser.wordType == WordReader::WordType::EndOfLine)
        {
            parser.ReadInOneWord();
            continue;
        }
        //Skip REM
        if (parser.IsKeywordOf(ST_REM))
        {
            parser.SkipToNextLine();
            parser.ReadInOneWord();
            continue;
        }

        //'DIM' is optional.
        if (parser.IsKeywordOf(ST_DIM))
            parser.ReadInOneWord();

        bool shouldByref = false;
        //There may be BYREF before member name.
        if (parser.IsKeywordOf(ST_BYREF))
        {
            shouldByref = true;
            parser.ReadInOneWord();
        }

        //Member name should be an identifier and should be unqiue inside the structure scope.
        if (parser.wordType != WordReader::WordType::Identifier)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        auto find = structDef.memberNames.Find(parser.strWord, MODULE_IRRELEVANT, parser.lenString);
        if (find != nullptr)
            throw Var(Exception::ERROR_CODE::CANNOT_REDEFINE_STRUCTURE);
        structDef.memberNames.Insert(parser.strWord, MODULE_IRRELEVANT, parser.lenString, memberIdx++);

        //If there is an '[' then this member is an array
        int arraySize = 0;
        parser.ReadInOneWord();
        if (parser.IsKeywordOf(OP_LEFTSQBRACKET))
        {
            parser.ReadInOneWord();
            HashTable<Var> localDummy;
            Expression expr(parser, localDummy);
            Var size = expr.Calculate();
            if (size.GetVarType() == VarType::Null || size.GetInteger() <= 0)
                throw Var(Exception::ERROR_CODE::ARRAY_SIZE_REQUIRED);
            arraySize = size.GetInteger();

            if (!parser.IsKeywordOf(OP_RIGHTSQBRACKET))
                throw Var(Exception::ERROR_CODE::RIGHT_SQBRACKET_EXPECTED);
            parser.ReadInOneWord();
        }

        //If there is an "AS" following the name
        VarType memberVarType { VarType::Null };
        int structureMemberIndex { -1 };
        if (parser.IsKeywordOf(ST_AS))
        {
            parser.ReadInOneWord();

            if (parser.wordType == WordReader::WordType::Keyword && (parser.keywordIdx & KT_DATATYPE) != 0)
            {
                //The member type is a built-in type
                memberVarType = VarType::ConvertWordReaderDataType(parser.keywordIdx);
            }
            else if (parser.wordType == WordReader::WordType::Identifier)
            {
                //The member type may be a structure
                int moduleToFind = parser.IdentifyModule(moduleIdx);
                int* idx = parser.structureNameIndex.Find(parser.strWord, moduleToFind, parser.lenString);
                if (idx == nullptr) //Try public module
                    idx = parser.structureNameIndex.Find(parser.strWord, 0, parser.lenString);
                if (idx == nullptr)
                {
                    //May be the type of this new structure (but this structure has not registered so far).
                    if (std::wcsncmp(parser.strWord, structName, parser.lenString) == 0)
                        structureMemberIndex = static_cast<int>(parser.structureTable.size());
                    else
                        throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);
                }
                else
                    structureMemberIndex = *idx;
                memberVarType = VarType::Structure;
            }
            else
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);

            parser.ReadInOneWord();
        }

        //There maybe a 'BYREF'. If this member is refer to the same new structure being defined, 'BYREF' is mandatory.
        if (shouldByref)
            memberVarType.SetReference();
        else if (structureMemberIndex == parser.structureTable.size())
            throw Var(Exception::ERROR_CODE::MUST_BE_REFERENCE);

        structDef.varTypes.push_back(memberVarType);
        structDef.structMemberIndex.push_back(structureMemberIndex);
        structDef.arraySize.push_back(arraySize);

        //There must be end of line
        if (parser.wordType != WordReader::WordType::EndOfLine)
            throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);

        parser.ReadInOneWord();
    }

    //If I have a parent. I should copy all the members from my parent.
    int parentStructIndex = structDef.parentStructIndex;
    while (parentStructIndex != -1) //Until every parent of parent has included.
    {
        auto parent = parser.structureTable[parentStructIndex];
        int sizep = static_cast<int>(structDef.varTypes.size());
        for (int i = 0; i < parent.memberNames.GetSize(); i++)
        {
            wchar_t* name = parent.memberNames.ReverseFind(i);
            if (structDef.memberNames.Find(name, MODULE_IRRELEVANT, static_cast<int>(std::wcslen(name))) != nullptr)
                throw Var(Exception::ERROR_CODE::MEMBER_DEFINED_IN_PARENT);
            structDef.memberNames.Insert(name, MODULE_IRRELEVANT, sizep++);
            structDef.structMemberIndex.push_back(parent.structMemberIndex[i]);
            structDef.varTypes.push_back(parent.varTypes[i]);
            structDef.arraySize.push_back(parent.arraySize[i]);
        }
        parentStructIndex = parent.parentStructIndex;
    }

    structDef.moduleIdx = moduleIdx;
    parser.structureTable.push_back(structDef);
    parser.structureNameIndex.Insert(structName, moduleIdx, static_cast<int>(parser.structureTable.size()) - 1);
    delete[] structName;
}

void St_PAUSE(HashTable<Var>& localVarTable, Parser& parser)
{
    Parser commandLine;
    wchar_t buffer[256];
    commandLine.CopyInfoFrom(parser);
    commandLine.srcCode = buffer;

    while (true)
    {
        std::wcout << L"]";   //Command line
        std::wstring str;
        std::wcin.getline(buffer, 256);
        commandLine.lenSrc = std::wcslen(buffer);
        commandLine.pos = commandLine.posNext = 0;
        commandLine.Execute(-1, localVarTable);

        if (commandLine.keywordIdx == ST_CONT)
            break;
        if (commandLine.keywordIdx == ST_END)
            throw 0;
    }
}

void St_LIST(HashTable<Var>& localVarTable, Parser& parser)
{
    wchar_t* src = parser.modules[parser.currentModule].srcCode;
    while (*src)
    {
        std::wcout << *src;
        src++;
    }
    std::wcout << std::endl;
}

void St_NotExpected(HashTable<Var>& localVarTable, Parser& parser)
{
    // Not expected to start a statement.
    throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
}

std::array<BuildInStatementEntrance, BUILTINSTAMENET_COUNT> builtInStatements
{
    St_CONST,
    St_PRINT,
    St_INPUT,
    St_DIM,
    St_REDIM,
    St_NotExpected, //As
    St_NotExpected, //Byref
    St_IF,
    St_NotExpected, //Then
    St_ELSE,
    St_ENDIF,
    St_FOR,
    St_NotExpected, //To
    St_NotExpected, //Step
    St_ENDFOR,
    St_WHILE,
    St_ENDWHILE,
    St_LOOP,
    St_ENDLOOP,
    St_FOREACH,
    St_NotExpected, //In
    St_BREAK,
    St_NotExpected, //All
    St_SELECT,
    St_CASE,
    St_ENDSELECT,
    St_TRY,
    St_CATCH,
    St_THROW,
    St_RETRY,
    St_ENDTRY,
    St_IMPORT,
    St_NotExpected, //On
    St_NotExpected, //Extends
    St_NotExpected, //REM
    St_STRUCT, //Struct
    St_NotExpected, //Endstruct
    St_NotExpected, //Return
    St_FN, //Fn
    St_NotExpected, //Endfn
    St_NotExpected, //End
    St_NotExpected, //Global
    St_NotExpected, //Lib
    St_PAUSE,       //Pause
    St_NotExpected, //Cont
    St_LIST, //List
    St_NotExpected  //Run
};
