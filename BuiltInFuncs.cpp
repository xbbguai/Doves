#include "var.h"
#include "Exception.h"
#include "HashTable.h"
#include "WordReader.h"
#include "BuiltInFuncs.h"
#include "parser.h"
#include <array>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

// ASC(CHARACTOR AS CHAR) AS INTEGER
// Get the ASCII / Unicode of the char
Var Func_ASC(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"CHARACTOR", MODULE_IRRELEVANT, 9);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    Var result;
    result.SetInteger(param0->GetChar());
    return result;
}

// VAL(STR AS String) AS REAL
// Convert the string to real
Var Func_VAL(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    Var result;
    result.SetReal(std::wcstod(param0->GetString(), nullptr));
    return result;
}

// CHR(asc AS INTEGER) AS STRING
// Convert the asc to String
Var Func_CHR(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"ASC", MODULE_IRRELEVANT, 3);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    Var result;
    wchar_t str[2];
    str[1] = 0;
    str[0] = param0->GetChar();
    result.SetString(str);
    return result;
}

// LEN(STR AS String) AS INTEGER
// Return length of a string
Var Func_LEN(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    Var result;
    result.SetInteger(static_cast<long>(std::wcslen(param0->GetString())));
    return result;
}

Var Func_TOUPPER(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    wchar_t* string = param0->GetString();
    long len = param0->GetDataSize();    
    wchar_t* newString = new wchar_t[len + 1];
    newString[len--] = 0;
    while (len >= 0)
    {
        newString[len] = string[len] >= L'a' && string[len] <= L'z' ? string[len] - L'a' + L'A' : string[len];
        len--;
    }
    Var result;
    result.EmplaceString(newString);
    return result;
}

Var Func_TOLOWER(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    wchar_t* string = param0->GetString();
    long len = param0->GetDataSize();    
    wchar_t* newString = new wchar_t[len + 1];
    newString[len--] = 0;
    while (len >= 0)
    {
        newString[len] = string[len] >= L'A' && string[len] <= L'Z' ? string[len] - L'A' + L'a' : string[len];
        len--;
    }
    Var result;
    result.EmplaceString(newString);
    return result;
}

Var Func_LEFT(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    auto param1 = localVarTable.Find(L"01", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param1 == nullptr)
        param1 = localVarTable.Find(L"LENGTH", MODULE_IRRELEVANT, 6);
    if (param0 == nullptr || param1 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    
    wchar_t* string = param0->GetString();
    long len = std::min(param1->GetInteger(), param0->GetDataSize());

    if (len < 0)
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    
    wchar_t* newString = new wchar_t[len + 1];
    newString[len--] = 0;
    while (len >= 0)
    {
        newString[len] = string[len];
        len--;
    }
    Var result;
    result.EmplaceString(newString);
    return result;
}

Var Func_RIGHT(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    auto param1 = localVarTable.Find(L"01", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param1 == nullptr)
        param1 = localVarTable.Find(L"LENGTH", MODULE_IRRELEVANT, 6);
    if (param0 == nullptr || param1 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    
    wchar_t* string = param0->GetString();
    long len = std::min(param1->GetInteger(), param0->GetDataSize());
    long start = param0->GetDataSize() - len;

    if (len < 0)
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    
    wchar_t* newString = new wchar_t[len + 1];
    for (int i = 0; i < len; i++)
    {
        newString[i] = string[start++];
    }
    newString[len] = 0;
    Var result;
    result.EmplaceString(newString);
    return result;
}

Var Func_MID(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    auto param1 = localVarTable.Find(L"01", MODULE_IRRELEVANT, 2);
    auto param2 = localVarTable.Find(L"02", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param1 == nullptr)
        param1 = localVarTable.Find(L"START", MODULE_IRRELEVANT, 5);
    if (param2 == nullptr)
        param2 = localVarTable.Find(L"LENGTH", MODULE_IRRELEVANT, 6);
    if (param0 == nullptr || param1 == nullptr || param2 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    
    wchar_t* string = param0->GetString();
    long len = param0->GetDataSize();
    long start = param1->GetInteger();
    long length = param2->GetInteger();
    start = std::min(len, start);
    length = std::min(start + length, len) - start;

    if (start < 0 || length < 0)
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    
    wchar_t* newString = new wchar_t[length + 1];
    for (int i = 0; i < length; i++)
    {
        newString[i] = string[start++];
    }
    newString[length] = 0;
    Var result;
    result.EmplaceString(newString);
    return result;
}

Var Func_FIND(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    auto param1 = localVarTable.Find(L"01", MODULE_IRRELEVANT, 2);
    auto param2 = localVarTable.Find(L"02", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"STR", MODULE_IRRELEVANT, 3);
    if (param1 == nullptr)
        param1 = localVarTable.Find(L"SUB", MODULE_IRRELEVANT, 3);
    if (param2 == nullptr)
        param2 = localVarTable.Find(L"FROM", MODULE_IRRELEVANT, 4);
    if (param0 == nullptr || param1 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    
    long from = 0;
    
    if (param2 != nullptr)
        from = param2->GetInteger();
    
    wchar_t* string = param0->GetString() + from;
    wchar_t* sub = param1->GetString();

    if (from < 0 || from >= param0->GetDataSize())
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);

    bool found = false;
    while (*string)
    {
        wchar_t* bp = string;
        wchar_t* sp = sub;
        do
        {
            if (!*sp)
            {
                found = true;
                break;
            }
        } while (*bp++ == *sp++);
        if (found)
            break;
        string += 1;
    }

    long result = -1;
    if (*string != 0)
    {
        result = static_cast<long>(string - param0->GetString());
    }
    return Var(result);
}

Var Func_REPLACE(HashTable<Var>& localVarTable, Parser& parser)
{
    return Var();
}

Var Func_MAX(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    auto param1 = localVarTable.Find(L"01", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param1 == nullptr)
        param1 = localVarTable.Find(L"B", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr || param1 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    double&& p0 = param0->GetReal();
    double&& p1 = param1->GetReal();

    return Var(p0 > p1 ? p0 : p1);
}

Var Func_MIN(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    auto param1 = localVarTable.Find(L"01", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param1 == nullptr)
        param1 = localVarTable.Find(L"B", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr || param1 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);

    double&& p0 = param0->GetReal();
    double&& p1 = param1->GetReal();

    return Var(p0 > p1 ? p1 : p0);
}

Var Func_SIN(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(sin(p0));
}

Var Func_COS(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(cos(p0));
}

Var Func_TAN(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(tan(p0));
}

Var Func_ASIN(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(asin(p0));
}

Var Func_ACOS(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(acos(p0));
}

Var Func_ATAN(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(atan(p0));
}

Var Func_LOG(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(log(p0));
}

Var Func_LOG10(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(log10(p0));
}

Var Func_SQRT(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(sqrt(p0));
}

Var Func_CEILING(HashTable<Var>& localVarTable, Parser& parser)
{
   auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(ceil(p0));    
}

Var Func_FLOOR(HashTable<Var>& localVarTable, Parser& parser)
{
   auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(floor(p0));    
}

Var Func_ROUND(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    double&& p0 = param0->GetReal();

    return Var(round(p0));    
}

Var Func_RAND(HashTable<Var>& localVarTable, Parser& parser)
{
    return Var(static_cast<double>((double)rand()) / RAND_MAX);
}

Var Func_SEED(HashTable<Var>& localVarTable, Parser& parser)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    rand();
    rand();
    rand();
    return Var();
}

Var Func_SETW(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"WIDTH", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    std::wcout << std::setw(param0->GetInteger());
    return Var();
}

Var Func_SETPRECISION(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"PRECISION", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    std::wcout << std::setprecision(param0->GetInteger());
    return Var();
}

Var Func_TAB(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"TAB", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    int pos = param0->GetInteger();
#if defined(_WIN32) || defined(_WIN64)
    if (pos < 0)
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    std::wstring ws;
    for (int i = 0; i < pos; i++)
        ws += L" ";
    std::wcout << ws;
#else
    if (pos > 0)
        std::wcout << L"\033[" << pos << L"C";
    else
        std::wcout << L"\033[" << -pos << L"D";
#endif
    return Var();
}

static auto tmOfSystemStart = std::chrono::system_clock::now();
Var Func_TICKS(HashTable<Var>& localVarTable, Parser& parser)
{
    auto tm = std::chrono::system_clock::now() - tmOfSystemStart;
    auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(tm).count();
    return Var(static_cast<long>(ticks));
}

Var Func_TYPENAME(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    switch (param0->GetVarType())
    {
    case VarType::Boolean:
        return Var(L"Boolean");
    case VarType::Array:
        return Var(L"Array");
    case VarType::Char:
        return Var(L"Char");
    case VarType::Null:
        return Var(L"Null");
    case VarType::Integer:
        return Var(L"Integer");
    case VarType::Memory:
        return Var(L"Memory");
    case VarType::Real:
        return Var(L"Real");
    case VarType::String:
        return Var(L"String");
    case VarType::Structure:
        {
        Var sdIdx = param0->GetStructureMember(-1);
        return Var(parser.structureNameIndex.ReverseFind(sdIdx.GetInteger()));
        }
    }
    return Var(L"");
}

Var Func_ISNUMERIC(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetVarType().IsNumeric())
        return Var(true);
    else
        return Var(false);
}

Var Func_ISSTRING(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetVarType() == VarType::String)
        return Var(true);
    else
        return Var(false);
}

Var Func_ISBOOLEAN(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetVarType() == VarType::Boolean)
        return Var(true);
    else
        return Var(false);
}

Var Func_ISARRAY(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetVarType() == VarType::Array)
        return Var(true);
    else
        return Var(false);
}

Var Func_ISNULL(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetVarType() == VarType::Null)
        return Var(true);
    else
        return Var(false);
}

Var Func_ISSTRUCTUREOF(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    auto param1 = localVarTable.Find(L"01", MODULE_IRRELEVANT, 2);
    if (param1 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetVarType() == VarType::Structure)
    {
        Var sdIdx = param0->GetStructureMember(-1);
        wchar_t* p1 = param1->GetString();
        while (*p1)
        {
            if (*p1 >= L'a' && *p1 <= L'z')
                *p1 = *p1 - L'a' + L'A';
            p1++;
        }
        bool br = std::wcscmp(parser.structureNameIndex.ReverseFind(sdIdx.GetInteger()), param1->GetString()) == 0;
        return Var(br);
    }
    else
        return Var(false);
}

Var Func_SLEEP(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"ms", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetInteger() < 0)
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);

    std::this_thread::sleep_for(std::chrono::milliseconds(param0->GetInteger()));
    return Var();
}

Var Func_GETREFCOUNT(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    
    return Var(param0->GetReferenceCount());
}

Var Func_ARRAYSIZE(HashTable<Var>& localVarTable, Parser& parser)
{
    auto param0 = localVarTable.Find(L"00", MODULE_IRRELEVANT, 2);
    if (param0 == nullptr)
        param0 = localVarTable.Find(L"A", MODULE_IRRELEVANT, 1);
    if (param0 == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (param0->GetVarType() != VarType::Array && param0->GetVarType() != VarType::Memory)
        throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    return Var(param0->GetDataSize());
}

std::array<BuildInFunctionEntrance, BUILTINFUNC_COUNT> builtInFunctions 
{
    Func_ASC, 
    Func_CHR,
    Func_VAL,
    Func_LEN,
    Func_TOUPPER,
    Func_TOLOWER,
    Func_LEFT,
    Func_RIGHT,
    Func_MID,
    Func_FIND,
    Func_REPLACE,
    Func_MAX,
    Func_MIN,
    Func_SIN,
    Func_COS,
    Func_TAN,
    Func_ASIN,
    Func_ACOS,
    Func_ATAN,
    Func_LOG,
    Func_LOG10,
    Func_SQRT,
    Func_CEILING,
    Func_FLOOR,
    Func_ROUND,
    Func_RAND,
    Func_SEED,
    Func_SETW,
    Func_SETPRECISION,
    Func_TAB,
    Func_TICKS,
    Func_TYPENAME,
    Func_ISNUMERIC,
    Func_ISSTRING,
    Func_ISARRAY,
    Func_ISBOOLEAN,
    Func_ISNULL,
    Func_ISSTRUCTUREOF,
    Func_SLEEP,
    Func_GETREFCOUNT,
    Func_ARRAYSIZE
};
