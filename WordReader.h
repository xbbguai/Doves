#pragma once

#include "Exception.h"
#include "HashTable.h"
#include <iostream>
#include <math.h>

//Keyword type
#define KT_STATEMENT        0x1000
#define KT_DATATYPE         0x2000
#define KT_OPERATOR         0x4000
#define KT_BUILTINFUNC      0x8000
#define KT_BOOLEAN_TRUE     0x0100
#define KT_BOOLEAN_FALSE    0x0200

#define BUILTINFUNC_COUNT   50
#define BUILTINSTAMENET_COUNT 50

//Statements index
#define DT_NULL             KT_DATATYPE + 0
#define DT_INTEGER          KT_DATATYPE + 1
#define DT_BOOLEAN          KT_DATATYPE + 2
#define DT_REAL             KT_DATATYPE + 3
#define DT_STRING           KT_DATATYPE + 4
#define DT_CHAR             KT_DATATYPE + 5
#define DT_ARRAY            KT_DATATYPE + 6
#define DT_MEMORY           KT_DATATYPE + 7
#define ST_CONST            KT_STATEMENT + 0
#define ST_PRINT            KT_STATEMENT + 1
#define ST_INPUT            KT_STATEMENT + 2
#define ST_DIM              KT_STATEMENT + 3
#define ST_REDIM            KT_STATEMENT + 4
#define ST_AS               KT_STATEMENT + 5
#define ST_BYREF            KT_STATEMENT + 6
#define ST_IF               KT_STATEMENT + 7
#define ST_THEN             KT_STATEMENT + 8
#define ST_ELSE             KT_STATEMENT + 9
#define ST_ENDIF            KT_STATEMENT + 10
#define ST_FOR              KT_STATEMENT + 11
#define ST_TO               KT_STATEMENT + 12
#define ST_STEP             KT_STATEMENT + 13
#define ST_ENDFOR           KT_STATEMENT + 14
#define ST_WHILE            KT_STATEMENT + 15
#define ST_ENDWHILE         KT_STATEMENT + 16
#define ST_LOOP             KT_STATEMENT + 17
#define ST_ENDLOOP          KT_STATEMENT + 18
#define ST_FOREACH          KT_STATEMENT + 19
#define ST_IN               KT_STATEMENT + 20
#define ST_BREAK            KT_STATEMENT + 21
#define ST_ALL              KT_STATEMENT + 22
#define ST_SELECT           KT_STATEMENT + 23
#define ST_CASE             KT_STATEMENT + 24
#define ST_ENDSELECT        KT_STATEMENT + 25
#define ST_TRY              KT_STATEMENT + 26
#define ST_CATCH            KT_STATEMENT + 27
#define ST_THROW            KT_STATEMENT + 28
#define ST_RETRY            KT_STATEMENT + 29
#define ST_ENDTRY           KT_STATEMENT + 30
#define ST_IMPORT           KT_STATEMENT + 31
#define ST_ON               KT_STATEMENT + 32
#define ST_EXTENDS          KT_STATEMENT + 33
#define ST_REM              KT_STATEMENT + 34
#define ST_STRUCT           KT_STATEMENT + 35
#define ST_ENDSTRUCT        KT_STATEMENT + 36
#define ST_RETURN           KT_STATEMENT + 37
#define ST_FN               KT_STATEMENT + 38
#define ST_ENDFN            KT_STATEMENT + 39
#define ST_END              KT_STATEMENT + 40
#define ST_GLOBAL           KT_STATEMENT + 41
#define ST_LIB              KT_STATEMENT + 42
#define ST_PAUSE            KT_STATEMENT + 43
#define ST_CONT             KT_STATEMENT + 44
#define ST_LIST             KT_STATEMENT + 45
#define ST_RUN              KT_STATEMENT + 46
//Operators index
#define OP_PLUS             KT_OPERATOR + 0
#define OP_MINUS            KT_OPERATOR + 1
#define OP_MULTIPLY         KT_OPERATOR + 2
#define OP_DIVIDE           KT_OPERATOR + 3
#define OP_INT_DIVIDE       KT_OPERATOR + 4
#define OP_MOD              KT_OPERATOR + 5
#define OP_EQUAL            KT_OPERATOR + 6
#define OP_NOTEQUAL         KT_OPERATOR + 7
#define OP_GREATER          KT_OPERATOR + 8
#define OP_LESS             KT_OPERATOR + 9
#define OP_GREATEREQUAL     KT_OPERATOR + 10
#define OP_LESSEQUAL        KT_OPERATOR + 11
#define OP_LEFTBRACKET      KT_OPERATOR + 12
#define OP_RIGHTBRACKET     KT_OPERATOR + 13
#define OP_AND              KT_OPERATOR + 14
#define OP_OR               KT_OPERATOR + 15
#define OP_NOT              KT_OPERATOR + 16
#define OP_XOR              KT_OPERATOR + 17
#define OP_IS               KT_OPERATOR + 18
#define OP_DOT              KT_OPERATOR + 19
#define OP_POWER            KT_OPERATOR + 20
#define OP_COMMA            KT_OPERATOR + 21
#define OP_COLON            KT_OPERATOR + 22
#define OP_LEFTSQBRACKET    KT_OPERATOR + 23
#define OP_RIGHTSQBRACKET   KT_OPERATOR + 24
#define OP_SEMICOLON        KT_OPERATOR + 25
#define OP_REFERTO          KT_OPERATOR + 26
#define OP_AT               KT_OPERATOR + 27

//Keywords
//English version
constexpr struct {const wchar_t* word; int value; } _keywords[] = 
{
    {L"NULL",       DT_NULL},
    {L"INTEGER",    DT_INTEGER},
    {L"BOOLEAN",    DT_BOOLEAN},
    {L"REAL",       DT_REAL},
    {L"STRING",     DT_STRING},
    {L"CHAR",       DT_CHAR},
    {L"ARRAY",      DT_ARRAY},
    {L"MEMORY",     DT_MEMORY},
    {L"CONST",      ST_CONST},
    {L"DIM",        ST_DIM},
    {L"REDIM",      ST_REDIM},
    {L"AS",         ST_AS},
    {L"BYREF",      ST_BYREF},
    {L"FN",         ST_FN},
    {L"ENDFN",      ST_ENDFN},
    {L"RETURN",     ST_RETURN},
    {L"FOR",        ST_FOR},
    {L"TO",         ST_TO},
    {L"STEP",       ST_STEP},
    {L"NEXT",       ST_ENDFOR}, //NEXT and ENDFOR are both parsed as ST_ENDFOR. 'NEXT' is the old Basic style.
    {L"ENDFOR",     ST_ENDFOR},
    {L"WHILE",      ST_WHILE},
    {L"LOOP",       ST_LOOP},
    {L"ENDLOOP",    ST_ENDLOOP},
    {L"WEND",       ST_ENDWHILE},
    {L"ENDWHILE",   ST_ENDWHILE},
    {L"FOREACH",    ST_FOREACH},
    {L"IN",         ST_IN},
    {L"IF",         ST_IF},
    {L"THEN",       ST_THEN},
    {L"ELSE",       ST_ELSE},
    {L"ENDIF",      ST_ENDIF},
    {L"SELECT",     ST_SELECT},
    {L"CASE",       ST_CASE},
    {L"ENDSELECT",  ST_ENDSELECT},
    {L"TRY",        ST_TRY},
    {L"CATCH",      ST_CATCH},
    {L"THROW",      ST_THROW},
    {L"RETRY",      ST_RETRY},
    {L"ENDTRY",     ST_ENDTRY},
    {L"PRINT",      ST_PRINT},
    {L"INPUT",      ST_INPUT},
    {L"REM",        ST_REM},
    {L"BREAK",      ST_BREAK},
    {L"ALL",        ST_ALL},
    //{L"//",         ST_REMLINE1},
    //{L"/*",         ST_REMBLOCKBEGIN},
    //{L"*/",         ST_REMBLOCKEND},
    {L"END",        ST_END},
    {L"STRUCT",     ST_STRUCT},
    {L"CLASS",      ST_STRUCT},     //Class and Struct are the same, the same! Nothing deferent!
    {L"ENDSTRUCT",  ST_ENDSTRUCT},
    {L"ENDCLASS",   ST_ENDSTRUCT},
    {L"EXTENDS",    ST_EXTENDS},
    {L"GLOBAL",     ST_GLOBAL},
    {L"IMPORT",     ST_IMPORT},
    {L"LIB",        ST_LIB},
    {L"PAUSE",      ST_PAUSE},
    {L"CONT",       ST_CONT},
    {L"SOURCE",     ST_LIST},
    {L"RUN",        ST_RUN},

    {L"TRUE",       KT_BOOLEAN_TRUE},
    {L"FALSE",      KT_BOOLEAN_FALSE},

    {L"MOD",        OP_MOD},    //NOTE: Other ops are parsed in the word reader
    {L"AND",        OP_AND},
    {L"OR",         OP_OR},
    {L"NOT",        OP_NOT},
    {L"XOR",        OP_XOR},
    {L"IS",         OP_IS},

    {L"ASC",        KT_BUILTINFUNC + 0},
    {L"CHR",        KT_BUILTINFUNC + 1},
    {L"VAL",        KT_BUILTINFUNC + 2},
    {L"LEN",        KT_BUILTINFUNC + 3},
    {L"TOUPPER",    KT_BUILTINFUNC + 4},
    {L"TOLOWER",    KT_BUILTINFUNC + 5},
    {L"LEFT",       KT_BUILTINFUNC + 6},
    {L"RIGHT",      KT_BUILTINFUNC + 7},
    {L"MID",        KT_BUILTINFUNC + 8},
    {L"FIND",       KT_BUILTINFUNC + 9},
    {L"REPLACE",    KT_BUILTINFUNC + 10},
    {L"MAX",        KT_BUILTINFUNC + 11},
    {L"MIN",        KT_BUILTINFUNC + 12},
    {L"SIN",        KT_BUILTINFUNC + 13},
    {L"COS",        KT_BUILTINFUNC + 14},
    {L"TAN",        KT_BUILTINFUNC + 15},
    {L"ASIN",       KT_BUILTINFUNC + 16},
    {L"ACOS",       KT_BUILTINFUNC + 17},
    {L"ATAN",       KT_BUILTINFUNC + 18},
    {L"LOG",        KT_BUILTINFUNC + 19},
    {L"LOG10",      KT_BUILTINFUNC + 20},
    {L"SQRT",       KT_BUILTINFUNC + 21},
    {L"CEILING",    KT_BUILTINFUNC + 22},
    {L"FLOOR",      KT_BUILTINFUNC + 23},
    {L"ROUND",      KT_BUILTINFUNC + 24},
    {L"RAND",       KT_BUILTINFUNC + 25},
    {L"SEED",       KT_BUILTINFUNC + 26},
    {L"SETW",       KT_BUILTINFUNC + 27},
    {L"SETPRECISION", KT_BUILTINFUNC + 28},
    {L"TAB",        KT_BUILTINFUNC + 29},
    {L"TICKS",      KT_BUILTINFUNC + 30},
    {L"TYPENAME",   KT_BUILTINFUNC + 31},
    {L"ISNUMERIC",  KT_BUILTINFUNC + 32},
    {L"ISSTRING",   KT_BUILTINFUNC + 33},
    {L"ISARRAY",    KT_BUILTINFUNC + 34},
    {L"ISBOOLEAN",  KT_BUILTINFUNC + 35},
    {L"ISNULL",     KT_BUILTINFUNC + 36},
    {L"ISSTRUCTUREOF", KT_BUILTINFUNC + 37},
    {L"SLEEP",      KT_BUILTINFUNC + 38},
    {L"GETREFCOUNT", KT_BUILTINFUNC + 39},
    {L"ARRAYSIZE",  KT_BUILTINFUNC + 40}
};

//Chinese version
constexpr struct {const wchar_t* word; int value; } _c_keywords[] = 
{
    {L"空",         DT_NULL},
    {L"整数",       DT_INTEGER},
    {L"布尔",       DT_BOOLEAN},
    {L"实数",       DT_REAL},
    {L"字符串",     DT_STRING},
    {L"字符",       DT_CHAR},
    {L"数组",       DT_ARRAY},
    {L"内存",       DT_MEMORY},
    {L"常量",       ST_CONST},
    {L"定义",       ST_DIM},
    {L"重定义",     ST_REDIM},
    {L"为",         ST_AS},
    {L"以引用形式",  ST_BYREF},
    {L"函数",        ST_FN},
    {L"函数结束",    ST_ENDFN},
    {L"返回",       ST_RETURN},
    {L"从",         ST_FOR},
    {L"到",         ST_TO},
    {L"步进",       ST_STEP},
    {L"下一个",     ST_ENDFOR}, //'下一个' and '从结束' are both parsed as ST_ENDFOR.
    {L"从结束",     ST_ENDFOR},
    {L"当",         ST_WHILE},
    {L"循环",       ST_LOOP},
    {L"循环结束",    ST_ENDLOOP},
    {L"当结束",      ST_ENDWHILE},
    {L"从每个",      ST_FOREACH},
    {L"取自",       ST_IN},
    {L"如果",       ST_IF},
    {L"那么",       ST_THEN},
    {L"否则",       ST_ELSE},
    {L"如果结束",    ST_ENDIF},
    {L"选择",       ST_SELECT},
    {L"条件",       ST_CASE},
    {L"选择结束",   ST_ENDSELECT},
    {L"尝试",       ST_TRY},
    {L"捕捉异常",     ST_CATCH},
    {L"抛出异常",     ST_THROW},
    {L"重试",      ST_RETRY},
    {L"尝试结束",     ST_ENDTRY},
    {L"打印",      ST_PRINT},
    {L"输入",      ST_INPUT},
    {L"注释",        ST_REM},
    {L"断出",      ST_BREAK},
    {L"全部",        ST_ALL},
    //{L"//",         ST_REMLINE1},
    //{L"/*",         ST_REMBLOCKBEGIN},
    //{L"*/",         ST_REMBLOCKEND},
    {L"结束",        ST_END},
    {L"结构",     ST_STRUCT},
    {L"类",       ST_STRUCT},
    {L"结构结束",  ST_ENDSTRUCT},
    {L"类结束",   ST_ENDSTRUCT},
    {L"扩展自",    ST_EXTENDS},
    {L"全局",     ST_GLOBAL},
    {L"导入",     ST_IMPORT},
    {L"动态库",    ST_LIB},
    {L"暂停",      ST_PAUSE},
    {L"继续",       ST_CONT},
    {L"列程序",       ST_LIST},
    {L"运行",        ST_RUN},

    {L"真",       KT_BOOLEAN_TRUE},
    {L"假",      KT_BOOLEAN_FALSE},

    {L"模",        OP_MOD},    //NOTE: Other ops are parsed in the word reader
    {L"与",        OP_AND},
    {L"或",        OP_OR},
    {L"非",        OP_NOT},
    {L"异或",      OP_XOR},
    {L"是",        OP_IS},

    {L"取内码",        KT_BUILTINFUNC + 0},
    {L"转字符",        KT_BUILTINFUNC + 1},      
    {L"求值",        KT_BUILTINFUNC + 2},
    {L"长度",        KT_BUILTINFUNC + 3},
    {L"转大写",    KT_BUILTINFUNC + 4},
    {L"转小写",    KT_BUILTINFUNC + 5},
    {L"取左",       KT_BUILTINFUNC + 6},
    {L"取右",      KT_BUILTINFUNC + 7},
    {L"取中间",        KT_BUILTINFUNC + 8},
    {L"查找",       KT_BUILTINFUNC + 9},
    {L"替换",    KT_BUILTINFUNC + 10},
    {L"最大值",        KT_BUILTINFUNC + 11},
    {L"最小值",        KT_BUILTINFUNC + 12},
    {L"SIN",        KT_BUILTINFUNC + 13},
    {L"COS",        KT_BUILTINFUNC + 14},
    {L"TAN",        KT_BUILTINFUNC + 15},
    {L"ASIN",       KT_BUILTINFUNC + 16},
    {L"ACOS",       KT_BUILTINFUNC + 17},
    {L"ATAN",       KT_BUILTINFUNC + 18},
    {L"LOG",        KT_BUILTINFUNC + 19},
    {L"LOG10",      KT_BUILTINFUNC + 20},
    {L"开平方",      KT_BUILTINFUNC + 21},
    {L"整数上顶",    KT_BUILTINFUNC + 22},
    {L"整数下底",    KT_BUILTINFUNC + 23},
    {L"取整",      KT_BUILTINFUNC + 24},
    {L"随机数",       KT_BUILTINFUNC + 25},
    {L"随机数播种",       KT_BUILTINFUNC + 26},
    {L"设宽",       KT_BUILTINFUNC + 27},
    {L"设精度", KT_BUILTINFUNC + 28},
    {L"制表位",        KT_BUILTINFUNC + 29},
    {L"滴答数",      KT_BUILTINFUNC + 30},
    {L"类型名",   KT_BUILTINFUNC + 31},
    {L"是否数值",  KT_BUILTINFUNC + 32},
    {L"是否字符串",   KT_BUILTINFUNC + 33},
    {L"是否数组",    KT_BUILTINFUNC + 34},
    {L"是否布尔",  KT_BUILTINFUNC + 35},
    {L"是否空",     KT_BUILTINFUNC + 36},
    {L"是结构的", KT_BUILTINFUNC + 37},
    {L"休眠",      KT_BUILTINFUNC + 38},
    {L"取引用计数", KT_BUILTINFUNC + 39},
    {L"数组大小",  KT_BUILTINFUNC + 40}
};

class WordReader
{
public:
    enum class WordType
    {
        Boolean,
        Integer,
        Double,
        Character,
        String,
        Keyword,
        Identifier,
        EndOfLine,
        EndOfFile
    };

    //Keyword table is public to all word reader objects.
    //And, surely it needs only to be initialized once.
    //This is static.
    static HashTable<int, 128> keywords;
public:
    //Source code and current reading position.
    wchar_t *srcCode { nullptr };
    size_t lenSrc { 0 };
    size_t pos { 0 };
    size_t posNext { 0 };

    //Atrributes of current word read
    WordType wordType;
    wchar_t *strWord; // The word is a string or identifier
    int lenString;    // For string or identifier: the length of strWord.
    double fWord;     // The word is a double
    long iWord;       // The word is an integer
    bool bWord;       // The word is a boolean
    wchar_t cWord;    // The word is a character
    int keywordIdx;   // The word is a keyword
public:
    void PrepareReader();
    void ReadInOneWord();
    void ParseLiteralNumber();

    void MoveToNextLine(); // This will check syntax. There should be nothing left in the rest of this line, or syntax error will be thrown.
    void SkipToNextLine(); // This doesn't check syntax, simply move to the head of next line.

public:
    WordReader(bool chn = false)
    {
        if (keywords.GetSize() == 0)
        {
            //If keyword hash table has not been built, build up the keyword hash table.
            if (chn)
            {
                for (auto& item : _c_keywords)
                    keywords.Insert(item.word, MODULE_IRRELEVANT, item.value);  //Note: moduleIdx is not used for keywords. just assign 0.
            }
            else
            {
                for (auto& item : _keywords)
                    keywords.Insert(item.word, MODULE_IRRELEVANT, item.value);  //Note: moduleIdx is not used for keywords. just assign 0.
            }
        }
    }

    WordReader(wchar_t *srcCode, const size_t pos = 0, const size_t len = -1) : WordReader()
    {
        SetupReader(srcCode, pos, len);
    }

    ~WordReader()
    {   
    }

    void SetupReader(wchar_t *srcCode, const size_t pos = 0, const size_t len = -1)
    {
        this->srcCode = srcCode;
        this->pos = this->posNext = pos;
        if (len == -1)
        {
            lenSrc = 0;
            while (srcCode[lenSrc++] != 0);
        }
        else
            lenSrc = len;
    }

    void MovePosition(const size_t pos)
    {
        this->pos = this->posNext = pos;
    }

    std::pair<int, int> LineFromPosition(const size_t pos);
    std::pair<int, int> LineFromPosition();

    inline bool IsKeywordOf(int keywordIdx)
    {
        return wordType == WordReader::WordType::Keyword && this->keywordIdx == keywordIdx;
    }
    inline bool IsEndOfStatement()
    {
        return wordType == WordReader::WordType::EndOfLine || wordType == WordReader::WordType::EndOfFile;
    }
};
