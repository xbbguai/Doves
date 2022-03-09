#pragma once

#include "HashTable.h"
#include "var.h"
#include <vector>
#include <stack>
#include <thread>
#include "WordReader.h"
#include "TableTypes.h"
#include "Expression.h"

class Parser : public WordReader
{
public:
    struct Module
    {
        wchar_t* srcCode;
        size_t pos;
        size_t size;
        std::string fileName;
        bool asPublic;  //If this module is public, Doves program may refer to global identifiers in this module without specifing the module name.
    };
    std::vector<Module> modules;
    int currentModule {-1};

    void SwitchToModule(int index)
    {
        if (currentModule != -1)
        {
            modules[currentModule].pos = pos;
        }
        currentModule = index;
        SetupReader(modules[index].srcCode, modules[index].pos, modules[index].size);
    }

    //Helper function to convert integer to wcs
    inline int IntToWCS(int l, wchar_t* dest, int size = 10)
    {
        int count = 0;
        do
        {
            *dest = (l % 10) + '0';
            l /= 10;
            dest++;
            count++;
        } while (l > 0 && count < size);
        *dest = 0;
        return count;
    }

    //Structure definition table.
    static HashTable<int> structureNameIndex;
    static std::vector<StructureDefinition> structureTable;

    //Function definition table.
    static HashTable<int> functionNameIndex;
    static std::vector<FunctionDefinition> functionTable;

    //Global variable table. Module names are stored here too.
    static HashTable<Var> globalVarTable;

/*    bool IsModuleName(wchar_t* strWord, int len, int moduleIdx)
    {
        Var* var = globalVarTable.Find(strWord, moduleIdx, len);
        if (var != nullptr && var->GetVarType() == VarType::InternalModuleIdx)
            return true;
        else
            return false;
    }
*/

    int IdentifyModule(int currentModule)
    {
        Var* varModule = globalVarTable.Find(strWord, currentModule, lenString);
        if (varModule != nullptr && varModule->GetVarType() == VarType::InternalModuleIdx)
        {
            //Yes, there is a module name. So, module index should be changed.
            currentModule = varModule->GetInteger();
            ReadInOneWord();
            //A dot should follow
            if (!IsKeywordOf(OP_DOT))
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            ReadInOneWord();
            //Now is the structure name
            if (wordType != WordReader::WordType::Identifier)
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
        }
        return currentModule;
    }

    //Garbage collector thread.
    void GarbageCollector();
    std::thread gcThread;
    volatile bool gcEnd { false };

    static std::atomic<int> objectCounter;
public:
    Parser(bool chn = false);
    virtual ~Parser();

    void CopyInfoFrom(Parser &copy);

    void ImportSource(std::string fileName, const wchar_t* moduleName = nullptr);
    void ImportDLL(std::string fileName, const wchar_t* moduleName = nullptr);

    Var Execute(int functionIndex, HashTable<Var>& localVarTable);

    //Becuase assignment has not specific statement here (Basic has LET, but it doesn't make sense to use LET),
    //If a statement starts with an identifier, go through ParseAssignment to do an variable assignment or call user function.
    //Returns the variable if there is a variable assigned, nullptr if only a user function is called.
    Var* ParseAssignment(Expression& expr, HashTable<Var>& localVarTable);

    Var CallDllFunction(Var& internalDllVar, wchar_t* funcName, HashTable<Var>& localVarTable, VarType returnType);
/*
    void (*errmsg)(...);
    errmsg = (void (*)(...))dlsym(dp, "Output");
    errmsg("asfsadf", 1, 2, 3);
*/
};
