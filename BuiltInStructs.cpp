#include "TableTypes.h"
#include "BuiltInStructs.h"
#include "parser.h"
#include "textencoding.h"
#include <ctime>

#define max(a,b) (a > b ? a : b)

//From BuiltInStructs_Thread.cpp
void BuildStruct_Thread(Parser& parser, bool chn);
void BuildStruct_Mutex(Parser& parser, bool chn);

//Helpers
void BuildStructMember(StructureDefinition& sd, const wchar_t* name, VarType varType, int arraySize /*= 0*/, int structIdx /*= -1*/)
{
    sd.memberNames.Insert(name, MODULE_IRRELEVANT, static_cast<int>(sd.varTypes.size()));
    sd.varTypes.push_back(varType);
    sd.arraySize.push_back(arraySize);                //-1 = Not an array
    sd.structMemberIndex.push_back(structIdx);        //-1 = Not a structure
}

void BuildFuncParam(FunctionDefinition& fd, const wchar_t* name, VarType paramType, int structParamIndex /*= -1*/)
{
    fd.paramIndexes.Insert(name, MODULE_IRRELEVANT, static_cast<int>(fd.paramTypes.size()));
    fd.paramTypes.push_back(paramType);
    fd.structParamIndex.push_back(structParamIndex);
}
//End helpers
//--------------------------------------

//Internal structure of EXCEPTION
//Structure body
static inline void BuildStruct_Exception(Parser& parser, bool chn)
{
    StructureDefinition structException;
    structException.moduleIdx = 0;
    structException.parentStructIndex = -1;
    //Field members
    BuildStructMember(structException, L"errCode", VarType::Integer);
    BuildStructMember(structException, L"line", VarType::Integer);
    BuildStructMember(structException, L"column", VarType::Integer);
    BuildStructMember(structException, L"moduleIdx", VarType::Integer);
    //Add structure to parser
    parser.structureNameIndex.Insert(L"Exception", 0, static_cast<int>(parser.structureTable.size()));
    parser.structureTable.push_back(structException);
}
//-------------------------------------------------------------------------------------

//Internal structure of DATETIME
//Member function(s)
Var DateTime_GetNow(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.

    time_t now = time(0);
    tm *ltm = localtime(&now);

    self->GetStructureMember(memberPos - 6).SetInteger(1900 + ltm->tm_year);
    self->GetStructureMember(memberPos - 5).SetInteger(ltm->tm_mon + 1);
    self->GetStructureMember(memberPos - 4).SetInteger(ltm->tm_mday);
    self->GetStructureMember(memberPos - 3).SetInteger(ltm->tm_hour);
    self->GetStructureMember(memberPos - 2).SetInteger(ltm->tm_min);
    self->GetStructureMember(memberPos - 1).SetInteger(ltm->tm_sec);

    return Var();
}

//Structure body
static inline void BuildStruct_DateTime(Parser& parser, bool chn)
{
    StructureDefinition dateTime;
    dateTime.moduleIdx = 0;
    dateTime.parentStructIndex = -1;
    //Field members
    BuildStructMember(dateTime, L"year", VarType::Integer);
    BuildStructMember(dateTime, L"month", VarType::Integer);
    BuildStructMember(dateTime, L"day", VarType::Integer);
    BuildStructMember(dateTime, L"hour", VarType::Integer);
    BuildStructMember(dateTime, L"minute", VarType::Integer);
    BuildStructMember(dateTime, L"second", VarType::Integer);
    //Add structure to parser
    parser.structureNameIndex.Insert(L"Datetime", 0, static_cast<int>(parser.structureTable.size()));
    parser.structureTable.push_back(dateTime);
    //Define member functions
    FunctionDefinition dtGetDateTime;
    dtGetDateTime.internalStructFunc = DateTime_GetNow;
    dtGetDateTime.moduleIdx = 0;
    dtGetDateTime.returnStructIndex = -1;   //No return
    dtGetDateTime.returnType = VarType::Null;
    dtGetDateTime.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(dtGetDateTime, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);

    parser.functionNameIndex.Insert(L"GetNow#1", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(dtGetDateTime);
}
//-------------------------------------------------------------------------------------

//Internal structure of File
//Member function(s)
//open(binary as boolean, write as boolean, append as boolean) as boolean
Var File_Open(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    Var& name = self->GetStructureMember(memberPos - 1);
    Var* read = localVarTable.Find(L"read", MODULE_IRRELEVANT, 4);
    Var* write = localVarTable.Find(L"write", MODULE_IRRELEVANT, 5);
    Var* binary = localVarTable.Find(L"binary", MODULE_IRRELEVANT, 6);
    Var* append = localVarTable.Find(L"append", MODULE_IRRELEVANT, 6);

    wchar_t* pname = name.GetString();
    std::string fileName;
    std::string rw;
    fileName = wchar_filename_to_string(pname);
    if (write == nullptr || write->GetBool() == false)
        rw += "r";
    else
        rw += "w";
    if (binary != nullptr && binary->GetBool() == true)
        rw += "b";
    if (append != nullptr && append->GetBool() == true)
        rw += "a";
    if (read != nullptr && read->GetBool() == true && write != nullptr && write->GetBool() == true)
        rw += "+";
    FILE* f = std::fopen(fileName.c_str(), rw.c_str());
    handle.SetPointer((void *)f);
    return Var(f != nullptr);
}

Var File_Close(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());
    int result = std::fclose(f);
    return Var(result == 0);
}

Var File_Read(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* readTo = localVarTable.Find(L"readTo", MODULE_IRRELEVANT, 6);
    if (readTo == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());
    long l;
    double r;
    std::string str;
    char strBuffer[1024];
    wchar_t paramIdxStr[3] {0,0,0};
    int paramIdx = 2;
    do
    {
        switch (readTo->GetVarType())
        {
        case VarType::Boolean:
            if (fscanf(f, "%ld", &l) == 0)
                return Var(false);
            else
                readTo->SetBool(l != 0);
            break;
        case VarType::Integer:
            if (fscanf(f, "%ld", &l) == 0)
                return Var(false);
            else
                readTo->SetInteger(l);
            break;
        case VarType::Real:
            if (fscanf(f, "%lf", &r) == 0)
                return Var(false);
            else
                readTo->SetReal(r);
            break;
        case VarType::String:
            l = fscanf(f, "%1023s", strBuffer);
            if (l > 0)
                readTo->SetString(utf8_to_wchar(strBuffer).c_str());
            else
            {
                readTo->SetString(L"");
                return Var(false);
            }
            break;
        default:
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
        }
        paramIdxStr[1] = (paramIdx % 10) + L'0';
        paramIdxStr[0] = (paramIdx / 10) + L'0';
        readTo = localVarTable.Find(paramIdxStr, MODULE_IRRELEVANT, 2);
        paramIdx++;
    } while (readTo != nullptr && paramIdx < 100);
    return Var(true);
}

Var File_ReadLn(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* readTo = localVarTable.Find(L"readTo", MODULE_IRRELEVANT, 6);
    if (readTo == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (readTo->GetVarType() != VarType::String && readTo->GetVarType() != VarType::Null)
        throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());

    std::string str;
    char strBuffer[32];
    wchar_t paramIdxStr[3] {0,0,0};
    int paramIdx = 2;
    do
    {
        str.clear();
        while (fgets(strBuffer, 32, f) != nullptr)
        {
            str += strBuffer;
            bool breakAll = false;
            for (int i = 0; i < 32; i++)
            {
                if (strBuffer[i] == '\n')
                {
                    breakAll = true;
                    break;
                }
                if (strBuffer[i] == 0)
                    break;
            }
            if (breakAll)
                break;
        }
        readTo->SetString(utf8_to_wchar(str.c_str()).c_str());
        paramIdxStr[1] = (paramIdx % 10) + L'0';
        paramIdxStr[0] = (paramIdx / 10) + L'0';
        readTo = localVarTable.Find(paramIdxStr, MODULE_IRRELEVANT, 2);
        paramIdx++;
    } while (readTo != nullptr && paramIdx < 100);
    return Var(str.size() > 0);
}

Var File_GetChar(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());
    long r = fgetc(f);
    return Var(r);
}

Var File_Write(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* toWrite = localVarTable.Find(L"toWrite", MODULE_IRRELEVANT, 7);
    if (toWrite == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());
    int bytes = 0;
    wchar_t paramIdxStr[3] {0,0,0};
    int paramIdx = 2;
    do
    {
        switch (toWrite->GetVarType())
        {
        case VarType::Boolean:
            bytes = fprintf(f, "%ld ", toWrite->GetInteger());
            break;
        case VarType::Integer:
            bytes = fprintf(f, "%ld ", toWrite->GetInteger());
            break;
        case VarType::Real:
            bytes = fprintf(f, "%lf ", toWrite->GetReal());
            break;
        case VarType::String:
            bytes = fprintf(f, "%s ", wchar_to_utf8(toWrite->GetString()).c_str());
            break;
        default:
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
        }
        paramIdxStr[1] = (paramIdx % 10) + L'0';
        paramIdxStr[0] = (paramIdx / 10) + L'0';
        toWrite = localVarTable.Find(paramIdxStr, MODULE_IRRELEVANT, 2);
        paramIdx++;
    } while (toWrite != nullptr && paramIdx < 100);
    return Var(bytes != 0);
}

Var File_WriteLn(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* toWrite = localVarTable.Find(L"toWrite", MODULE_IRRELEVANT, 7);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);

    FILE* f = (FILE *)(handle.GetPointer());
    int bytes = 0;
    wchar_t paramIdxStr[3] {0,0,0};
    int paramIdx = 2;
    do
    {
        if (toWrite != nullptr)
        {
            switch (toWrite->GetVarType())
            {
            case VarType::Boolean:
                bytes = fprintf(f, "%ld\n", toWrite->GetInteger());
                break;
            case VarType::Integer:
                bytes = fprintf(f, "%ld\n", toWrite->GetInteger());
                break;
            case VarType::Real:
                bytes = fprintf(f, "%lf\n", toWrite->GetReal());
                break;
            case VarType::String:
                bytes = fprintf(f, "%s\n", wchar_to_utf8(toWrite->GetString()).c_str());
                break;
            default:
                throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
            }
        }
        else
            bytes = fprintf(f, "\n");
        paramIdxStr[1] = (paramIdx % 10) + L'0';
        paramIdxStr[0] = (paramIdx / 10) + L'0';
        toWrite = localVarTable.Find(paramIdxStr, MODULE_IRRELEVANT, 2);
        paramIdx++;
    } while (toWrite != nullptr && paramIdx < 100);
    return Var(bytes > 0);
}

Var File_Seek(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* pos = localVarTable.Find(L"pos", MODULE_IRRELEVANT, 3);
    Var* from = localVarTable.Find(L"from", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());   
    int res = fseek(f, pos == nullptr ? 0 : pos->GetInteger(), from == nullptr ? 0 : from->GetInteger());
    return Var(res == 0);
}

Var File_Tell(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());   
    long res = ftell(f);
    return Var(res);
}

Var File_ReadBin(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* readTo = localVarTable.Find(L"readTo", MODULE_IRRELEVANT, 6);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vsize = localVarTable.Find(L"size", MODULE_IRRELEVANT, 4);
    if (readTo == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (readTo->GetVarType() != VarType::Structure || !readTo->IsReference() || readTo->GetStructureMember(-1).GetInteger() != 3)
        throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    long size = vsize->GetInteger();
    long index = 0;
    if (vidx != nullptr)
        index = vidx->GetInteger();
    Var& memory = readTo->GetStructureMember(0);
    if (size <= 0 || index < 0 || size + index > memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
    
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());
    size_t r = fread(memory.GetMemory() + index, 1, size, f);

    return Var(static_cast<long>(r));
}

Var File_WriteBin(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* readTo = localVarTable.Find(L"toWrite", MODULE_IRRELEVANT, 7);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vsize = localVarTable.Find(L"size", MODULE_IRRELEVANT, 4);
    if (readTo == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    if (readTo->GetVarType() != VarType::Structure || !readTo->IsReference() || readTo->GetStructureMember(-1).GetInteger() != 3)
        throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    long size = vsize->GetInteger();
    long index = 0;
    if (vidx != nullptr)
        index = vidx->GetInteger();
    Var& memory = readTo->GetStructureMember(0);
    if (size <= 0 || index < 0 || size + index > memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
    
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& handle = self->GetStructureMember(memberPos - 2);
    FILE* f = (FILE *)(handle.GetPointer());
    size_t r = fwrite(memory.GetMemory() + index, 1, size, f);

    return Var(static_cast<long>(r));
}

//Structure body
static inline void BuildStruct_File(Parser& parser, bool chn)
{
    StructureDefinition file;
    file.moduleIdx = 0;
    file.parentStructIndex = -1;
    //Field members
    BuildStructMember(file, L"#handle", VarType::Null);  //Named as #handle to avoid being visited.
    BuildStructMember(file, L"name", VarType::String);
    //Add structure to parser
    parser.structureNameIndex.Insert(L"File", 0, static_cast<int>(parser.structureTable.size()));
    parser.structureTable.push_back(file);
    //Define member functions
    //  open
    FunctionDefinition fileOpen;
    fileOpen.internalStructFunc = File_Open;
    fileOpen.moduleIdx = 0;
    fileOpen.returnStructIndex = -1;   //Not returning a structure
    fileOpen.returnType = VarType::Boolean;
    fileOpen.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileOpen, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileOpen, L"read", VarType::Boolean);
    BuildFuncParam(fileOpen, L"write", VarType::Boolean);
    BuildFuncParam(fileOpen, L"binary", VarType::Boolean);
    BuildFuncParam(fileOpen, L"append", VarType::Boolean);
    parser.functionNameIndex.Insert(L"open#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileOpen);
    //  close
    FunctionDefinition fileClose;
    fileClose.internalStructFunc = File_Close;
    fileClose.moduleIdx = 0;
    fileClose.returnStructIndex = -1;   //Not returning a structure
    fileClose.returnType = VarType::Boolean;
    fileClose.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileClose, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"close#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileClose);
    //  read
    FunctionDefinition fileRead;
    fileRead.internalStructFunc = File_Read;
    fileRead.moduleIdx = 0;
    fileRead.returnStructIndex = -1;   //Not returning a structure
    fileRead.returnType = VarType::Boolean;
    fileRead.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileRead, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileRead, L"readTo", VarType::Null | VarType::ReferenceFlag);
    parser.functionNameIndex.Insert(L"read#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileRead);
    //  readln
    FunctionDefinition fileReadLn;
    fileReadLn.internalStructFunc = File_ReadLn;
    fileReadLn.moduleIdx = 0;
    fileReadLn.returnStructIndex = -1;   //Not returning a structure
    fileReadLn.returnType = VarType::Boolean;
    fileReadLn.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileReadLn, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileReadLn, L"readTo", VarType::Null | VarType::ReferenceFlag);
    parser.functionNameIndex.Insert(L"readln#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileReadLn);
    //  getchar
    FunctionDefinition fileGetChar;
    fileGetChar.internalStructFunc = File_GetChar;
    fileGetChar.moduleIdx = 0;
    fileGetChar.returnStructIndex = -1;   //Not returning a structure
    fileGetChar.returnType = VarType::Integer;
    fileGetChar.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileGetChar, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"getchar#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileGetChar);
    // write
    FunctionDefinition fileWrite;
    fileWrite.internalStructFunc = File_Write;
    fileWrite.moduleIdx = 0;
    fileWrite.returnStructIndex = -1;   //Not returning a structure
    fileWrite.returnType = VarType::Boolean;
    fileWrite.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileWrite, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileWrite, L"toWrite", VarType::Null);
    parser.functionNameIndex.Insert(L"write#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileWrite);
    // writeln
    FunctionDefinition fileWriteLn;
    fileWriteLn.internalStructFunc = File_WriteLn;
    fileWriteLn.moduleIdx = 0;
    fileWriteLn.returnStructIndex = -1;   //Not returning a structure
    fileWriteLn.returnType = VarType::Boolean;
    fileWriteLn.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileWriteLn, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileWriteLn, L"toWrite", VarType::Null);
    parser.functionNameIndex.Insert(L"writeLn#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileWriteLn);
    // seek
    FunctionDefinition fileSeek;
    fileSeek.internalStructFunc = File_Seek;
    fileSeek.moduleIdx = 0;
    fileSeek.returnStructIndex = -1;   //Not returning a structure
    fileSeek.returnType = VarType::Boolean;
    fileSeek.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileSeek, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileSeek, L"pos", VarType::Null);
    BuildFuncParam(fileSeek, L"from", VarType::Null);
    parser.functionNameIndex.Insert(L"seek#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileSeek);
    // tell
    FunctionDefinition fileTell;
    fileTell.internalStructFunc = File_Tell;
    fileTell.moduleIdx = 0;
    fileTell.returnStructIndex = -1;   //Not returning a structure
    fileTell.returnType = VarType::Boolean;
    fileTell.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileTell, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"tell#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileTell);

    // readbin
    FunctionDefinition fileReadBin;
    fileReadBin.internalStructFunc = File_ReadBin;
    fileReadBin.moduleIdx = 0;
    fileReadBin.returnStructIndex = -1;   //Not returning a structure
    fileReadBin.returnType = VarType::Integer;
    fileReadBin.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileReadBin, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileReadBin, L"readTo", VarType::Structure | VarType::ReferenceFlag, 3);
    BuildFuncParam(fileReadBin, L"index", VarType::Integer);
    BuildFuncParam(fileReadBin, L"size", VarType::Integer);
    parser.functionNameIndex.Insert(L"readbin#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileReadBin);
    // writebin
    FunctionDefinition fileWriteBin;
    fileWriteBin.internalStructFunc = File_WriteBin;
    fileWriteBin.moduleIdx = 0;
    fileWriteBin.returnStructIndex = -1;   //Not returning a structure
    fileWriteBin.returnType = VarType::Integer;
    fileWriteBin.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(fileWriteBin, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(fileWriteBin, L"toWrite", VarType::Structure | VarType::ReferenceFlag, 3);
    BuildFuncParam(fileWriteBin, L"index", VarType::Integer);
    BuildFuncParam(fileWriteBin, L"size", VarType::Integer);
    parser.functionNameIndex.Insert(L"writebin#2", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(fileWriteBin);
}
//-------------------------------------------------------------------------------------

//Internal structure of Binary
//Member functions
Var Binary_Create(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vSize = localVarTable.Find(L"size", MODULE_IRRELEVANT, 4);
    if (vSize == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    long size = vSize->GetInteger();
    if (size <= 0)
        throw Var(Exception::ERROR_CODE::ARRAY_SIZE_REQUIRED);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    align.SetInteger(1);
    memory.InitMemory(size);
    return Var(true);
}

Var Binary_GetSize(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    return Var(memory.GetDataSize());
}

Var Binary_SetAlign(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vAlignSize = localVarTable.Find(L"alignsize", MODULE_IRRELEVANT, 9);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    long alignSize = vAlignSize->GetInteger();
    if (alignSize < 1 || alignSize > sizeof(long) || alignSize % 2 != 0 && alignSize != 1)
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    align.SetInteger(alignSize);

    return Var(true);
}

Var Binary_GetAt(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    if (vidx == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    long idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

    return Var(static_cast<long>(memory.GetMemoryAt(idx)));
}

Var Binary_SetAt(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vbyte = localVarTable.Find(L"byte", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    if (vidx == nullptr || vbyte == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    long idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
    memory.GetMemoryAt(idx) = static_cast<uint8_t>(vbyte->GetInteger());
    return Var(true);
}

Var Binary_PutInteger(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vdata = localVarTable.Find(L"data", MODULE_IRRELEVANT, 4);
    Var* vtype = localVarTable.Find(L"type", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    if (vidx == nullptr || vdata == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    int idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

    wchar_t* type = nullptr;
    if (vtype != nullptr)
    {
        type = vtype->GetString();
        //convert to lower case
        while (*type)
        {
            if (*type >= L'A' && *type <= L'Z')
                *type = *type - L'A' + L'a';
            type++;
        }
        type = vtype->GetString();
    }
    long data = vdata->GetInteger();
    uint8_t *p = (uint8_t*)&data;
    int i = 0;
    if (type == nullptr || wcscmp(type, L"long") == 0)
    {
        for (i = 0; i < sizeof(long); i++)
        {
            memory.GetMemoryAt(idx) = p[i];
            idx++;
            if (idx >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        }
    }
    else if (wcscmp(type, L"int") == 0)
    {
        for (i = 0; i < sizeof(int); i++)
        {
            memory.GetMemoryAt(idx) = p[i];
            idx++;
            if (idx >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        }
    }
    else if (wcscmp(type, L"short") == 0)
    {
        for (i = 0; i < sizeof(short); i++)
        {
            memory.GetMemoryAt(idx) = p[i];
            idx++;
            if (idx >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        }
    }
    else if (wcscmp(type, L"char") == 0)
    {
        for (i = 0; i < sizeof(char); i++)
        {
            memory.GetMemoryAt(idx) = p[i];
            idx++;
            if (idx >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        }
    }
    else 
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    //Pad the alignment with 0.
    while (i % align.GetInteger() != 0)
    {
        if (idx >= memory.GetDataSize())
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        memory.GetMemoryAt(idx) = 0;
        i++;
        idx++;
    }
    
    return Var(static_cast<long>(idx));
}

Var Binary_PutReal(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vdata = localVarTable.Find(L"data", MODULE_IRRELEVANT, 4);
    Var* vtype = localVarTable.Find(L"type", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    if (vidx == nullptr || vdata == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    int idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

    wchar_t* type = nullptr;
    if (vtype != nullptr)
    {
        type = vtype->GetString();
        //convert to lower case
        while (*type)
        {
            if (*type >= L'A' && *type <= L'Z')
                *type = *type - L'A' + L'a';
            type++;
        }
        type = vtype->GetString();
    }
    int i = 0;
    if (type == nullptr || wcscmp(type, L"double") == 0)
    {
        double data = vdata->GetReal();
        uint8_t *p = (uint8_t*)&data;
        for (i = 0; i < sizeof(double); i++)
        {
            memory.GetMemoryAt(idx) = p[i];
            idx++;
            if (idx >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        }
    }
    else if (wcscmp(type, L"float") == 0)
    {
        float data = static_cast<float>(vdata->GetReal());
        uint8_t *p = (uint8_t*)&data;
        for (i = 0; i < sizeof(float); i++)
        {
            memory.GetMemoryAt(idx) = p[i];
            idx++;
            if (idx >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        }
    }
    else 
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    //Pad the alignment with 0.
    while (i % align.GetInteger() != 0)
    {
        if (idx >= memory.GetDataSize())
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        memory.GetMemoryAt(idx) = 0;
        i++;
        idx++;
    }
    
    return Var(static_cast<long>(idx));
}

Var Binary_PutString(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vdata = localVarTable.Find(L"data", MODULE_IRRELEVANT, 4);
    Var* vtype = localVarTable.Find(L"encoding", MODULE_IRRELEVANT, 8);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    if (vidx == nullptr || vdata == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    int idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

    wchar_t* type = nullptr;
    if (vtype != nullptr)
    {
        type = vtype->GetString();
        //convert to lower case
        while (*type)
        {
            if (*type >= L'A' && *type <= L'Z')
                *type = *type - L'A' + L'a';
            type++;
        }
        type = vtype->GetString();
    }
    int i = 0;
    wchar_t* pdata = vdata->GetString();
    if (type == nullptr || wcscmp(type, L"ucs4") == 0)
    {
        while (true)
        {
            if (idx + 4 >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
            wchar_t data = *pdata;
            memory.GetMemoryAt(idx++) = data & 0xff;
            data >>= 8;
            memory.GetMemoryAt(idx++) = data & 0xff;
            data >>= 8;
            memory.GetMemoryAt(idx++) = data & 0xff;
            data >>= 8;
            memory.GetMemoryAt(idx++) = data & 0xff;
            data >>= 8;
            i += 4;
            if (*pdata == 0)
                break;
            pdata++;
        }
    }
    else if (wcscmp(type, L"ucs2") == 0)
    {
        while (true)
        {
            if (idx + 2 >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
            wchar_t data = *pdata;
            if (sizeof(wchar_t) == 4 && data > 0xffff)
                throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
            memory.GetMemoryAt(idx++) = data & 0xff;
            data >>= 8;
            memory.GetMemoryAt(idx++) = data & 0xff;
            data >>= 8;
            i += 2;
            if (*pdata == 0)
                break;
            pdata++;
        }
    }
    else if (wcscmp(type, L"utf8") == 0)
    {
        std::string toPut = wchar_to_utf8(pdata);
        if (idx + toPut.size() + 1 >= memory.GetDataSize())
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        for (auto& item : toPut)
        {
            memory.GetMemoryAt(idx++) = static_cast<uint8_t>(item);
        }
        memory.GetMemoryAt(idx++) = 0;
        i += static_cast<int>(toPut.size()) + 1;
    }
    else 
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    //Pad the alignment with 0.
    while (i % align.GetInteger() != 0)
    {
        if (idx >= memory.GetDataSize())
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        memory.GetMemoryAt(idx) = 0;
        i++;
        idx++;
    }
    
    return Var(static_cast<long>(idx));
}

Var Binary_GetInteger(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vtype = localVarTable.Find(L"type", MODULE_IRRELEVANT, 4);
    Var* vdata = localVarTable.Find(L"data", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    if (vidx == nullptr || vdata == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    int idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

    wchar_t* type = nullptr;
    if (vtype != nullptr)
    {
        type = vtype->GetString();
        //convert to lower case
        while (*type)
        {
            if (*type >= L'A' && *type <= L'Z')
                *type = *type - L'A' + L'a';
            type++;
        }
        type = vtype->GetString();
    }
    long data = 0;
    int i = 0;
    if (type == nullptr || wcscmp(type, L"long") == 0)
    {
        data = *((long *)&memory.GetMemoryAt(idx));
        i = sizeof(long);
    }
    else if (wcscmp(type, L"int") == 0)
    {
        data = *((int *)&memory.GetMemoryAt(idx));
        i = sizeof(int);
    }
    else if (wcscmp(type, L"short") == 0)
    {
        data = *((short *)&memory.GetMemoryAt(idx));
        i = sizeof(short);
    }
    else if (wcscmp(type, L"char") == 0)
    {
        data = *((char *)&memory.GetMemoryAt(idx));
        i = sizeof(char);
    }
    else 
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    vdata->SetInteger(data);
    return Var(static_cast<long>(idx) + max(i, align.GetInteger()));
}

Var Binary_GetReal(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vtype = localVarTable.Find(L"type", MODULE_IRRELEVANT, 4);
    Var* vdata = localVarTable.Find(L"data", MODULE_IRRELEVANT, 4);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    if (vidx == nullptr || vdata == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    int idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

    wchar_t* type = nullptr;
    if (vtype != nullptr)
    {
        type = vtype->GetString();
        //convert to lower case
        while (*type)
        {
            if (*type >= L'A' && *type <= L'Z')
                *type = *type - L'A' + L'a';
            type++;
        }
        type = vtype->GetString();
    }
    double data = 0;
    int i = 0;
    if (type == nullptr || wcscmp(type, L"double") == 0)
    {
        data = *((double *)&memory.GetMemoryAt(idx));
        i = sizeof(double);
    }
    else if (wcscmp(type, L"float") == 0)
    {
        data = *((float *)&memory.GetMemoryAt(idx));
        i = sizeof(float);
    }
    else 
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    vdata->SetReal(data);
    return Var(static_cast<long>(idx) + max(i, align.GetInteger()));
}

Var Binary_GetString(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4);
    Var* vidx = localVarTable.Find(L"index", MODULE_IRRELEVANT, 5);
    Var* vdata = localVarTable.Find(L"data", MODULE_IRRELEVANT, 4);
    Var* vtype = localVarTable.Find(L"encoding", MODULE_IRRELEVANT, 8);
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& memory = self->GetStructureMember(memberPos - 2);
    Var& align = self->GetStructureMember(memberPos - 1);
    if (vidx == nullptr || vdata == nullptr)
        throw Var(Exception::ERROR_CODE::PARAM_EXPECTED);
    long idx = vidx->GetInteger();
    if (idx < 0 || idx >= memory.GetDataSize())
        throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

    wchar_t* type = nullptr;
    if (vtype != nullptr)
    {
        type = vtype->GetString();
        //convert to lower case
        while (*type)
        {
            if (*type >= L'A' && *type <= L'Z')
                *type = *type - L'A' + L'a';
            type++;
        }
        type = vtype->GetString();
    }
    int i = 0;
    std::wstring wstr;
    if (type == nullptr || wcscmp(type, L"ucs4") == 0)
    {
        uint32_t data;
        do
        {
            if (idx + 4 >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
            data = memory.GetMemoryAt(idx++);
            data += (memory.GetMemoryAt(idx++) << 8);
            data += (memory.GetMemoryAt(idx++) << 16);
            data += (memory.GetMemoryAt(idx++) << 24);
            if (sizeof(wchar_t) == 2 && data > 0xffff)
                throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
            wstr.push_back(data);
            i += 4;
        } while (data != 0);
    }
    else if (wcscmp(type, L"ucs2") == 0)
    {
        wchar_t data;
        do
        {
            if (idx + 4 >= memory.GetDataSize())
                throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
            data = memory.GetMemoryAt(idx++);
            data += (memory.GetMemoryAt(idx++) << 8);
            wstr.push_back(data);
            i += 2;
        } while (data != 0);
    }
    else if (wcscmp(type, L"utf8") == 0)
    {
        char* utf8 = (char *)memory.GetMemory() + idx;
        while (utf8[i] != 0 && i + idx < memory.GetDataSize())
        {
            i++;
        }
        if (utf8[i] == 0)
            i++;
        else
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        wstr = utf8_to_wchar(utf8);
        idx += i;
    }
    else 
        throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
    //Pad the alignment with 0.
    while (i % align.GetInteger() != 0)
    {
        if (idx >= memory.GetDataSize())
            break;
        i++;
        idx++;
    }
    
    vdata->SetString(wstr.c_str());
    return Var(idx);
}

//structure body
void BuildStruct_Binary(Parser& parser, bool chn)
{
    StructureDefinition bin;
    bin.moduleIdx = 0;
    bin.parentStructIndex = -1;
    //Field members
    BuildStructMember(bin, L"#mem", VarType::Memory);  //Named as #mem to avoid being visited.
    BuildStructMember(bin, L"#alignsize", VarType::Integer);  //Named as #mem to avoid being visited.
    //Add structure to parser
    parser.structureNameIndex.Insert(L"Binary", 0, static_cast<int>(parser.structureTable.size()));
    parser.structureTable.push_back(bin);
    //Define member functions
    //  Create
    FunctionDefinition binCreate;
    binCreate.internalStructFunc = Binary_Create;
    binCreate.moduleIdx = 0;
    binCreate.returnStructIndex = -1;   //Not returning a structure
    binCreate.returnType = VarType::Boolean;
    binCreate.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binCreate, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binCreate, L"size", VarType::Integer);
    parser.functionNameIndex.Insert(L"create#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binCreate);
    // SetAlign
    FunctionDefinition binSetAlign;
    binSetAlign.internalStructFunc = Binary_SetAlign;
    binSetAlign.moduleIdx = 0;
    binSetAlign.returnStructIndex = -1;   //Not returning a structure
    binSetAlign.returnType = VarType::Boolean;
    binSetAlign.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binSetAlign, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binSetAlign, L"alignsize", VarType::Integer);
    parser.functionNameIndex.Insert(L"setalign#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binSetAlign);
    //  getsize
    FunctionDefinition binGetSize;
    binGetSize.internalStructFunc = Binary_GetSize;
    binGetSize.moduleIdx = 0;
    binGetSize.returnStructIndex = -1;   //Not returning a structure
    binGetSize.returnType = VarType::Integer;
    binGetSize.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binGetSize, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"getsize#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binGetSize);
    //  getat
    FunctionDefinition binGetAt;
    binGetAt.internalStructFunc = Binary_GetAt;
    binGetAt.moduleIdx = 0;
    binGetAt.returnStructIndex = -1;   //Not returning a structure
    binGetAt.returnType = VarType::Integer;
    binGetAt.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binGetAt, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binGetAt, L"index", VarType::Integer);
    parser.functionNameIndex.Insert(L"getat#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binGetAt);
    //  setat
    FunctionDefinition binSetAt;
    binSetAt.internalStructFunc = Binary_SetAt;
    binSetAt.moduleIdx = 0;
    binSetAt.returnStructIndex = -1;   //Not returning a structure
    binSetAt.returnType = VarType::Integer;
    binSetAt.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binSetAt, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binSetAt, L"index", VarType::Integer);
    BuildFuncParam(binSetAt, L"byte", VarType::Integer);
    parser.functionNameIndex.Insert(L"setat#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binSetAt);
    //  putinteger
    FunctionDefinition binPutInteger;
    binPutInteger.internalStructFunc = Binary_PutInteger;
    binPutInteger.moduleIdx = 0;
    binPutInteger.returnStructIndex = -1;   //Not returning a structure
    binPutInteger.returnType = VarType::Integer;
    binPutInteger.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binPutInteger, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binPutInteger, L"index", VarType::Integer);
    BuildFuncParam(binPutInteger, L"data", VarType::Integer);
    BuildFuncParam(binPutInteger, L"type", VarType::String);
    parser.functionNameIndex.Insert(L"putinteger#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binPutInteger);
    //  putreal
    FunctionDefinition binPutReal;
    binPutReal.internalStructFunc = Binary_PutReal;
    binPutReal.moduleIdx = 0;
    binPutReal.returnStructIndex = -1;   //Not returning a structure
    binPutReal.returnType = VarType::Integer;
    binPutReal.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binPutReal, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binPutReal, L"index", VarType::Integer);
    BuildFuncParam(binPutReal, L"data", VarType::Real);
    BuildFuncParam(binPutReal, L"type", VarType::String);
    parser.functionNameIndex.Insert(L"putreal#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binPutReal);
    //  putstring
    FunctionDefinition binPutString;
    binPutString.internalStructFunc = Binary_PutString;
    binPutString.moduleIdx = 0;
    binPutString.returnStructIndex = -1;   //Not returning a structure
    binPutString.returnType = VarType::Boolean;
    binPutString.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binPutString, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binPutString, L"index", VarType::Integer);
    BuildFuncParam(binPutString, L"data", VarType::String);
    BuildFuncParam(binPutString, L"encoding", VarType::String);
    parser.functionNameIndex.Insert(L"putstring#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binPutString);
    //  getinteger
    FunctionDefinition binGetInteger;
    binGetInteger.internalStructFunc = Binary_GetInteger;
    binGetInteger.moduleIdx = 0;
    binGetInteger.returnStructIndex = -1;   //Not returning a structure
    binGetInteger.returnType = VarType::Integer;
    binGetInteger.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binGetInteger, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binGetInteger, L"index", VarType::Integer);
    BuildFuncParam(binGetInteger, L"data", VarType::Integer | VarType::ReferenceFlag);
    BuildFuncParam(binGetInteger, L"type", VarType::String);
    parser.functionNameIndex.Insert(L"getinteger#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binGetInteger);
    //  getreal
    FunctionDefinition binGetReal;
    binGetReal.internalStructFunc = Binary_GetReal;
    binGetReal.moduleIdx = 0;
    binGetReal.returnStructIndex = -1;   //Not returning a structure
    binGetReal.returnType = VarType::Real;
    binGetReal.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binGetReal, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binGetReal, L"index", VarType::Integer);
    BuildFuncParam(binGetReal, L"data", VarType::Real | VarType::ReferenceFlag);
    BuildFuncParam(binGetReal, L"type", VarType::String);
    parser.functionNameIndex.Insert(L"getreal#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binGetReal);
    //  getstring
    FunctionDefinition binGetString;
    binGetString.internalStructFunc = Binary_GetString;
    binGetString.moduleIdx = 0;
    binGetString.returnStructIndex = -1;   //Not returning a structure
    binGetString.returnType = VarType::Integer;
    binGetString.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(binGetString, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    BuildFuncParam(binGetString, L"index", VarType::Integer);
    BuildFuncParam(binGetString, L"data", VarType::String | VarType::ReferenceFlag);
    BuildFuncParam(binGetString, L"encoding", VarType::String);
    parser.functionNameIndex.Insert(L"getstring#3", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(binGetString);
}

//-------------------------------------------------------------------------------------
void InitializeBuiltinStructs(Parser& parser, bool chn /* = false */)
{
    BuildStruct_Exception(parser, chn); //#0
    BuildStruct_DateTime(parser, chn);  //#1
    BuildStruct_File(parser, chn);      //#2
    BuildStruct_Binary(parser, chn);    //#3
    BuildStruct_Thread(parser, chn);    //#4
    BuildStruct_Mutex(parser, chn);     //#5
}