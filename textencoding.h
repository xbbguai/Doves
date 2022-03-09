#pragma once

std::string wchar_to_utf8(const wchar_t* wstr);
std::wstring utf8_to_wchar(const char* str);

//Windows and Linux/macOS have different way of handling std::string
std::string wchar_filename_to_string(const wchar_t* wstr);
std::wstring string_filename_to_wchar(const char* str);