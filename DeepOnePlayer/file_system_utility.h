#ifndef FILE_SYSTEM_UTILITY_H_
#define FILE_SYSTEM_UTILITY_H_

#include <string>
#include <vector>

bool CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileExtension, std::vector<std::wstring>& paths);
std::wstring LoadFileAsString(const wchar_t* pwzFilePath);

#endif //FILE_SYSTEM_UTILITY_H_