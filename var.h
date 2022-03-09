#pragma once

#include <string>
#include <vector>
#include "WordReader.h"
#include "Exception.h"
#include "TableTypes.h"
#include <iostream>
#include <thread>
#include <atomic>

class Parser;
class Var
{
public:
    struct QueueInfo
    {
        int type;   //ST_FOR, ST_WHILE, ...
        size_t srcPosSave;
        Var* controllerVar {nullptr};
        Var* iterateVar {nullptr};
        double step;
        double to;
        long index {0}; //Used by foreach - next/endfor
    };

protected:
    VarType varType { VarType::Null };
    union UnitedData
    {
        bool boolean;
        wchar_t charactor;
        long integer;
        double real;
        wchar_t* string;
        Var* array;
        uint8_t* memory;
        Var* structure;
        Var* tuple;
        Var* referedTo;
        void* internalQueue;  //For internal use only. Points to a queue for loops, tries, etc.
        void* pointer;        //For internal use only. Save a pointer for any purpose.
    } data;

    union ExtraData
    {
        long dataSize;      //Size of string, memory, array, structure, tuple, etc. Not used for other data type.
        long referedStructureIdx;   //If it is a structure byref, this stores the structure index
    } extraData;

    volatile int refCount;      //Reference count of this var.

    inline void RemoveMemory()
    {
        if (!varType.IsReference())  //Do not bother the memory of a referenced variable
        {
            RemoveMemory(data, varType);
            data.memory = nullptr;
            extraData.dataSize = 0;
        }
    }

    inline void RemoveMemory(UnitedData& data, VarType& varType)
    {
        if (varType == VarType::String)         //if...else is more elegant than switch...case
            delete[] data.string;
        else if (varType == VarType::Array)
            delete[] data.array;
        else if (varType == VarType::Memory)
            delete[] data.memory;
        else if (varType == VarType::Structure)
            delete[] data.structure;
        else if (varType == VarType::InternalLoopQueue)
            delete (std::vector<QueueInfo> *)data.internalQueue;
        else if (varType == VarType::InternalPointer)
            delete[] data.memory;
        else if (varType == VarType::InternalThreadObj)
            delete (std::thread*)data.pointer;
        data.memory = nullptr;
    }

    inline void ClearData()
    {
        for (int i = 0; i < sizeof(data); i++)
            ((char *)&data)[i] = 0;
    }
public:
    //Getter of regular data type
    bool GetBool() const;
    long GetInteger() const;
    double GetReal() const;
    wchar_t* GetString() const;
    wchar_t GetChar() const;
    uint8_t* GetMemory() { return data.memory; }
    inline long GetDataSize() const 
    {
        if (varType.IsReference())
            return data.referedTo->GetDataSize();
        else 
            return extraData.dataSize; 
    }
    void* GetPointer() { return data.pointer; }   //Internal use only.

    //Setter of regular data type
    void SetBool(const bool& val);
    void SetInteger(const long& val);
    void SetReal(const double& val);
    void SetString(const wchar_t* val, size_t size = -1);
    void EmplaceString(wchar_t* mem);
    void SetChar(const wchar_t& ch);
    void SetArray(const Var& array);
    void SetMemory(const Var& memory);
    void SetStructure(const Var& structure);
    void SetPointer(void* pointer) { data.pointer = pointer; }  //Internal use only, no safety/type check.

    void Clear()
    {
        if (IsReference() && HasReferedTo())
            data.referedTo->DecreaseReferCount();

        extraData.dataSize = 0;
        data.referedTo = nullptr;
        varType = VarType::Null;
    }
    //Special setter. Move the content of another var to this var instead of copying it. 
    void MoveFrom(Var&& var)
    {
        if (varType.IsReference())
            data.referedTo->MoveFrom(std::move(var));
        else
        {
            varType = var.varType;
            data = var.data;
            extraData.dataSize = var.extraData.dataSize;
            refCount = static_cast<long>(var.refCount);    //!!!!!!!!!!!!!!! problem!
            if (this != &var)
                var.ClearData();    //So that the pointers are 0 and will not be re-deleted at any case.
        }
    }

    //Special setter. Make this var a reference to another var.
    //This function will reset the current content and make myself refer to another Var anyway.
    inline void ResetAndReferTo(Var* var)
    {
        if (IsReference() && HasReferedTo())
            data.referedTo->DecreaseReferCount();

        RemoveMemory();
        varType = VarType::ReferenceFlag | var->varType;
        if (var->varType.IsReference())
            data.referedTo = var->data.referedTo;
        else
            data.referedTo = var;
        data.referedTo->IncreaseReferCount();
    }

    //Special setter. Make this var a reference to another var.
    //This function will check the type. Only Null type or the same type with reference flag can refer to another Var.
    void ReferTo(Var* var, Parser& parser);

    //For those setting a 'byref' var to null, this will be called.
    void ClearRefer()
    {
        if (IsReference() && HasReferedTo())
            data.referedTo->DecreaseReferCount();

        data.referedTo = nullptr;
    }

    //Reference count increases only when "@=" is operated
    inline void IncreaseReferCount()
    {
        //!!!!!!!!!!!!!!!! SHOULD BE THREAD SAFE !!!!!!!!!!!!!!!!!
        if (varType.IsReference() && HasReferedTo())
            data.referedTo->IncreaseReferCount();
        else
            refCount++;
    }
    //Reference count decreases when a function ends and local varables that is a reference to other variables are to be release.
    //And when "@=" of a variable change its reference to another.
    inline void DecreaseReferCount()
    {
        //!!!!!!!!!!!!!!!! SHOULD BE THREAD SAFE !!!!!!!!!!!!!!!!!
        if (varType.IsReference() && HasReferedTo())
                data.referedTo->DecreaseReferCount();
        else
            refCount--;
    }

    inline long GetReferenceCount()
    {
        long result = 0;
        if (varType.IsReference() && data.referedTo != nullptr)
            result = data.referedTo->GetReferenceCount();
        else
            result = refCount;
        return result;
    }

    //Array type methods
    void InitArray(long size, VarType elementVarType = VarType::Null);  //For built-in data type
    void InitArray(long size, const std::vector<StructureDefinition>& sdTable, const int& index);   //For structrure array
    void ResizeArray(long size, const std::vector<StructureDefinition>& sdTable);
    Var& GetElementAt(const long& idx);                 //Return an element for read/write.
    const Var& ReadElementAt(const long& idx) const;    //Return an element for read only.
    
    //Memory type methods
    void InitMemory(long size);
    void ResizeMemory(long size);
    uint8_t& GetMemoryAt(const long& idx);

    //Structure type methods
    void InitStructure(const std::vector<StructureDefinition>& sdTable, const int& index);
    void InitStructure(const int& size, const int& index);  //Used by copy construtor
    Var& GetStructureMember(const long& idx);               //Return a member for read/write.
    Var& GetStructureMember(std::vector<StructureDefinition>& sdTable, wchar_t* name, int len = -1);
    const Var& ReadStructureMember(const long& idx) const;  //Return a member for read only.
    Var* FindStructureMember(std::vector<StructureDefinition>& sdTable, wchar_t* name, int len = -1);

    //Internal usage
    void InitInternalQueue()
    {
        data.internalQueue = new std::vector<Var::QueueInfo>();
        varType = VarType::InternalLoopQueue;
    }
    std::vector<Var::QueueInfo>* GetInternalQueue()
    {
        auto que = (std::vector<Var::QueueInfo> *)data.internalQueue;
        return que;
    }
 
    //Get the real data type of this variable. Reference and/or constant properties are omitted.
    inline VarType GetVarType() const
    { 
        return varType & (~(VarType::ReferenceFlag | VarType::ConstFlag)); 
    }
    inline bool IsReference() const
    {
        return varType.IsReference();
    }
    inline bool HasReferedTo() const
    {
        return data.referedTo != nullptr;
    }
    inline bool IsConstant() const
    {
        return varType.IsConstant();
    }
    inline Var* GetReferedTo()
    {
        return data.referedTo;
    }
    //Default constructor
    Var();
    Var(Exception::ERROR_CODE errCode)
    {
        SetVarType(VarType::InternalException);
        data.integer = static_cast<int>(errCode);
    }
    //Copy constructor for regular data types
    Var(const bool& val) { SetBool(val); }
    Var(const long& val) { SetInteger(val); }
    Var(const double& val) { SetReal(val); }
    Var(const wchar_t*val, size_t size = -1) { SetString(val, size); }
    Var(const wchar_t& ch) { SetChar(ch); }
    //Copy constructor for Var type
    Var(const Var& var);
    //Move constructor.
    Var(Var&& var);

    //Destructor
    ~Var()
    {
        if (IsReference() && HasReferedTo())
            data.referedTo->DecreaseReferCount();
        RemoveMemory();
    }

    Var& operator=(const Var& varIn);

    //Initialize var type
    void SetVarType(VarType type, long referedStructureIndex = -1)
    {
        if (type == VarType::String)
            SetString(L"");
        else
            varType = type;
        if (type.IsStructure() && type.IsReference())
            extraData.referedStructureIdx = referedStructureIndex;
    }

    long GetReferedStructureIdx()
    {
        return extraData.referedStructureIdx;
    }
};
