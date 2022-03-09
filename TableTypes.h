#pragma once

#include "WordReader.h"
#include <vector>

class VarType
{
private:
    int type;
public:
    static constexpr int NotAType = -1;
    static constexpr int ReferenceFlag = 0x8000;
    static constexpr int ConstFlag     = 0x4000;
    static constexpr int NumericFlag   = 0x2000;
    static constexpr int IndexableFlag = 0x1000;
    static constexpr int Null        = 0;
    static constexpr int Integer     = 1 + NumericFlag;
    static constexpr int Boolean     = 2 + NumericFlag;
    static constexpr int Real        = 3 + NumericFlag;
    static constexpr int Char        = 4 + NumericFlag;
    static constexpr int String      = 6;
    static constexpr int Array       = 7 + IndexableFlag;
    static constexpr int Memory      = 8 + IndexableFlag;
    static constexpr int Structure   = 9 + IndexableFlag;
    static constexpr int InternalLoopQueue = 0x80;  //Internal use only. For loops.
    static constexpr int InternalException = 0x81;  //Internal use only. When error occurs, throw this type of Var object with integer value = error code.
    static constexpr int InternalModuleIdx = 0x82;  //Internal use only. Save the module index of a module. Only stored in global variable table.
    static constexpr int InternalPointer   = 0x83;  //Internal use only. For the built-in struct Memory to allocate memory.
    static constexpr int InternalDLL       = 0x84;  //Internal use only. Save the dll handle of a loaded dll. Only stored in global variable table.
    static constexpr int InternalThreadObj = 0x85;  //Internal use only. Save the thread object's pointer.

    static inline int ConvertWordReaderDataType(int dt)
    {
        switch (dt)
        {
        case DT_INTEGER:
            return Integer;
        case DT_BOOLEAN:
            return Boolean;
        case DT_REAL:
            return Real;
        case DT_CHAR:
            return Char;
        case DT_STRING:
            return String;
        case DT_ARRAY:
            return Array;
        case DT_MEMORY:
            return Memory;
        case DT_NULL:
            return Null;
        default:
            break;
        }
        return NotAType;
    }

    VarType() { type = Null; }
    VarType(const VarType& varType) { type = varType.type; }
    VarType(const int& varType) { type = varType; }
    
    inline VarType& operator=(const int& varType)
    {
        type = varType;
        return *this;
    }
    
    inline operator int() const { return type; }
    inline bool operator==(const int& varType) const { return type == varType; }    //The two operands should be exactly equal to return true.
    inline bool IsCompatible(const int& varType) const  //Returns true if varType is compatible with me.
    {
        if (IsNumeric() && VarType(varType).IsNumeric())
            return true;    //numbers are compatible.
        else if ((varType & 0x3fff) == (type & 0x3fff))
            return true;
        else if ((type & 0x3fff) == VarType::Null || (varType & 0x3fff) == VarType::Null)
            return true;
        else if ((type & 0x3fff) == VarType::Structure && (varType & 0x3fff) == VarType::Array)
            return true;
        else
            return false;
    }
    inline bool IsNumeric() const { return (type & NumericFlag) > 0; }
    inline bool IsIndexable() const { return (type & IndexableFlag) > 0; }
    inline bool IsReference() const { return (type & ReferenceFlag) > 0; }
    inline bool IsConstant() const { return (type & ConstFlag) > 0; }
    inline bool IsStructure() const { return (type & 0x3fff) == VarType::Structure; }
    inline bool IsRedimable() const { return (type & 0x3fff) == VarType::Array; } // || (type & 0x3fff) == VarType::Memory; }
    inline void SetReference() { type |= ReferenceFlag; }
};

class StructureDefinition
{
public:
    HashTable<int, 16, 64> memberNames; //Name of the members, map to indices of vector varTypes
    std::vector<VarType> varTypes;      //Member's variable type, in the sequence of each member.
    std::vector<int> structMemberIndex; //If the member is a structure, this is used to store the index of the structure definition table.
    std::vector<long> arraySize;        //If the member is an array, this is used to store the size of the array
    int parentStructIndex {-1};         //If this structure has a parent, set to the parent structure index. Otherwise is default to -1.
    int moduleIdx {0};                  //In which module this structure is defined.
};


class Var;
class Parser;
typedef Var(*InternalStructFunc)(HashTable<Var>&, Parser&);

class FunctionDefinition
{
public:
    HashTable<int, 16, 64> paramIndexes;    //For member functions of a structure, the function name will be mapped as FunctionName{Index}.
    std::vector<VarType> paramTypes;        //Types of each parameter.
    std::vector<int> structParamIndex;      //If the parameter is a structure, this is used to store the index of the structure definition table.
    VarType returnType;                     //Return type.
    int returnStructIndex { -1 };           //If it returns a structure, this is used to store the index of the structure definition table.
    size_t sourceCodePosition {0};          //The start position of this function in the source code.
    int moduleIdx {0};                      //In which module this function is defined.
    InternalStructFunc internalStructFunc {nullptr};
};