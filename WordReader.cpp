#include "WordReader.h"
#include "var.h"

HashTable<int, 128> WordReader::keywords;

void WordReader::PrepareReader()
{
    pos = 0;
    posNext = 0;
}

void WordReader::MoveToNextLine()
{
    if (wordType == WordType::EndOfLine || wordType == WordType::EndOfFile)
    {
        pos = posNext;
        return;
    }
    while (posNext < lenSrc)
    {
        if (srcCode[posNext] == L' ' || srcCode[posNext] == L'\t')
            posNext++;
        else if (srcCode[posNext] == 0x0d && posNext + 1 < lenSrc && srcCode[posNext + 1] == 0x0a) // Windows EOL
        {
            posNext += 2;
            break;
        }
        else if (srcCode[posNext] == 0x0a) // UNIX EOL
        {
            posNext++;
            break;
        }
        else if (srcCode[posNext] == 0) //End of file
        {
            posNext++;
            break;
        }
        else
            throw Var(Exception::ERROR_CODE::EOL_EXPECTED);
    }
    pos = posNext; // Now pos and posNext are both at the beginning of the new line.
}

void WordReader::SkipToNextLine()
{
    if (wordType == WordType::EndOfLine)
    {
        pos = posNext;
        return;
    }
    while (posNext < lenSrc)
    {
        if (srcCode[posNext] == 0x0d && posNext + 1 < lenSrc && srcCode[posNext + 1] == 0x0a) // Windows EOL
        {
            posNext += 2;
            break;
        }
        else if (srcCode[posNext] == 0x0a) // UNIX EOL
        {
            posNext++;
            break;
        }
        else
            posNext++;
    }
    pos = posNext; // Now pos and posNext are both at the beginning of the new line.
}

std::pair<int, int> WordReader::LineFromPosition()
{
    return LineFromPosition(pos);
}

std::pair<int, int> WordReader::LineFromPosition(const size_t pos)
{
    int line = 1;
    int column = 1;
    size_t i = 0;
    while (i < pos)
    {
        if (srcCode[i] == 0x0d)
        {
            line++;
            if (i < pos && srcCode[i + 1] == 0x0a)  //Avoid Windows \r\n
                i++;
            column = 1;
        }
        else if (srcCode[i] == 0x0a)
        {
            line++;
            column = 1;
        }
        i++;
        column++;
    }

    return std::pair<int, int>(line, column);
}

void WordReader::ParseLiteralNumber()
{
    long long nDotPosition = 0;
    long long iPart1 = 0;
    int ePart = 0;
    while (posNext < lenSrc)
    {
        wchar_t &ch = srcCode[posNext];
        if (ch >= L'0' && ch <= L'9')
        {
            iPart1 *= 10;
            iPart1 += (ch - L'0');
            nDotPosition *= 10;
        }
        else if (ch == L'.')
        {
            if (nDotPosition > 0)
                throw Var(Exception::ERROR_CODE::SYNTAX_ERROR);
            else
                nDotPosition = 1;
        }
        else
            break;
        posNext++;
    }

    int eSigned = 1;
    if (posNext < lenSrc && (srcCode[posNext] == L'E' || srcCode[posNext] == L'e'))
    {
        posNext++;
        if (posNext >= lenSrc)
            throw Var(Exception::ERROR_CODE::EXPONENT_EXPECTED);
        if (srcCode[posNext] == L'+' || srcCode[posNext] == L'-')
        {
            if (srcCode[posNext] == L'-')
                eSigned = -1;
            posNext++;
            if (posNext >= lenSrc)
                throw Var(Exception::ERROR_CODE::EXPONENT_EXPECTED);
        }
        while (posNext < lenSrc && (srcCode[posNext] >= L'0' && srcCode[posNext] <= L'9'))
        {
            ePart *= 10;
            ePart += (srcCode[posNext] - L'0');
            posNext++;
        }
        ePart *= eSigned;
    }

    if (nDotPosition || ePart != 0)
    {
        //Real
        fWord = static_cast<double>(iPart1) / (nDotPosition > 0 ? nDotPosition : 1) * (ePart == 0 ? 1 : pow(10, ePart));
        wordType = WordType::Double;
    }
    else
    {
        //Integer
        wordType = WordType::Integer;
        iWord = static_cast<long>(iPart1);
    }
}

//Very important:
//EOL, EOF are considered to be word types.
void WordReader::ReadInOneWord()
{
    // Skip all leading blanks
    while ((srcCode[posNext] == L' ' || srcCode[posNext] == L'\t') && posNext < lenSrc)
        posNext++;
    // Skip remarks
    while (srcCode[posNext] == L'/' && posNext < lenSrc && srcCode[posNext + 1] == L'*')
    {
        //Skip to the next '*/'
        posNext += 2;
        while (posNext < lenSrc && (srcCode[posNext] != L'*' || srcCode[posNext + 1] != L'/'))
            posNext++;
        posNext += 2;
        //Skip blanks
        while ((srcCode[posNext] == L' ' || srcCode[posNext] == L'\t') && posNext < lenSrc)
            posNext++;
    }
    
    //Ignore single line remark
    if (srcCode[posNext] == L'/' && posNext < lenSrc && srcCode[posNext + 1] == L'/')
    {
        //The following content is remark. Move to the end of the line.
        posNext += 2;
        while (posNext < lenSrc && srcCode[posNext] != 0x0d && srcCode[posNext] != 0x0a)
            posNext++;
    }

    pos = posNext;
    if (pos >= lenSrc || srcCode[pos] == 0)
    {
        // End of file.
        wordType = WordType::EndOfFile;
        return;
    }

    // Now, pos is the start position of this word.
    // Determine the attribute of this word.
    if (srcCode[posNext] == L'\'')
    {
        // It's a character
        posNext++;
        cWord = srcCode[posNext];
        posNext++;
        if (srcCode[posNext] == L'\'')
        {
            posNext++;
            wordType = WordType::Character;
            return;
        }
        else
            throw Var(Exception::ERROR_CODE::SINGLE_QUOTATION_MARK_EXPECTED);
    }
    else if (srcCode[posNext] == L'"')
    {
        // It's a literal string
        pos = ++posNext;
        // Get the next quotation mark.
        while (srcCode[posNext] != 0x0d && srcCode[posNext] != 0x0a && posNext < lenSrc)
        {
            if (srcCode[posNext] == L'"')
            {
                // Good!
                wordType = WordType::String;
                strWord = srcCode + pos;
                lenString = static_cast<int>(posNext - pos);
                posNext++;
                return;
            }
            posNext++;
        }
        // End of line or end of file encountered. This is an error.
        throw Var(Exception::ERROR_CODE::QUOTATION_MARK_EXPECTED);
    }
    else if (srcCode[posNext] == 0x0d || srcCode[posNext] == 0x0a)
    {
        // It's eol
        while ((srcCode[posNext] == 0x0d || srcCode[posNext] == 0x0a) && posNext < lenSrc)
            posNext++;
        if (posNext == lenSrc)
            wordType = WordType::EndOfFile;
        else
            wordType = WordType::EndOfLine;
        return;
    }
    else if (srcCode[posNext] >= L'0' && srcCode[posNext] <= L'9')
    {
        // It's a literal number
        ParseLiteralNumber();
        return;
    }
    else
    {
        // Keyword, operator or idenfier.
        // Note: before "true" and "false" are transformed to a boolean value, they are considered to be keywords.
        pos = posNext;
        while ((srcCode[posNext] >= L'A' && srcCode[posNext] <= L'Z' || srcCode[posNext] >= L'a' && srcCode[posNext] <= L'z' 
            || srcCode[posNext] >= L'0' && srcCode[posNext] <= L'9' || srcCode[posNext] == L'_' || srcCode[posNext] > 0xff) && posNext < lenSrc)
        {
            posNext++;
        }
        //Most operators should be handled seperately because they don't always have a clear boundary.
        int result { -1 };
        if (pos == posNext && srcCode[pos] <= 0xff)
        {
            switch (srcCode[posNext])
            {
            case L'+':
                result = OP_PLUS;
                posNext++;
                break;
            case L'-':
                result = OP_MINUS;
                posNext++;
                break;
            case L'=':
                result = OP_EQUAL;
                posNext++;
                break;
            case L'\\':
                result = OP_INT_DIVIDE;
                posNext++;
                break;
            case L'^':
                result = OP_POWER;
                posNext++;
                break;
            case L'.':
                result = OP_DOT;
                posNext++;
                break;
            case L'(':
                result = OP_LEFTBRACKET;
                posNext++;
                break;
            case L')':
                result = OP_RIGHTBRACKET;
                posNext++;
                break;
            case L'[':
                result = OP_LEFTSQBRACKET;
                posNext++;
                break;
            case L']':
                result = OP_RIGHTSQBRACKET;
                posNext++;
                break;
            case L',':
                result = OP_COMMA;
                posNext++;
                break;
            case L':':
                result = OP_COLON;
                posNext++;
                break;
            case L'*':
                posNext++;
                result = OP_MULTIPLY;
                break;
            case L'/':
                posNext++;
                result = OP_DIVIDE;
                break;
            case L';':
                posNext++;
                result = OP_SEMICOLON;
                break;
            case L'@':
                posNext++;
                if (posNext < lenSrc && srcCode[posNext] == L'=')
                {
                    posNext++;
                    result = OP_REFERTO;
                }
                else
                    result = OP_AT;
                break;
            case L'>':
                posNext++;
                if (posNext < lenSrc && srcCode[posNext] == L'=')
                {
                    posNext++;
                    result = OP_GREATEREQUAL;
                }
                else
                    result = OP_GREATER;
                break;
            case L'<':
                posNext++;
                if (posNext < lenSrc && srcCode[posNext] == L'=')
                {
                    posNext++;
                    result = OP_LESSEQUAL;
                }
                else if (posNext < lenSrc && srcCode[posNext] == L'>')
                {
                    posNext++;
                    result = OP_NOTEQUAL;
                }
                else
                    result = OP_LESS;
            }
        }
        else
        {
            int *intr = keywords.Find(srcCode + pos, MODULE_IRRELEVANT, static_cast<int>(posNext - pos));
            if (intr == nullptr)
                result = -1;
            else
                result = *intr;
        }
        if (result == -1)
        {
            //Idenfitier
            wordType = WordType::Identifier;
            strWord = srcCode + pos;
            lenString = static_cast<int>(posNext - pos);
            if (lenString > 53)
                throw Var(Exception::ERROR_CODE::IDENTIFIER_TOO_LONG);
        }
        else
        {
            //Keyword or operator
            keywordIdx = result;
            if (keywordIdx == KT_BOOLEAN_TRUE)
            {
                wordType = WordType::Boolean;
                bWord = true;
            }
            else if (keywordIdx == KT_BOOLEAN_FALSE)
            {
                wordType = WordType::Boolean;
                bWord = false;
            }
            else
            {
                wordType = WordType::Keyword;
            }
        }
        return;
    }
}