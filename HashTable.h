#pragma once

#include <string>
#include <optional>
#include <list>
#include <mutex>

#define MODULE_IRRELEVANT -1
#define MAX_IDENTIFIER_SIZE 64

extern std::mutex anonymousListMtx;

template<typename T = int, int MAX_BARRELS = 64, int MAX_KEYWORD_LEN = MAX_IDENTIFIER_SIZE>
class HashTable
{
protected:
    int size {0};
public:
    struct BarrelElement
    {
        wchar_t keyword[MAX_KEYWORD_LEN];
        int moduleIdx;   //If there are many same keywords, they may be distinguished by moduleIdx.
        int len;
        T value;
    };

    std::list<BarrelElement> barrels[MAX_BARRELS];
    std::list<T> anonymousList;

    T& Insert(const wchar_t* keyword, const int& moduleIdx, const T& value);
    T& Insert(const wchar_t* keyword, const int& moduleIdx, const int len, const T& value);
    T* Find(const wchar_t* key, const int& moduleIdx, const int len); //, int sum = -1);

    T& InsertAnonymous(const T& value);
    void RemoveAnonymous(const T& value);   //Not used.

    int GetSize() const { return size; }
    wchar_t* ReverseFind(const T& value);
};

inline wchar_t toupper(const wchar_t& ch)
{
    if (ch >= L'a' && ch <= L'z')
        return ch - L'a' + L'A';
    else
        return ch;
}

template<typename T, int MAX_BARRELS, int MAX_KEYWORD_LEN>
T& HashTable<T, MAX_BARRELS, MAX_KEYWORD_LEN>::Insert(const wchar_t* keyword, const int& moduleIdx, const T& value)
{
    wchar_t sum = 0;
    BarrelElement ele;

    for (ele.len = 0; keyword[ele.len] > 0; ele.len++)
    {
        ele.keyword[ele.len] = toupper(keyword[ele.len]);
        sum += ele.keyword[ele.len];
    }
    ele.keyword[ele.len] = 0;
    ele.moduleIdx = moduleIdx;
    ele.value = value;  //This will result in a complete copy of the original value
    sum += static_cast<wchar_t>(moduleIdx) + ele.len;
    sum %= MAX_BARRELS;
    size++;
    return barrels[sum].emplace_back(ele).value;
}

template<typename T, int MAX_BARRELS, int MAX_KEYWORD_LEN>
T& HashTable<T, MAX_BARRELS, MAX_KEYWORD_LEN>::Insert(const wchar_t* keyword, const int& moduleIdx, const int len, const T& value)
{
    wchar_t sum = 0;
    BarrelElement ele;

    for (ele.len = 0; ele.len < len; ele.len++)
    {
        ele.keyword[ele.len] = toupper(keyword[ele.len]);
        sum += ele.keyword[ele.len];
    }

    ele.keyword[ele.len] = 0;
    ele.value = value; //This will result in a complete copy of the original value
    ele.moduleIdx = moduleIdx;
    sum += static_cast<wchar_t>(moduleIdx) + ele.len;
    sum %= MAX_BARRELS;
    size++;
    return barrels[sum].emplace_back(ele).value;
}

template<typename T, int MAX_BARRELS, int MAX_KEYWORD_LEN>
T* HashTable<T, MAX_BARRELS, MAX_KEYWORD_LEN>::Find(const wchar_t* key, const int& moduleIdx, const int len)
{
    wchar_t sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += toupper(key[i]);
    }
    sum += static_cast<wchar_t>(moduleIdx) + len;

    int pos = sum % MAX_BARRELS;
    
    for (auto& item : barrels[pos])
    {
        if (len == item.len && moduleIdx == item.moduleIdx)
        {
            int i;
            for (i = 0; i < item.len; i++)
            {
                if (item.keyword[i] != toupper(key[i]))
                    break;
            }
            if (i == item.len)
                return &item.value;
        }
    }
    return nullptr;
}

template<typename T, int MAX_BARRELS, int MAX_KEYWORD_LEN>
wchar_t* HashTable<T, MAX_BARRELS, MAX_KEYWORD_LEN>::ReverseFind(const T& value)
{
    for (auto& barrel : barrels)
    {
        for (auto& element : barrel)
        {
            if (element.value == value)
                return element.keyword;
        }
    }

    return nullptr;
}

template<typename T, int MAX_BARRELS, int MAX_KEYWORD_LEN>
T& HashTable<T, MAX_BARRELS, MAX_KEYWORD_LEN>::InsertAnonymous(const T& value)
{
    std::unique_lock lock(anonymousListMtx);
    T& var = anonymousList.emplace_back(value);
    lock.unlock();
    return var;
}

template<typename T, int MAX_BARRELS, int MAX_KEYWORD_LEN>
void HashTable<T, MAX_BARRELS, MAX_KEYWORD_LEN>::RemoveAnonymous(const T& value)
{
    std::unique_lock lock(anonymousListMtx);
    anonymousList.remove(value);
    lock.unlock();
}
