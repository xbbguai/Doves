#include "parser.h"
#include "HashTable.h"
#include "Exception.h"
#include "Expression.h"
#include "BuiltInStatements.h"
#include "BuiltInFuncs.h"
#include "BuiltInStructs.h"
#include "textencoding.h"
#if defined(_WIN32) || defined(_WIN64)
#else
    #include <dlfcn.h>
#endif
#include <fstream>
#include <string>
#include <filesystem>
#include <atomic>

//Static members:
HashTable<int> Parser::structureNameIndex;
std::vector<StructureDefinition> Parser::structureTable;
HashTable<int> Parser::functionNameIndex;
std::vector<FunctionDefinition> Parser::functionTable;
HashTable<Var> Parser::globalVarTable;
std::atomic<int> Parser::objectCounter {0};

Parser::Parser(bool chn /* = false */) : WordReader(chn)
{
    if (objectCounter == 0)
    {
        //These two things are done only once for all Parser objects.
        InitializeBuiltinStructs(*this, chn);
        gcThread = std::thread(&Parser::GarbageCollector, this);
    }
    objectCounter++;
}

Parser::~Parser()
{
    objectCounter--;
    if (objectCounter == 0)
    {
        gcEnd = true;
        gcThread.join();
        for (auto srcModule : modules)
            delete[] srcModule.srcCode;
    }
}

void Parser::CopyInfoFrom(Parser& copy)
{
    currentModule = copy.currentModule;
    //Copy the modules
    for (auto item : copy.modules)
    {
        modules.push_back(item);
    }
    //WordReader members
    srcCode = copy.srcCode;
    lenSrc = copy.lenSrc;
    pos = copy.pos;
    posNext = copy.posNext;
}

void Parser::ImportSource(std::string fileName, const wchar_t* moduleName /* = nullptr */)
{
    //File starts with EF BB BF is BOM of UTF8
    //But it is ok if there is no BOM in the source code file.

    wchar_t* src;
    size_t size {0};
    std::ifstream file(fileName, std::ios::binary);
    if (file.is_open())
    {
        //determine the size of source code.
        int utf8size = 0;
        unsigned char ch;
        bool hasBOM = false;
        unsigned char bom[3] {0, 0, 0};
        if (!file.eof())
            file.read((char *)bom, 3);
        if (bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf)
            hasBOM = true;
        else if (bom[0] == 0xfe || bom[1] == 0xff)
            throw Var(Exception::ERROR_CODE::SOURCE_FORMAT_NOT_SUPPORTED);
        if (!hasBOM)
        {
            file.clear();
            file.seekg(0, std::ios::beg);
        }
        while (!file.eof())
        {
            file.read((char *)&ch, sizeof(ch));
            if (ch < 0x80)
            {
                size++;
                utf8size = 0;
            }
            else
            {
                if (utf8size == 0)
                {
                    if ((ch & 0xc0) == 0xc0)
                        utf8size = 2;
                    if ((ch & 0xe0) == 0xe0)
                        utf8size = 3;
                    if ((ch & 0xf0) == 0xf0)
                        utf8size = 4;
                    if ((ch & 0xf8) == 0xf8)
                        utf8size = 5;
                    if ((ch & 0xfc) == 0xfc)
                        utf8size = 6;
                }
                utf8size--;
                if (utf8size == 0)
                    size++;
            }
        }

        file.clear();
        file.seekg(hasBOM ? 3 : 0, std::ios::beg);
        src = new wchar_t[size];
        size_t pos = 0;
        utf8size = 0;
        while (!file.eof())
        {
            file.read((char *)&ch, sizeof(ch));
            if (ch < 0x80)
            {
                if (utf8size > 0)
                {
                    //Not utf-8 !!!!!!
                    pos++;
                }
                src[pos++] = static_cast<wchar_t>(ch);
                utf8size = 0;
            }
            else
            {
                if (utf8size == 0)
                {
                    if ((ch & 0xc0) == 0xc0)
                    {
                        utf8size = 1;
                        src[pos] = ch & 0x1f;
                    }
                    if ((ch & 0xe0) == 0xe0)
                    {
                        utf8size = 2;
                        src[pos] = ch & 0xf;
                    }
                    if ((ch & 0xf0) == 0xf0)
                    {
                        utf8size = 3;
                        src[pos] = ch & 0x7;
                    }
                    if ((ch & 0xf8) == 0xf8)
                    {
                        utf8size = 4;
                        src[pos] = ch & 0x3;
                    }
                    if ((ch & 0xfc) == 0xfc)
                    {
                        utf8size = 5;
                        src[pos] = ch & 0x1;
                    }
                }
                else
                {
                    utf8size--;
                    src[pos] <<= 6;
                    src[pos] |= ch & 0x3f;
                    if (utf8size == 0)
                        pos++;
                }
            }
        }
        file.close();
        src[pos - 1] = 0;
    }
    else
        throw Var(Exception::ERROR_CODE::CANNOT_OPEN_SOURCE_CODE);

    Module newModule { src, 0, size, {0} };
    newModule.fileName = fileName;
    modules.push_back(newModule);
    long moduleIdx = static_cast<long>(modules.size()) - 1;

    Var var(moduleIdx);    //Integer
    var.SetVarType(VarType::InternalModuleIdx);
    if (moduleName == nullptr || moduleName[0] == 0)
    {
        newModule.asPublic = true;
        //split file name and get the main file name as module name.
        std::wstring ws = string_filename_to_wchar(fileName.c_str());
        const wchar_t* p = ws.c_str();
        int dotPos = 0;
        while (p[dotPos] && p[dotPos] != L'.')
            dotPos++;
        
        if (globalVarTable.Find(p, currentModule == -1 ? 0 : currentModule, dotPos) == nullptr)
            globalVarTable.Insert(p, currentModule == -1 ? 0 : currentModule, dotPos, var);  //Import only not imported module
    }
    else
    {
        newModule.asPublic = false;
        if (globalVarTable.Find(moduleName, currentModule == -1 ? 0 : currentModule, static_cast<int>(std::wcslen(moduleName))) != nullptr)
            throw Var(Exception::ERROR_CODE::IMPORT_NAME_USED);
        globalVarTable.Insert(moduleName, currentModule == -1 ? 0 : currentModule, var);  //Import only not imported module
    }

    int currentModuleSave = currentModule;
    SwitchToModule(moduleIdx);
    //CreatePreExecuteTables(moduleIdx);
    HashTable<Var> localVarTable;

    Execute(-1, localVarTable);

    if (currentModuleSave != -1)
        SwitchToModule(currentModuleSave);
}

void Parser::ImportDLL(std::string fileName, const wchar_t* moduleName /* = nullptr */)
{
/*
#ifdef WIN32
#else   //linux or macos
    void *dp = dlopen(fileName.c_str(), RTLD_LAZY);
    if (dp == nullptr)
        throw Var(Exception::ERROR_CODE::FAIL_LOADING_LIB);
    
    Var var;
    var.SetVarType(VarType::InternalDLL);
    var.SetPointer(dp);
    if (moduleName == nullptr || moduleName[0] == 0)
    {
        //split file name and get the main file name as module name.
        std::filesystem::path pname(fileName);
        std::wstring ws = pname.filename().wstring();
        const wchar_t* p = ws.c_str();
        int dotPos = 0;
        while (p[dotPos] && p[dotPos] != L'.')
            dotPos++;
        
        if (globalVarTable.Find(p, currentModule == -1 ? 0 : currentModule, dotPos) == nullptr)
            globalVarTable.Insert(p, currentModule == -1 ? 0 : currentModule, dotPos, var);  //Import only not imported module
    }
    else
    {
        if (globalVarTable.Find(moduleName, currentModule == -1 ? 0 : currentModule, std::wcslen(moduleName)) != nullptr)
            throw Var(Exception::ERROR_CODE::IMPORT_NAME_USED);
        globalVarTable.Insert1(moduleName, currentModule == -1 ? 0 : currentModule, var);  //Import only not imported module
    }
#endif
*/
}

Var Parser::Execute(int functionIndex, HashTable<Var>& localVarTable)
{
    //Save current word reader position
    size_t posSave = pos;
    size_t posNextSave = posNext;  
    int currentModuleSave = currentModule;

    //Set the position of word reader to first line of this function.
    if (functionIndex >= 0)
    {
        if (functionTable[functionIndex].moduleIdx != currentModule)
            SwitchToModule(functionTable[functionIndex].moduleIdx);
        if (functionTable[functionIndex].internalStructFunc == nullptr)
            pos = posNext = functionTable[functionIndex].sourceCodePosition;
    }
    Var result;
    Expression expr(*this, localVarTable);

    bool circleBack;
    do
    {
        circleBack = false;
        try
        {
            if (functionIndex == -1 || functionTable[functionIndex].internalStructFunc == nullptr)
            {
                while (true)
                {
                    ReadInOneWord();

                    //Generally a statement starts with a keyword.
                    if (wordType == WordReader::WordType::Keyword && (keywordIdx & KT_STATEMENT) > 0)
                    {
                        //Call the corresponding statement parser to parse the statement.
                        //'RETURN' and 'ENDFN' should be treated seperatedly.
                        if (keywordIdx == ST_ENDFN || keywordIdx == ST_CONT || keywordIdx == ST_END)
                            break;
                        else if (keywordIdx == ST_RETURN)
                        {
                            ReadInOneWord();
                            if (IsEndOfStatement())
                                break;
                            else
                            {
                                //If byref: result.MoveFrom(expr.Calculate());
                                result.Clear();
                                result = expr.Calculate();
                                //A function should never return a reference, this should be a value
                                if (result.IsReference())
                                {
                                    Var* refVar = result.GetReferedTo();
                                    result.Clear();
                                    result = *refVar;
                                }
                                break;
                            }
                        }
                        else
                            builtInStatements[keywordIdx - KT_STATEMENT](localVarTable, *this);
                    }
                    else if (wordType == WordReader::WordType::Keyword && (keywordIdx & KT_BUILTINFUNC) > 0)
                    {
                        //If it is an built-in function call, make the call by using Expression
                        expr.Calculate();
                    }
                    else if (wordType == WordReader::WordType::Identifier || IsKeywordOf(OP_LEFTSQBRACKET))
                    {
                        ParseAssignment(expr, localVarTable);
                    }
                    else if (wordType == WordReader::WordType::EndOfFile)
                        break;
                    else if (wordType != WordReader::WordType::EndOfLine && !IsKeywordOf(OP_COLON))
                        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
                }
            }
            else
                result = (functionTable[functionIndex].internalStructFunc)(localVarTable, *this);
        }
        catch (Var& eVar)
        {
            //Get a variable thrown by THROW statement in the source code or by an internal throw.
            //In this case, I should rely throwning this variable to the upper level.
            //The upper level is the Parser::Execute function itself. 
            //So, before throwing it to the upper level, I should check if I am inside a TRY block.
            //If I'm in a TRY block, I should not throw this eVar to upper level, instead I should find the appropriate CATCH block,
            //and handle the control to this CATCH block.

            if (eVar.GetVarType() == VarType::InternalException)
            {
                //Turn it to internal structure of 'Exception'
                int errCode = eVar.GetInteger();
                std::pair<int, int> pos = LineFromPosition();
                eVar.InitStructure(structureTable, *structureNameIndex.Find(L"EXCEPTION", 0, 9));
                eVar.GetStructureMember(0).SetInteger(errCode);
                eVar.GetStructureMember(1).SetInteger(pos.first);
                eVar.GetStructureMember(2).SetInteger(pos.second);
                eVar.GetStructureMember(3).SetInteger(currentModule);
            }

            //Search the queue in localVarTable
            Var* queueVar = localVarTable.Find(L"#Q", MODULE_IRRELEVANT, 2);
            if (queueVar == nullptr)
            {
                pos = posSave;
                posNext = posNextSave;
                throw eVar; //In this level, I'm not in a TRY block. This will throw eVar to upper level.
            }

            std::vector<Var::QueueInfo> *qi = queueVar->GetInternalQueue();
            int qp = qi->size();
        retry:
            int posq = static_cast<int>(qi->size()) - 1;
            while (posq >= 0)
            {
                if (qi->at(posq).type == ST_TRY)
                    break;
                posq--;
            }
            if (posq < 0)
            {
                pos = posSave;
                posNext = posNextSave;
                throw eVar; //In this level, I'm not in a TRY block. This will throw eVar to upper level.
            }
            else if (qi->at(posq).to == 1)
            {
                if (posq == 0)
                    qi->clear();
                else
                    qi->resize(posq);
                pos = posSave;
                posNext = posNextSave;
                throw eVar; //In this level, I'm not in a TRY block. This will throw eVar to upper level.
            }
            else
            {
                //This eVar is now in my level of TRY-CATCH block.
                //I should find an appropriate CATCH block for this.
                if (!Helper_FindCatchSection(localVarTable, *this, eVar))
                {
                    if (posq == 0)
                        qi->clear();
                    else
                        qi->resize(posq);
                    goto retry;
                }
                circleBack = true;
                //When return from this point, the next statement will be the first statement inside the appropriate catch block.
                if (posq >= 0 && qi->at(posq).type == ST_TRY)
                    qi->at(posq).to = 1; //Indicating that we are in a catch block
            }
        }
        catch (int)
        {
            throw 0;
        }
        catch (...)
        {
            std::pair<int, int> pos = LineFromPosition();
            std::cout<< "Internal Error at line #" << pos.first << " column #" << pos.second << " module #" << currentModule << std::endl;
            throw 0;  //Throw to end the program
        }
    } while (circleBack);
    
    //If return type is defined, should match the return result with return type.
    if (functionIndex != -1 && functionTable[functionIndex].returnType != VarType::Null &&
        !functionTable[functionIndex].returnType.IsCompatible(result.GetVarType()))
    {
        throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    }
 
    //If there is a thread running, should wait for it.
    for (auto& item : localVarTable.anonymousList)
    {
        if (item.GetVarType() == VarType::InternalThreadObj)
        {
            std::thread* pobj = (std::thread*)item.GetPointer();
            if (pobj != nullptr && pobj->joinable())
                pobj->join();   //Wait for the thread to end
            item.DecreaseReferCount();
        }
    }

    //All referenced variables from this localVarTable, should minus 1.
    for (auto& barrel : localVarTable.barrels)
    {
        for (auto& item : barrel)
        {
            if (item.value.IsReference())
                item.value.DecreaseReferCount();   //It will safely decrease the ref count of the variable being referenced to by item.value
        }
    }

    //Restore word reader position
    if (currentModule != currentModuleSave)
        SwitchToModule(currentModuleSave);
    pos = posSave;
    posNext = posNextSave;
    return result;
}

//Becuase assignment has not specific statement here (Basic has LET, but it doesn't make sense to use LET),
//If a statement starts with an identifier, go through ParseAssignment to do an variable assignment or call user function.
//Returns the variable if there is a variable assigned, returns a Null var if only a user function is called.
Var* Parser::ParseAssignment(Expression& expr, HashTable<Var>& localVarTable)
{
    //Left side of '='
    Var* left = expr.GetLeftValue();    //When it returns, wordReader will read in the next word.
    if (left == nullptr)
        return nullptr;   //Not a left value. But might have called a function or method, which means there cannot be assignment or reference here.

    if (left->IsConstant())
        throw Var(Exception::ERROR_CODE::CONSTANT_NOT_MUTABLE);
    //Check '='(assignment), '@='(refer to)
    if (IsKeywordOf(OP_EQUAL))
    {
        //Calculate the right side of '=' and assign the value to the left side.
        ReadInOneWord();
        if (left->IsReference() && !left->HasReferedTo())
            throw Var(Exception::ERROR_CODE::USING_NOT_INITIALIZED_REF);
        else
        {
            Var&& right = expr.Calculate();
            if (left->GetVarType() == VarType::Null || left->GetVarType() == right.GetVarType() ||
                left->GetVarType().IsNumeric() && right.GetVarType().IsNumeric() ||
                left->GetVarType().IsStructure() && right.GetVarType().IsRedimable())
            {
                *left = right;
                //If the 'left' is a reference, and the destination is vartype::null before new value being assigned,
                //this var type of left value should also get changed.
                if (left->GetVarType() == VarType::Null && left->IsReference()) //it had refered to a null.
                {
                    left->SetVarType(right.GetVarType() | VarType::ReferenceFlag);
                }
            }
            else if (left->IsReference() && right.GetVarType() == VarType::Null)
            {
                left->Clear();
            }
            else
                throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
        }
    }
    else if (IsKeywordOf(OP_REFERTO))
    {
        ReadInOneWord();
        if (left->IsReference())
        {
            Var&& calcResult = expr.Calculate();
            //To refer to another variable, types should match
            if (left->GetVarType() == VarType::Null || left->GetVarType() == calcResult.GetVarType())
            {
                if (calcResult.IsReference())
                {
//                    if (left->HasReferedTo())
//                        left->DecreaseReferCount(); //This will decrease the reference count of the variable being refered by 'left', if any.
                    left->ReferTo(&calcResult, *this);
//                    calcResult.IncreaseReferCount();    //Can also call left->IncreaseReferCount().
                }
                else
                    throw Var(Exception::ERROR_CODE::RVALUE_NOT_REFERENCABLE);
            }
            else if (left->IsReference() && calcResult.GetVarType() == VarType::Null)
            {
//                if (left->HasReferedTo())
//                    left->DecreaseReferCount();
                left->ClearRefer();
            }
            else
                throw Var(Exception::ERROR_CODE::CANNOT_REFER_TO_DIFFERENT_TYPE);
        }
        else
            throw Var(Exception::ERROR_CODE::NOT_A_REF_VAR);
    }
    else if (!IsEndOfStatement())
        throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
    return left;
}

Var Parser::CallDllFunction(Var& internalDllVar, wchar_t* funcName, HashTable<Var>& localVarTable, VarType returnType)
{
    Var result;
    result.SetVarType(returnType);

    void* dp = (void*)internalDllVar.GetPointer();
#if defined(_WIN32) || defined(_WIN64)
#else
    asm volatile (
        "nop"
    );
#endif
    return result;
}

void Parser::GarbageCollector()
{
    //Collects useless vars every 2.5 seconds
    do
    {
        for (int i = 0; i < 25 && !gcEnd; i++)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::unique_lock lock(anonymousListMtx);
        for (auto& item : globalVarTable.anonymousList)
        {
            if (item.GetReferenceCount() == 0)
            {
                Var var;
                var.MoveFrom(std::move(item));
                //so that it will be destroied and memory reclaimed after the '}'.
            }
        }

        //Remove the objects from anonymous list.
        globalVarTable.anonymousList.remove_if([](Var& var) 
            {
                return var.GetReferenceCount() == 0;
            } 
        );
        lock.unlock();
    } while (!gcEnd);
}