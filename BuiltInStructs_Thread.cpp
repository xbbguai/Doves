#include "TableTypes.h"
#include "BuiltInStructs.h"
#include "parser.h"
#include "textencoding.h"
#include <thread>
#include <mutex>

//Start point for each user thread.
void ThreadFunction(Parser* pparser, Var* self)
{
    int memberPos = self->GetDataSize();
    Var& running = self->GetStructureMember(memberPos - 2);
    running.SetBool(true);

    //Find runner function and execute.
    Parser parser;
    parser.CopyInfoFrom(*pparser);

    long sdTableIdx = self->GetStructureMember(-1).GetInteger();	//Use -1 to get the structure table index.
    wchar_t temp[20] {L'R', L'U', L'N', L'N', L'E', L'R', L'#'};
    int* idx;
    while (true)
    {
        //If it is a member function of a structure, the function name become FunctionName{#Index}
        int count = parser.IntToWCS(sdTableIdx, temp + 7);
        idx = parser.functionNameIndex.Find(temp, parser.structureTable[sdTableIdx].moduleIdx, 7 + count);
        if (idx == nullptr && parser.structureTable[sdTableIdx].parentStructIndex != -1)
        {
            //There is a parent. I should try parent.
            sdTableIdx = parser.structureTable[sdTableIdx].parentStructIndex;
        }
        else
            break;
    }
    if (idx == nullptr)
        throw Var(Exception::ERROR_CODE::IDENTIFIER_NOT_DEFINED);

    HashTable<Var> paramTable;
    Var& byref = paramTable.Insert(L"self", MODULE_IRRELEVANT, Var());
    byref.ReferTo(self, parser);
    self->IncreaseReferCount();	//Can also call byref.IncreaseReferCount();
    //Call the user's 'runner' function
    Var result = parser.Execute(*idx, paramTable);
    self->DecreaseReferCount();
    running.SetBool(false);

    delete self;
}

//Internal structure of THREAD
//Member function(s)
Var Thread_Start(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4); 
    int memberPos = self->GetDataSize() - 1;    //I'm always the father. My member variables are indexed as the last ones.
    Var& itnThrdObj = self->GetStructureMember(memberPos); 
    //Caller's localVarTable
    Var* fvtVar = localVarTable.Find(L"#fvt", MODULE_IRRELEVANT, 4);
    HashTable<Var>* fvt = (HashTable<Var>*)fvtVar->GetPointer();

    std::thread* threadObj = (std::thread*)(itnThrdObj.GetPointer());
    if (threadObj != nullptr)
    {
        if (threadObj->joinable())
            threadObj->join();
        //Remove this thread object from caller localVarTable's anonymous variable list.
        for (auto it = fvt->anonymousList.begin(); it != fvt->anonymousList.end(); it++)
        {
            if ((*it).GetPointer() == (void*)threadObj)
            {
                fvt->anonymousList.erase(it);   //This will delete threadObj because ~Var() will do it.
                break;
            }
        }
    }

    //Make a copy of 'self'
    Var* newSelf = new Var();
    newSelf->ReferTo(self, parser);
    //New thread
    threadObj = new std::thread(ThreadFunction, &parser, newSelf);
    itnThrdObj.SetPointer((void *)threadObj);
 
    //Add this threadObj to anonymous list so that when the function holding this thread is exiting, the thread is joined and
    //the function waits for the thread to end before being destroied.
    Var varThreadObj;
    varThreadObj.SetPointer((void*)threadObj);
    varThreadObj.SetVarType(VarType::InternalThreadObj);
    varThreadObj.IncreaseReferCount();
    fvt->InsertAnonymous(varThreadObj);
    varThreadObj.Clear();   //It has been copied into the list.

    return Var();
}

Var Thread_IsRunning(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4); 
    int memberPos = self->GetDataSize();    //I'm always the father. My member variables are indexed as the last ones.
    Var& itnThrdObj = self->GetStructureMember(memberPos - 1); 
    Var& running = self->GetStructureMember(memberPos - 2);

    std::thread* threadObj = (std::thread*)(itnThrdObj.GetPointer());
    if (threadObj == nullptr || threadObj->joinable() == false || running.GetBool() == false)
        return Var(false);
    else 
        return Var(true);
}

Var Thread_Runner(HashTable<Var>& localVarTable, Parser& parser)
{
    return Var();
}

//Structure body
void BuildStruct_Thread(Parser& parser, bool chn)
{
    StructureDefinition thread;
    thread.moduleIdx = 0;
    thread.parentStructIndex = -1;
    //Field members
    BuildStructMember(thread, L"running", VarType::Boolean);
    BuildStructMember(thread, L"itnThrdObj#", VarType::Null);   //Store thread object's pointer, but set to Null to avoid being deleted.
    //Add structure to parser
    parser.structureNameIndex.Insert(L"thread", 0, static_cast<int>(parser.structureTable.size()));
    parser.structureTable.push_back(thread);
    //Define member functions
    // Start
    FunctionDefinition threadStart;
    threadStart.internalStructFunc = Thread_Start;
    threadStart.moduleIdx = 0;
    threadStart.returnStructIndex = -1;   //No return
    threadStart.returnType = VarType::Null;
    threadStart.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(threadStart, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"Start#4", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(threadStart);
    // IsRunning
    FunctionDefinition threadIsRunning;
    threadIsRunning.internalStructFunc = Thread_IsRunning;
    threadIsRunning.moduleIdx = 0;
    threadIsRunning.returnStructIndex = -1;   //No return
    threadIsRunning.returnType = VarType::Boolean;
    threadIsRunning.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(threadIsRunning, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"IsRunning#4", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(threadIsRunning);
    // Runner
    FunctionDefinition threadRunner;
    threadRunner.internalStructFunc = Thread_Runner;
    threadRunner.moduleIdx = 0;
    threadRunner.returnStructIndex = -1;   //No return
    threadRunner.returnType = VarType::Null;
    threadRunner.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(threadRunner, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"Runner#4", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(threadRunner);
}
//-------------------------------------------------------------------------------------

static std::mutex publicMtx;

//Internal structure of MUTEX
//Member functions
Var Mutex_Lock(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4); 
    int memberPos = self->GetDataSize() - 1;    //I'm always the father. My member variables are indexed as the last ones.
    Var& itnMtx = self->GetStructureMember(memberPos); 
    
    std::unique_lock lock = std::unique_lock(publicMtx);
    long mtxCount = itnMtx.GetInteger();
    mtxCount++;
    itnMtx.SetInteger(mtxCount);
    lock.unlock();

    while (itnMtx.GetInteger() > 1)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return Var();
}

Var Mutex_Unlock(HashTable<Var>& localVarTable, Parser& parser)
{
    //'self' can never be null. Should be a structure.
    Var* self = localVarTable.Find(L"self", MODULE_IRRELEVANT, 4); 
    int memberPos = self->GetDataSize() - 1;    //I'm always the father. My member variables are indexed as the last ones.
    Var& itnMtx = self->GetStructureMember(memberPos); 

    std::unique_lock lock = std::unique_lock(publicMtx);
    long mtxCount = itnMtx.GetInteger();
    if (mtxCount > 0)
        mtxCount--;
    itnMtx.SetInteger(mtxCount);
    lock.unlock();

    return Var();
}

//Structure body
void BuildStruct_Mutex(Parser& parser, bool chn)
{
    StructureDefinition mutex;
    mutex.moduleIdx = 0;
    mutex.parentStructIndex = -1;
    //Field members
    BuildStructMember(mutex, L"itnMtx#", VarType::Null);
    //Add structure to parser
    parser.structureNameIndex.Insert(L"mutex", 0, static_cast<int>(parser.structureTable.size()));
    parser.structureTable.push_back(mutex);
    //Define member functions
    // lock
    FunctionDefinition mutexLock;
    mutexLock.internalStructFunc = Mutex_Lock;
    mutexLock.moduleIdx = 0;
    mutexLock.returnStructIndex = -1;   //No return
    mutexLock.returnType = VarType::Null;
    mutexLock.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(mutexLock, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"Lock#5", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(mutexLock);
    // unlock
    FunctionDefinition mutexUnlock;
    mutexUnlock.internalStructFunc = Mutex_Unlock;
    mutexUnlock.moduleIdx = 0;
    mutexUnlock.returnStructIndex = -1;   //No return
    mutexUnlock.returnType = VarType::Boolean;
    mutexUnlock.sourceCodePosition = 0;   //Internal. Not in source code.
    BuildFuncParam(mutexUnlock, L"self", VarType::Structure | VarType::ReferenceFlag, static_cast<int>(parser.structureTable.size()) - 1);
    parser.functionNameIndex.Insert(L"Unlock#5", 0, static_cast<int>(parser.functionTable.size()));
    parser.functionTable.push_back(mutexUnlock);
}