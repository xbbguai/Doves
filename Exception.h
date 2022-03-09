#pragma once

#include <exception>
#include <string>

class Exception : public std::exception
{
public:
    enum class ERROR_CODE
    {
        SYNTAX_ERROR,               //0 Syntax error.
        INVALID_IDENTIFIER,         //1 Invalid variable name.
        CONSTANT_NOT_MUTABLE,       //2 Constant can be assigned for one time.
        UNSUPPORTED_REFERENCE,      //3 Unsupported reference.
        CANNOT_CONVERT_DATATYPE,    //4 Cannot convert the data desired type.
        VALUE_OUT_OF_RANGE,         //5 Value out of range.
        EOL_EXPECTED,               //6 There should be end of line at current position.
        SINGLE_QUOTATION_MARK_EXPECTED,  //7 There should be a single quotation mark.
        QUOTATION_MARK_EXPECTED,    //8 There should be a quotation mark.
        EXPONENT_EXPECTED,          //9 There should be an exponent part of a floating point literal
        PARAM_EXPECTED,             //10 Parameters are not all specified when calling a function.
        TYPE_MISMATCH,              //11 Types not match each other when comparing two variables.
        DIVIDE_BY_ZEOR,             //12 Divided by zero.
        RIGHT_BRACE_EXPECTED,       //13 Right brace expected
        IDENTIFIER_NOT_DEFINED,     //14 Identifier not defined.
        LEFT_BRACE_EXPECTED,        //15 Function or method should start by a left brace
        TOO_MANY_PARAMS,            //16 Function param count should be less than 100.
        INDEX_OUT_OF_RANGE,         //17 Index out of range when visiting an element of an array, structure, tuple, etc.
        RIGHT_SQBRACKET_EXPECTED,   //18 There should be a right square bracket when visiting an element of an array 
        MEMBER_NOT_EXIST,           //19 Try to visit a non-exist member of a structure.
        CANNOT_REDEFINE_FUNC,       //20 Function definition should be unique.
        UNKNOW_DATATYPE,            //21 Unknown data type while trying to convert a WordReader datatype to VarType
        ONE_STATEMENT_ONLY,         //22 There should be only one statement in one line.
        CANNOT_OPEN_SOURCE_CODE,    //23 Unable to open the source code file and read.
        PARAM_NAME_SHOULD_BE_UNIQUE,//24 Parameter names of a function should be unique.
        CANNOT_REDEFINE_STRUCTURE,  //25 Structure definition should be unique.
        PARAM_NOT_DEFINED,          //26 The parameter name used here is not defined in the function body.
        NONE_PRINTABLE_DATA,        //27 Try to print something that is not printable.
        NONE_INPUTABLE_TYPE,        //28 Try to input to a variable type that is not inputable.
        INPUT_CHAR_REQUIRED,        //29 Input statement requires a character.
        INPUT_INTEGER_REQUIRED,     //30 Input statement requires an integer.
        INPUT_BOOLEAN_REQUIRED,     //31 Input statement requires a boolean (T, true, 1 / F, false, 0)
        VAR_REDFINED,               //32 The variable has defined before.
        ARRAY_SIZE_REQUIRED,        //33 There should be an integer value to define the size of an array.
        UNDEFINED_TYPE,             //34 Try to declare a variable as an undefined type.
        CANNOT_REFER_TO_DIFFERENT_TYPE,  //35 Try to set a variable as a reference to another variable but the types do not match.
        RVALUE_NOT_REFERENCABLE,    //36 Cannot refer to an expression result that is not a variable.
        USING_NOT_INITIALIZED_REF,  //37 Using a reference variable that is not initialized.
        MUST_BE_REFERENCE,          //38 A structure member of the same type of the structure itself should always be a reference. Add 'BYREF'.
        FIXED_ARRAY_CANNOT_BE_REF,  //39 An array itself can not refer to another array.
        NOT_A_REF_VAR,              //40 Only a reference variable can refer to another variable.
        IF_WITHOUT_ENDIF,           //41 No ENDIF nor ELSE found in the IF statement.
        FOR_WITHOUT_INDEX_VAR,      //42 'FOR' should followed by a variable assignment to be used as loop controlling variable.
        FOR_WITHOUT_TO,             //43 'FOR' without 'TO'
        NEXT_WITHOUT_FOR,           //44 'NEXT/ENDFOR' without 'FOR'
        DIFF_ST_CANNOT_CROSS,       //45 Try to go across different statement structures.
        STEP_IS_ZERO,               //46 Step cannot be 0.
        ENDWHILE_WITHOUT_WHILE,     //47 'ENDWHILE/WEND' without 'WHILE'.
        FOREACH_WITHOUT_IN,         //48 'FOREACH' without 'IN'.
        FOREACH_WITHOUT_VAR,        //49 'FOREACH' should be followed by a variable.
        NOT_INDEXABLE,              //50 The variable used or to be iterated is not indexable nor iteratable.
        ENDFOREACH_WITHOUT_FOREACH, //51 'ENDFOREACH' without 'FOREACH'
        SELECT_WITHOU_CASE,         //52 'SELECT' must be followed by 'CASE'
        CASE_WITHOUT_SELECT,        //53 There should be a 'SELECT CASE' statement before this 'CASE'.
        UNSUPPROTE_TYPE_IN_CASE,    //54 Only numerics and strings are support in 'select case'.
        NO_TEMP_VAR_AVLB,           //55 No room to create a temp variable. Temp vars exceed 999.
        ENDSELECT_WITHOUT_SELECT,   //56 'ENDSELECT' without 'SELECT'.
        ENDTRY_WITHOUT_TRY,         //57 'ENDTRY' or 'CATCH' without 'TRY'.
        THROW_NOT_CATCHED,          //58 Error thrown but not catched in any catch block.
        ENDLOOP_WITHOUT_LOOP,       //59 'ENDLOOP' without 'LOOP'
        BREAK_WITHOUT_LOOP,         //60 'BREAK' should only be inside a loop.
        RETRY_NOT_IN_CATCH,         //61 'RETRY' should only be inside a CATCH block.
        MEMBER_DEFINED_IN_PARENT,   //62 Member has been defined in one of the parents.
        NOT_REDIMABLE,              //63 Only an array or memory area can be re-dimmed.
        IMPORT_NAME_USED,           //64 Try to import a module as a used name.
        IDENTIFIER_TOO_LONG,        //65 An identifier should not exceed 53 charactors.
        MEMORY_SHOULD_BE_ARRAY,     //66 Memory can only be defined as an array.
        FAIL_LOADING_LIB,           //67 Fail loading the library.
        SOURCE_FORMAT_NOT_SUPPORTED,//68 Source code should be encoded with UTF-8.
        DIM_GLOBAL_MISMATCH,        //69 Same global variable DIMed in two places where types do not match.
        ONE_DIM_FOR_REF_ONLY        //70 When using DIM to allocate memory for a BYREF var, only one dimesion is allowed.
    };

    static const std::string GetErrorMessage(ERROR_CODE errCode)
    {
        switch (errCode)
        {
        case ERROR_CODE::SYNTAX_ERROR:               
            return "Syntax error.";
        case ERROR_CODE::INVALID_IDENTIFIER:         
            return "Invalid variable name";
        case ERROR_CODE::CONSTANT_NOT_MUTABLE:       
            return "Constant cannot be re-assigned.";
        case ERROR_CODE::UNSUPPORTED_REFERENCE:      
            return "Unsupported reference.";
        case ERROR_CODE::CANNOT_CONVERT_DATATYPE:    
            return "Cannot convert the data to desired type.";
        case ERROR_CODE::VALUE_OUT_OF_RANGE:         
            return "Value out of range.";
        case ERROR_CODE::EOL_EXPECTED:               
            return "There should be end of line at current position.";
        case ERROR_CODE::SINGLE_QUOTATION_MARK_EXPECTED:  
            return "There should be a single quotation mark.";
        case ERROR_CODE::QUOTATION_MARK_EXPECTED:    
            return "There should be a quotation mark.";
        case ERROR_CODE::EXPONENT_EXPECTED:          
            return "There should be an exponent part of a floating point literal.";
        case ERROR_CODE::PARAM_EXPECTED:             
            return "Parameters are not all specified when calling a function.";
        case ERROR_CODE::TYPE_MISMATCH:              
            return "Type does not match when assigning/returning a variable or comparing two variables.";
        case ERROR_CODE::DIVIDE_BY_ZEOR:             
            return "Divided by zero.";
        case ERROR_CODE::RIGHT_BRACE_EXPECTED:       
            return "Right brace expected.";
        case ERROR_CODE::IDENTIFIER_NOT_DEFINED:     
            return "Identifier not defined.";
        case ERROR_CODE::LEFT_BRACE_EXPECTED:        
            return "Function or method should start by a left bracket.";
        case ERROR_CODE::TOO_MANY_PARAMS:            
            return "Function param count should be less than 100.";
        case ERROR_CODE::INDEX_OUT_OF_RANGE:         
            return "Index out of range when visiting an element of an array, structure, tuple, etc.";
        case ERROR_CODE::RIGHT_SQBRACKET_EXPECTED:   
            return "There should be a right square bracket when visiting an element of an array.";
        case ERROR_CODE::MEMBER_NOT_EXIST:           
            return "Try to visit a non-exist member of a structure.";
        case ERROR_CODE::CANNOT_REDEFINE_FUNC:       
            return "Function definition should be unique.";
        case ERROR_CODE::UNKNOW_DATATYPE:            
            return "Unknown data type while trying to convert a WordReader datatype to VarType.";
        case ERROR_CODE::ONE_STATEMENT_ONLY:         
            return "There should be only one statement in one line.";
        case ERROR_CODE::CANNOT_OPEN_SOURCE_CODE:    
            return "Unable to open the source code file and read.";
        case ERROR_CODE::PARAM_NAME_SHOULD_BE_UNIQUE:
            return "Parameter names of a function should be unique.";
        case ERROR_CODE::CANNOT_REDEFINE_STRUCTURE:   
            return "Structure definition should be unique.";
        case ERROR_CODE::PARAM_NOT_DEFINED:
            return "The parameter name used here is not defined in the function body.";
        case ERROR_CODE::NONE_PRINTABLE_DATA:
            return "Try to print something that is not printable.";
        case ERROR_CODE::NONE_INPUTABLE_TYPE:
            return "Try to input to a variable type that is not inputable.";
        case ERROR_CODE::INPUT_CHAR_REQUIRED:
            return "Input statement requires a character.";
        case ERROR_CODE::INPUT_INTEGER_REQUIRED:
            return "Input statement requires an integer.";
        case ERROR_CODE::INPUT_BOOLEAN_REQUIRED:
            return "Input statement requires a boolean (T, true, 1 / F, false, 0).";
        case ERROR_CODE::VAR_REDFINED:
            return "The variable has defined before.";
        case ERROR_CODE::ARRAY_SIZE_REQUIRED:
            return "There should be an integer value to define the size of an array.";
        case ERROR_CODE::UNDEFINED_TYPE:
            return "Try to declare a variable as an undefined type.";
        case ERROR_CODE::CANNOT_REFER_TO_DIFFERENT_TYPE:
            return "Try to set a variable as a reference to another variable but the types do not match.";
        case ERROR_CODE::RVALUE_NOT_REFERENCABLE:
            return "Cannot refer to an expression result that is not a variable.";
        case ERROR_CODE::USING_NOT_INITIALIZED_REF:
            return "Using a reference variable that is not initialized.";
        case ERROR_CODE::MUST_BE_REFERENCE:
            return "A structure member of the same type of the structure itself should always be a reference. Add 'BYREF'.";
        case ERROR_CODE::FIXED_ARRAY_CANNOT_BE_REF:
            return "An array itself can not refer to another array.";
        case ERROR_CODE::NOT_A_REF_VAR:
            return "Only a reference variable can refer to another variable.";
        case ERROR_CODE::IF_WITHOUT_ENDIF:
            return "No ENDIF nor ELSE found in the IF statement.";
        case ERROR_CODE::FOR_WITHOUT_INDEX_VAR:
            return "'FOR' should followed by a variable assignment to be used as loop controlling variable.";
        case ERROR_CODE::FOR_WITHOUT_TO:
            return "'FOR' without 'TO'.";
        case ERROR_CODE::NEXT_WITHOUT_FOR:
            return "'NEXT/ENDFOR' without 'FOR'";
        case ERROR_CODE::DIFF_ST_CANNOT_CROSS:
            return "Try to go across different statement structures.";
        case ERROR_CODE::STEP_IS_ZERO:
            return "Step cannot be 0.";
        case ERROR_CODE::ENDWHILE_WITHOUT_WHILE:
            return "'ENDWHILE/WEND' without 'WHILE'.";
        case ERROR_CODE::FOREACH_WITHOUT_IN:
            return "'FOREACH' without 'IN'.";
        case ERROR_CODE::FOREACH_WITHOUT_VAR:
            return "'FOREACH' should be followed by a variable.";
        case ERROR_CODE::NOT_INDEXABLE:
            return "The variable used or to be iterated is not indexable nor iteratable.";
        case ERROR_CODE::ENDFOREACH_WITHOUT_FOREACH:
            return "'ENDFOREACH' without 'FOREACH'";
        case ERROR_CODE::SELECT_WITHOU_CASE:
            return "'SELECT' must be followed by 'CASE'.";
        case ERROR_CODE::CASE_WITHOUT_SELECT:
            return "There should be a 'SELECT CASE' statement before this 'CASE'.";
        case ERROR_CODE::UNSUPPROTE_TYPE_IN_CASE:
            return "Only numerics and strings are support in 'select case'.";
        case ERROR_CODE::NO_TEMP_VAR_AVLB:
            return "No room to create a temp variable. Temp vars exceed 999.";
        case ERROR_CODE::ENDSELECT_WITHOUT_SELECT:
            return "'ENDSELECT' without 'SELECT'.";
        case ERROR_CODE::ENDTRY_WITHOUT_TRY:
            return "'ENDTRY' or 'CATCH' without 'TRY'.";
        case ERROR_CODE::THROW_NOT_CATCHED:
            return "Error thrown but not catched in any catch block.";
        case ERROR_CODE::ENDLOOP_WITHOUT_LOOP:
            return "'ENDLOOP' without 'LOOP'.";
        case ERROR_CODE::BREAK_WITHOUT_LOOP:
            return "'BREAK' should only be inside a loop.";
        case ERROR_CODE::RETRY_NOT_IN_CATCH:
            return "'RETRY' should only be inside a CATCH block.";
        case ERROR_CODE::MEMBER_DEFINED_IN_PARENT:
            return "Member has been defined in one of the parents.";
        case ERROR_CODE::NOT_REDIMABLE:
            return "Only an array or memory area can be re-dimmed.";
        case ERROR_CODE::IMPORT_NAME_USED:
            return "Try to import a module as a used name.";
        case ERROR_CODE::IDENTIFIER_TOO_LONG:
            return "An identifier should not exceed 53 charactors.";
        case ERROR_CODE::MEMORY_SHOULD_BE_ARRAY:
            return "Memory can only be defined as an array.";
        case ERROR_CODE::FAIL_LOADING_LIB:
            return "Fail loading the library.";
        case ERROR_CODE::SOURCE_FORMAT_NOT_SUPPORTED:
            return "Source code should be encoded with UTF-8.";
        case ERROR_CODE::DIM_GLOBAL_MISMATCH:
            return "Same global variable DIMed in two places where types do not match.";
        case ERROR_CODE::ONE_DIM_FOR_REF_ONLY:
            return "When using DIM to allocate memory for a BYREF var, only one dimesion is allowed.";
        }
        return "Internal error.";
    }
};