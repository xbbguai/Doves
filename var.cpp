#include "var.h"
#include "parser.h"

//Default constructor
Var::Var()
{
    ClearData();
    varType = VarType::Null;
    extraData.dataSize = 0;
    refCount = 0;
#ifdef TRACE_VAR_CONSTRUCTION
    std::cout << "Default constructor" << std::endl;
#endif
}

//Copy constructor
Var::Var(const Var& var)
{
//    operator=(var);
#ifdef TRACE_VAR_CONSTRUCTION
    std::cout << "Copy constructor" << std::endl;
#endif
    if (var.varType.IsReference())
    {
        varType = var.varType;
        data = var.data;
        extraData = var.extraData;
        refCount = 0;
        return;    
    }
    else
        operator=(var);
}

//Move constructor
Var::Var(Var&& var)
{
    MoveFrom(std::forward<Var>(var));
#ifdef TRACE_VAR_CONSTRUCTION
    std::cout << "Rvalue constructor" << std::endl;
#endif
}

//operator =
Var& Var::operator=(const Var& varIn)
{
#ifdef TRACE_VAR_CONSTRUCTION
    std::cout << "Operator =" << std::endl;
#endif
    if (IsReference())
        return data.referedTo->operator=(varIn);

    if (varIn.varType.IsReference() && varIn.data.referedTo == nullptr)
    {
        //Wanna copy from a reference variable that is not referenced.
        //So, just let me be Null.
        varType = VarType::Null;
        return *this;        
    }

    const Var& var = varIn.varType.IsReference() ? *(varIn.data.referedTo) : varIn;

    switch (var.GetVarType())
    {
    case VarType::String:
        SetString(var.data.string);
        break;
    case VarType::Array:
        //An array, whatever anonymous or not, can be used to initialize a structure.
        //So, the type of myself should be checked.
        if (GetVarType().IsStructure())
            SetStructure(var);
        else
            SetArray(var);
        break;
    case VarType::Structure:
        SetStructure(var);
        break;
    case VarType::Memory:
        SetMemory(var);
        break;
    case VarType::Integer:
        SetInteger(var.data.integer);
        break;
    case VarType::Real:
        SetReal(var.data.real);
        break;
    case VarType::Boolean:
        SetBool(var.data.boolean);
        break;
    case VarType::Char:
        SetChar(var.data.charactor);
        break;
    case VarType::InternalLoopQueue:
        varType = VarType::InternalLoopQueue;
        data = var.data;
        break;
    default:
        varType = var.GetVarType();
        data = var.data;
    }
    if (var.GetVarType() != VarType::Array)
        extraData.dataSize = var.extraData.dataSize;
    refCount = 0;
    return *this;
}

bool Var::GetBool() const
{
    if (varType.IsReference())
        return data.referedTo->GetBool();
    switch (GetVarType())
    {
    case VarType::Boolean:
        return data.boolean;
    case VarType::Integer:
        return data.integer != 0;
    case VarType::Char:
        return data.charactor != 0;
    case VarType::Real:
        return data.real != 0;
    case VarType::Null:
        return false;
    default:
        throw Var(Exception::ERROR_CODE::CANNOT_CONVERT_DATATYPE);
    }
}

long Var::GetInteger() const
{
    if (varType.IsReference())
        return data.referedTo->GetInteger();
    switch (GetVarType())
    {
    case VarType::Integer:
    case VarType::InternalException:
    case VarType::InternalModuleIdx:
    case VarType::InternalDLL:
    case VarType::InternalThreadObj:
        return data.integer;
    case VarType::Char:
        return static_cast<long>(data.charactor);
    case VarType::Boolean:
        return data.boolean ? 1 : 0;
    case VarType::Real:
    {
        const double &val = data.real;
        if (val < std::numeric_limits<long>::min() || val > std::numeric_limits<long>::max())
            throw Var(Exception::ERROR_CODE::VALUE_OUT_OF_RANGE);
        return static_cast<long>(val);
    }
    case VarType::Null:
        return 0;
    default:
        throw Var(Exception::ERROR_CODE::CANNOT_CONVERT_DATATYPE);
    }
}

double Var::GetReal() const
{
    if (varType.IsReference())
        return data.referedTo->GetReal();
    switch (GetVarType())
    {
    case VarType::Real:
        return data.real;
    case VarType::Integer:
        return static_cast<double>(data.integer);
    case VarType::Char:
        return static_cast<double>(data.charactor);
    case VarType::Boolean:
        return data.boolean ? 1 : 0;
    case VarType::Null:
        return 0;
    default:
        throw Var(Exception::ERROR_CODE::CANNOT_CONVERT_DATATYPE);
    }
}

wchar_t *Var::GetString() const
{
    if (varType.IsReference())
        return data.referedTo->GetString();
    else if (GetVarType() == VarType::String)
        return data.string;
    else if (GetVarType() == VarType::Null)
        return Var(L"").GetString();
    else
        throw Var(Exception::ERROR_CODE::CANNOT_CONVERT_DATATYPE);
}

wchar_t Var::GetChar() const
{
    if (varType.IsReference())
        return data.referedTo->GetChar();
    switch (GetVarType())
    {
    case VarType::Char:
        return data.charactor;
    case VarType::String:
        return data.string[0];
    case VarType::Integer:
    {
        const long &val = data.integer;
        if (val <= std::numeric_limits<wchar_t>::max() && val >= 0)
            return static_cast<wchar_t>(val);
        else
            throw Var(Exception::ERROR_CODE::CANNOT_CONVERT_DATATYPE);
    }
    case VarType::Real:
    {
        const double &val = data.real;
        if (val <= std::numeric_limits<wchar_t>::max() && val >= 0)
            return static_cast<wchar_t>(val);
        else
            throw Var(Exception::ERROR_CODE::CANNOT_CONVERT_DATATYPE);
    }
    default:
        throw Var(Exception::ERROR_CODE::CANNOT_CONVERT_DATATYPE);
    }
}

void Var::SetBool(const bool &val)
{
    if (varType.IsReference())
        data.referedTo->SetBool(val);
    else
    {
        if (varType == VarType::Boolean || varType == VarType::Null)
        {
            varType = VarType::Boolean;
            data.boolean = val;
        }
        else if (varType == VarType::Real)
            data.real = static_cast<double>(val == true);
        else if (varType == VarType::Integer)
            data.integer =  static_cast<long>(val == true);
        else if (varType == VarType::Char)
            data.charactor =  static_cast<wchar_t>(val == true);
        else
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    }
}

void Var::SetInteger(const long &val)
{
    if (varType.IsReference())
        data.referedTo->SetInteger(val);
    else
    {
        if (varType == VarType::Integer || varType == VarType::Null)
        {
            varType = VarType::Integer;
            data.integer = val;
        }
        else if (varType == VarType::Real)
            data.real = static_cast<double>(val);
        else if (varType == VarType::Boolean)
            data.boolean =  (val != 0);
        else if (varType == VarType::Char)
            data.charactor =  static_cast<wchar_t>(val);
        else if (varType == VarType::InternalThreadObj)
            data.integer = val;
        else
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    }
}

void Var::SetReal(const double &val)
{
    if (varType.IsReference())
        data.referedTo->SetReal(val);
    else
    {
        if (varType == VarType::Real || varType == VarType::Null)
        {
            varType = VarType::Real;
            data.real = val;
        }
        else if (varType == VarType::Integer)
            data.integer = static_cast<long>(val);
        else if (varType == VarType::Boolean)
            data.boolean =  (val != 0);
        else if (varType == VarType::Char)
            data.charactor =  static_cast<wchar_t>(val);
        else
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    }
}

void Var::SetString(const wchar_t *val, size_t size)
{
    if (varType.IsReference())
        data.referedTo->SetString(val, size);
    else
    {
        if (varType != VarType::Null && varType != VarType::String)
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);

        UnitedData dataSave = data;
        VarType varTypeSave = varType;
        if (size == -1 && val != nullptr)
            while (val[++size] != 0);
        else if (size == -1)
            size = 0;
        extraData.dataSize = static_cast<long>(size);
        data.string = new wchar_t[size + 1];
        data.string[size] = 0;
        while (size > 0)
        {
            size--;
            data.string[size] = val[size];
        }
        varType = VarType::String;
        RemoveMemory(dataSave, varTypeSave);
    }
}

void Var::EmplaceString(wchar_t *mem)
{
    if (varType.IsReference())
        data.referedTo->EmplaceString(mem);
    else
    {
        if (varType != VarType::Null && varType != VarType::String)
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);

        UnitedData dataSave = data;
        VarType varTypeSave = varType;

        data.string = mem;
        extraData.dataSize = 0;
        while (mem[extraData.dataSize] > 0)
            extraData.dataSize++;
        varType = VarType::String;
        RemoveMemory(dataSave, varTypeSave);
    }
}

void Var::SetChar(const wchar_t &val)
{
    if (varType.IsReference())
        data.referedTo->SetChar(val);
    else
    {
        if (varType == VarType::Char || varType == VarType::Null)
        {
            varType = VarType::Char;
            data.charactor = val;
        }
        else if (varType == VarType::Integer)
            data.integer = static_cast<long>(val);
        else if (varType == VarType::Boolean)
            data.boolean =  (val != 0);
        else if (varType == VarType::Real)
            data.real =  static_cast<double>(val);
        else
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
    }
}

void Var::SetArray(const Var& array)
{
    if (varType.IsReference())
        data.referedTo->SetArray(array);
    else
    {
        //Check for compatibility
        if (varType == VarType::Null || varType == VarType::Array && GetDataSize() == 0)
            InitArray(array.extraData.dataSize);

        if (GetDataSize() < array.GetDataSize())
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);

        for (int i = -2; i < array.extraData.dataSize; i++)   //Types of each element should match.
            GetElementAt(i) = array.ReadElementAt(i);
    }
}

void Var::SetStructure(const Var& structure)
{
    if (varType.IsReference())
        data.referedTo->SetStructure(structure);
    else if (structure.GetVarType().IsStructure())
    {
        //Check for compatibility
        int structIdx = structure.ReadStructureMember(-1).GetInteger();
        if (varType == VarType::Null)
        {
            InitStructure(structure.extraData.dataSize, structIdx);
        }
        else if (varType == VarType::Structure)
        {
            if (ReadStructureMember(-1).GetInteger() != structIdx)
                throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);
        }
        //Copy members
        for (int i = 0; i < extraData.dataSize; i++)
            GetStructureMember(i) = structure.ReadStructureMember(i);
    }
    else if (structure.GetVarType().IsIndexable())  //Array
    {
        if (extraData.dataSize < structure.GetDataSize())
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        //Copy array elements to members
        for (int i = 0; i < structure.GetDataSize(); i++)
        {
            GetStructureMember(i) = structure.ReadElementAt(i);
        }
    }
}

void Var::SetMemory(const Var& memory)
{
    if (varType.IsReference())
        data.referedTo->SetMemory(memory);
    else
    {
        //Check for compatibility
        if (varType == VarType::Memory)
            RemoveMemory();
        if (varType == VarType::Null || varType == VarType::Memory)
            InitMemory(memory.extraData.dataSize);
        else
            throw Var(Exception::ERROR_CODE::TYPE_MISMATCH);

        for (int i = 0; i < extraData.dataSize; i++)
            data.memory[i] = memory.data.memory[i];
    }
}

void Var::ReferTo(Var* var, Parser& parser)
{
    if (var->IsConstant())
        throw Var(Exception::ERROR_CODE::CANNOT_REFER_TO_DIFFERENT_TYPE);
    
    if (IsReference() && HasReferedTo())
        data.referedTo->DecreaseReferCount();
    
    if ((varType & (~VarType::ReferenceFlag)) == VarType::Null || 
        varType.IsReference() && varType == (var->varType | VarType::ReferenceFlag))
    {
        //If reference happens on structures, should check the structure type and the inheritance relationship.
        if (var->GetVarType().IsStructure())
        {
            //Father can refer to descendents.
            int idx;
            if (var->IsReference())
            {
                if (!var->HasReferedTo())
                    throw Var(Exception::ERROR_CODE::USING_NOT_INITIALIZED_REF);
                idx = var->data.referedTo->GetStructureMember(-1).GetInteger();
            }
            else
                idx = var->GetStructureMember(-1).GetInteger();
            if ((varType & (~VarType::ReferenceFlag)) == VarType::Null)
                extraData.referedStructureIdx = idx;
            else
            {
                while (idx != extraData.referedStructureIdx)
                {
                    idx = parser.structureTable[idx].parentStructIndex;
                    if (idx == -1)
                        throw Var(Exception::ERROR_CODE::CANNOT_REFER_TO_DIFFERENT_TYPE);
                }
            }
        }

        varType = var->varType | VarType::ReferenceFlag;
        if (var->varType.IsReference())
            data.referedTo = var->data.referedTo;
        else
            data.referedTo = var;
    }
    else
        throw Var(Exception::ERROR_CODE::CANNOT_REFER_TO_DIFFERENT_TYPE);
    
    data.referedTo->IncreaseReferCount();
}

// Array
// Allocates two more vars for array to store: elementVarType, structureTableIndex(if a structure array)
void Var::InitArray(long size, VarType elementVarType)
{
    if (varType.IsReference())
        data.referedTo->InitArray(size, elementVarType);
    else
    {
        extraData.dataSize = size;
        data.array = new Var[size + 2]();
        varType = VarType::Array;
        data.array[0].SetInteger(elementVarType);
        data.array[1].SetInteger(-1);
        if (elementVarType != VarType::Null)
        {
            for (long i = 2; i < size + 2; i++)
                data.array[i].SetVarType(elementVarType);
        }
    }
}

// Allocates two more vars for array to store: elementVarType, structureTableIndex(if a structure array)
void Var::InitArray(long size, const std::vector<StructureDefinition>& sdTable, const int& index)   //For structrure array
{
    if (varType.IsReference())
        data.referedTo->InitArray(size, sdTable, index);
    else
    {
        extraData.dataSize = size;
        data.array = new Var[size + 2]();
        varType = VarType::Array;
        data.array[0].SetInteger(VarType::Structure);
        data.array[1].SetInteger(index);
        for (long i = 2; i < size + 2; i++)
            data.array[i].InitStructure(sdTable, index);
    }
}

void Var::ResizeArray(long size, const std::vector<StructureDefinition>& sdTable)
{
    if (varType.IsReference())
        data.referedTo->ResizeArray(size, sdTable);
    else
    {
        Var *newAllocation = new Var[size + 2]();
        long i = 0;
        for (; i < std::min(size + 2, extraData.dataSize + 2); i++)
            newAllocation[i].MoveFrom(std::move(data.array[i]));
        RemoveMemory();
        data.array = newAllocation;
        extraData.dataSize = size;
        //data.array[0] is the varType of this array
        //data.array[1] is the structure index, if this array is of a structure.
        VarType elementVarType = static_cast<VarType>(data.array[0].GetInteger());
        if (elementVarType != VarType::Null)
        {
            int structIdx = data.array[1].GetInteger();
            for (; i < extraData.dataSize + 2; i++)
            {
                if (elementVarType.IsStructure())
                {
                    if (elementVarType.IsReference())
                        data.array[i].SetVarType(elementVarType, structIdx);
                    else
                        data.array[i].InitStructure(sdTable, structIdx);
                }
                else
                    data.array[i].SetVarType(elementVarType);
            }
        }
    }
}
Var& Var::GetElementAt(const long &idx)
{
    if (varType.IsReference())
        return data.referedTo->GetElementAt(idx);
    else
    {
        if (idx >= extraData.dataSize || idx < -2)
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        return data.array[idx + 2];
    }
}
const Var& Var::ReadElementAt(const long &idx) const
{
    if (varType.IsReference())
        return data.referedTo->ReadElementAt(idx);
    else
    {
        if (idx >= extraData.dataSize || idx < -2)
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
        return data.array[idx + 2];
    }
}


void Var::InitMemory(long size)
{
    if (varType.IsReference())
        data.referedTo->InitMemory(size);
    else
    {
        extraData.dataSize = size;
        data.memory = new uint8_t[size];
        varType = VarType::Memory;
        for (long i = 0; i < size; i++)
            data.memory[i] = 0;
    }
}

void Var::ResizeMemory(long size)
{
    if (varType.IsReference())
        data.referedTo->ResizeMemory(size);
    else
    {
        if (size < extraData.dataSize)
        {
            extraData.dataSize = size;
            return;
        }
        else
        {
            uint8_t *newAllocation = new uint8_t[size]();
            long i;
            for (i = 0; i < extraData.dataSize; i++)
                newAllocation[i] = data.memory[i];
            for (; i < size; i++)
                newAllocation[i] = 0;
            RemoveMemory();
            data.memory = newAllocation;
            extraData.dataSize = size;
        }
    }
}

uint8_t & Var::GetMemoryAt(const long &idx)
{
    if (varType.IsReference())
        return data.referedTo->GetMemoryAt(idx);
    else
        return data.memory[idx];
}


//The structure of a "Structure" variable is the same as an "Array". It's a fix-sized "Var" array.
//The content of this array has some differences from an "Array".
//The first element will always be of Integer, which is the index of the structure definition.
//From the second element are the member variables of the structure, in the sequence of member definition.
void Var::InitStructure(const std::vector<StructureDefinition>& sdTable, const int& index)
{
    if (varType.IsReference())
        data.referedTo->InitStructure(sdTable, index);
    else
    {
        int size = static_cast<int>(sdTable[index].varTypes.size());
        data.structure = new Var[size + 1]();
        data.structure[0].SetInteger(index);
        extraData.dataSize = size;
        varType = VarType::Structure;
        for (int i = 0; i < size; i++)
        {
            //Initialize each member's type
            if (sdTable[index].arraySize[i] == 0)
            {
                if (sdTable[index].structMemberIndex[i] >= 0)   //I should initialize a structure
                {
                    if (sdTable[index].varTypes[i].IsReference())
                        data.structure[i + 1].SetVarType(sdTable[index].varTypes[i], sdTable[index].structMemberIndex[i]);
                    else
                        data.structure[i + 1].InitStructure(sdTable, sdTable[index].structMemberIndex[i]);
                }
                else    //common built-in data type
                    data.structure[i + 1].SetVarType(sdTable[index].varTypes[i]);
            }
            else
            {
                //It is an array member
                if (sdTable[index].structMemberIndex[i] >= 0)   //wo! An array of structre
                    data.structure[i + 1].InitArray(sdTable[index].arraySize[i], sdTable, sdTable[index].structMemberIndex[i]);
                else    //common array of built-in data type
                    data.structure[i + 1].InitArray(sdTable[index].arraySize[i], sdTable[index].varTypes[i]);
            }
        }
    }
}

void Var::InitStructure(const int& size, const int& index)
{
    if (varType.IsReference())
        data.referedTo->InitStructure(size, index);
    else
    {
        data.structure = new Var[size + 1]();
        data.structure[0].SetInteger(index);
        extraData.dataSize = size;
        varType = VarType::Structure;
    }
}

Var& Var::GetStructureMember(const long& idx)
{
    if (varType.IsReference())
        return data.referedTo->GetStructureMember(idx);
    else
    {
        if (idx < extraData.dataSize)
            return data.structure[idx + 1];
        else
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
    }
}

Var& Var::GetStructureMember(std::vector<StructureDefinition>& sdTable, wchar_t* name, int len /* = -1*/)
{
    if (varType.IsReference())
        return data.referedTo->GetStructureMember(sdTable, name, len);
    else
    {
        int index = static_cast<int>(data.structure[0].GetInteger());
        int *memberIndex = sdTable[index].memberNames.Find(name, MODULE_IRRELEVANT, len);
        if (memberIndex == nullptr)
            throw Var(Exception::ERROR_CODE::MEMBER_NOT_EXIST);
        return data.structure[*memberIndex + 1];
    }
}

Var* Var::FindStructureMember(std::vector<StructureDefinition>& sdTable, wchar_t* name, int len /* = -1 */)
{
    if (varType.IsReference())
        return data.referedTo->FindStructureMember(sdTable, name, len);
    else
    {
        int index = static_cast<int>(data.structure[0].GetInteger());
        int *memberIndex = sdTable[index].memberNames.Find(name, MODULE_IRRELEVANT, len);
        if (memberIndex == nullptr)
            return nullptr;
        else
            return &data.structure[*memberIndex + 1];
    }
}


const Var& Var::ReadStructureMember(const long& idx) const
{
    if (varType.IsReference())
        return data.referedTo->ReadStructureMember(idx);
    else
    {
        if (idx < extraData.dataSize)
            return data.structure[idx + 1];
        else
            throw Var(Exception::ERROR_CODE::INDEX_OUT_OF_RANGE);
    }
}
