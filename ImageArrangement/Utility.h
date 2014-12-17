/**
*	@file	Utility.h
*/

#pragma once

#include <string>
#include <vector>
#include <atlmisc.h>	// for CString

std::string ConvertUTF8fromUTF16(const std::wstring& utf16);

std::wstring ConvertUTF16fromUTF8(const std::string& utf8);


std::vector<std::string>	SeparatePathUTF8(const std::wstring& path);
std::vector<std::wstring>	SeparatePathUTF16(const std::wstring& path);

CString	GetFileOrFolderName(CString path);
CString GetPathBaseName(CString path);
CString	GetParentFolderPath(CString path);
CString AddBackslash(CString path);
std::wstring PathFindExtention(const std::wstring& fileName);

FILETIME	GetLastWriteTime(const std::wstring& filePath);

bool SetClipboardText(const CString &str);

CString GetExeDirectory();























