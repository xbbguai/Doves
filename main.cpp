#include <iostream>
#include "var.h"
#include "HashTable.h"
#include "WordReader.h"
#include "Expression.h"
#include "parser.h"
#include "Exception.h"
#include "textencoding.h"
#include <locale.h>
#include <fstream>
#include <string.h>

int main(int paramsCount, char** params)
{   
    bool chn = false;
    std::string mainModuleName = "";
    int paramsUsed = 1;
    if (paramsCount > 1)
    {
        //Parse the params
        for (paramsUsed = 1; paramsUsed < paramsCount; paramsUsed++)
        {
            if (params[paramsUsed][0] == '/' || params[paramsUsed][0] == '-')
            {
                //Must be parameter
                switch (params[paramsUsed][1])
                {
                case 'C':
                case 'c':
                    chn = true;
                    break;
                case 'E':
                case 'e':
                    break;
                default:
                    std::wcout << L"Unknown parameter." << std::endl;
                    return 0;
                }
            }
            else
            {
                //Must be main module name
                mainModuleName = params[paramsUsed];
                paramsUsed++;
                break;  //The params after this, will be transfered to Doves program.
            }
        }
    }
    else
    {
        std::cout << "Doves V0.1a" << std::endl;
        std::cout << "Copyrighted (C) 2022 Feng Dai" << std::endl;
        std::cout << "Released under GNU GPLv3" << std::endl;
#ifndef _DEBUG
        return 0;
#endif
    }

    std::ios::sync_with_stdio(false);
    std::wcout.imbue(std::locale(std::locale(), "", LC_CTYPE));
    std::cout.imbue(std::locale(std::locale(), "", LC_CTYPE));
    Parser parser(chn);
    Var& var = parser.globalVarTable.Insert(L"Params", 0, Var());   //Module 0 is main module.
    if (paramsUsed < paramsCount)
    {
        var.InitArray(paramsCount - paramsUsed, VarType::String);
        int i = 0;
        for (; paramsUsed < paramsCount; paramsUsed++)
        {
            std::wstring w = string_filename_to_wchar(params[paramsUsed]);
            var.GetElementAt(i++).SetString(w.c_str());
        }
    }
    try
    {
        parser.ImportSource(mainModuleName);
        int* idx = parser.functionNameIndex.Find(L"main", 0, 4);
        HashTable<Var> localVarTable;
        if (idx != nullptr)
            parser.Execute(*idx, localVarTable);
        
        //Before exiting, check the public threads.
        for (auto& item : parser.globalVarTable.anonymousList)
        {
            if (item.GetVarType() == VarType::InternalThreadObj)
            {
                std::thread* pobj = (std::thread*)item.GetPointer();
                if (pobj != nullptr && pobj->joinable())
                    pobj->join();   //Wait for the thread to end
                item.DecreaseReferCount();
            }
        }    
    }
    catch (Var& eVar)
    {
        if (eVar.GetVarType() == VarType::Structure && eVar.GetStructureMember(-1).GetInteger() == *parser.structureNameIndex.Find(L"EXCEPTION", 0, 9))
            std::cout<< "Error at line #" << eVar.GetStructureMember(1).GetInteger()
                     << " column #" << eVar.GetStructureMember(2).GetInteger()
                     << " module #" << eVar.GetStructureMember(3).GetInteger() 
                     << " in file: '" << parser.modules[eVar.GetStructureMember(3).GetInteger()].fileName << "'" << std::endl 
                     << Exception::GetErrorMessage(static_cast<Exception::ERROR_CODE>(eVar.GetStructureMember(0).GetInteger()))
                     << std::endl;
        else if (eVar.GetVarType() == VarType::InternalException)
        {
            int errCode = eVar.GetInteger();
            std::pair<int, int> pos = parser.LineFromPosition();
            if (parser.currentModule == -1)
                std::cout << Exception::GetErrorMessage(static_cast<Exception::ERROR_CODE>(errCode)) << std::endl;
            else
                std::cout << "Error at line #" << pos.first
                        << " column #" << pos.second
                        << " module #" << parser.currentModule
                        << " in file: '" << parser.modules[parser.currentModule].fileName << "'" << std::endl 
                        << Exception::GetErrorMessage(static_cast<Exception::ERROR_CODE>(errCode))
                        << std::endl;
        }
        else
        {
            auto pos = parser.LineFromPosition();
            std::cout << "Thrown exception at line # " << pos.first << " column # " << pos.second 
                      << " module #" << parser.currentModule
                      << " is not handled by the program." << std::endl;
        }
    }
    catch (int)
    {
        
    }
    catch (...)
    {
        //Program ends.
    }

    return 0;
}