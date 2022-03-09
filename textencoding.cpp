#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <filesystem>
#endif

std::wstring utf8_to_wchar(const char* str)
{
    std::wstring wstr;
    int i = 0;
    int utf8size = 0;
    wchar_t wchar;
    while (str[i])
    {
        char ch = str[i];
        i++;

        if ((unsigned char)ch < 0x80)
        {
            if (utf8size > 0)
            {
                //Not UTF-8!
            }
            wchar = static_cast<wchar_t>(ch);
            wstr.push_back(wchar);
            utf8size = 0;
        }
        else
        {
            if (utf8size == 0)
            {
                if ((ch & 0xc0) == 0xc0)
                {
                    utf8size = 1;
                    wchar = ch & 0x1f;
                }
                if ((ch & 0xe0) == 0xe0)
                {
                    utf8size = 2;
                    wchar = ch & 0xf;
                }
                if ((ch & 0xf0) == 0xf0)
                {
                    utf8size = 3;
                    wchar = ch & 0x7;
                }
                if ((ch & 0xf8) == 0xf8)
                {
                    utf8size = 4;
                    wchar = ch & 0x3;
                }
                if ((ch & 0xfc) == 0xfc)
                {
                    utf8size = 5;
                    wchar = ch & 0x1;
                }
            }
            else
            {
                utf8size--;
                wchar <<= 6;
                wchar |= ch & 0x3f;
                if (utf8size == 0)
                    wstr.push_back(wchar);
            }
        }
    }
    return wstr;
}

std::string wchar_to_utf8(const wchar_t* wstr)
{
    char array[6] {};
    std::string result;
    wchar_t w;
    int pos = 0;
    while ((w = wstr[pos++]) != 0)
    {
        int i = 0;
        if (w < 0x80)
            array[i] = (char)w;
        else
        {
            while (w != 0)
            {
                array[i] = (w & 0x3f) | 0x80;
                w >>= 6;
                i++;
            }
            i--;
        }
        if (i == 1)
            array[1] |= 0xc0;
        if (i == 2)
            array[2] |= 0xe0;
        if (i == 3)
            array[3] |= 0xf0;
        if (i == 4)
            array[4] |= 0xf8;
        if (i == 5)
            array[5] |= 0xfc;
        while (i >= 0)
        {
            result.push_back(array[i]);
            i--;
        }
    }
    return result;
}

//Windows and Linux/macOS have different way of handling std::string
std::string wchar_filename_to_string(const wchar_t* wstr)
{
#if defined(_WIN32) || defined(_WIN64)
    int size = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* p = new char[size];
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, p, size, NULL, NULL);
    std::string str = p;
    delete[] p;
    return str;
#else
    return wchar_to_utf8(wstr);
#endif
}

//Windows and Linux/macOS have different way of handling std::string
std::wstring string_filename_to_wchar(const char* str)
{
#if defined(_WIN32) || defined(_WIN64)
    std::filesystem::path pname(str);
    std::wstring ws = pname.filename().wstring();
    return ws;
#else
    return utf8_to_wchar(str);
#endif    
}