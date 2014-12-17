/**
*	@file	Utility.cpp
*/

#include "stdafx.h"
#include "Utility.h"
#include <boost\algorithm\string\case_conv.hpp>


std::string ConvertUTF8fromUTF16(const std::wstring& utf16)
{
	std::string utf8;
	int size = ::WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), utf16.length(), (LPSTR)utf8.data(), 0, nullptr, nullptr);
	if (size > 0) {
		utf8.resize(size);
		if (::WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), utf16.length(), (LPSTR)utf8.data(), size, nullptr, nullptr))
			return utf8;
	}
	return utf8;
}

std::wstring ConvertUTF16fromUTF8(const std::string& utf8)
{
	std::wstring utf16;
	int size = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), (LPWSTR)utf16.data(), 0);
	if (size > 0) {
		utf16.resize(size);
		if (::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), (LPWSTR)utf16.data(), size))
			return utf16;
	}
	return utf16;
}


std::vector<std::string>	SeparatePathUTF8(const std::wstring& path)
{
	std::vector<std::string>	partPath;
	int start = 0;
	int slash = path.find(L'\\', start);
	while (slash != path.npos) {
		std::wstring part = path.substr(start, slash - start);
		if (part.empty()) {
			ATLASSERT(FALSE);
			return partPath;
		}
		partPath.emplace_back(ConvertUTF8fromUTF16(part));
		start = slash + 1;
		slash = path.find(L'\\', start);
	}
	std::wstring part = path.substr(start);
	if (part.length())
		partPath.emplace_back(ConvertUTF8fromUTF16(part));

	return partPath;
}

std::vector<std::wstring>	SeparatePathUTF16(const std::wstring& path)
{
	std::vector<std::wstring>	partPath;
	int start = 0;
	int slash = path.find(L'\\', start);
	while (slash != path.npos) {
		std::wstring part = path.substr(start, slash - start);
		if (part.empty()) {
			ATLASSERT(FALSE);
			return partPath;
		}
		partPath.emplace_back(part);
		start = slash + 1;
		slash = path.find(L'\\', start);
	}
	std::wstring part = path.substr(start);
	if (part.length())
		partPath.emplace_back(part);

	return partPath;
}

CString	GetFileOrFolderName(CString path)
{
	::PathRemoveBackslashW(path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();
	LPCWSTR name = ::PathFindFileNameW(path);
	return name;
}

CString GetPathBaseName(CString path)
{
	CString fileName = GetFileOrFolderName(path);
	::PathRemoveExtensionW(fileName.GetBuffer(MAX_PATH));
	fileName.ReleaseBuffer();
	return fileName;
}

CString	GetParentFolderPath(CString path)
{
	::PathRemoveBackslashW(path.GetBuffer(MAX_PATH));
	::PathRemoveFileSpecW(path.GetBuffer(MAX_PATH));
	::PathRemoveBackslashW(path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();
	return path;
}

CString AddBackslash(CString path)
{
	::PathAddBackslashW(path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();
	return path;
}

std::wstring PathFindExtention(const std::wstring& fileName)
{
	std::wstring ext = ::PathFindExtensionW(fileName.c_str());
	if (ext.empty())
		return ext;
	if (ext.front() == L'.')
		ext.erase(ext.begin());
	boost::algorithm::to_lower(ext);
	return ext;
}


FILETIME	GetLastWriteTime(const std::wstring& filePath)
{
	FILETIME ft = {};
	HANDLE hFile = ::CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return ft;
	
	::GetFileTime(hFile, nullptr, nullptr, &ft);

	::CloseHandle(hFile);
	return ft;
}


bool SetClipboardText(const CString &str)
{
	if (str.IsEmpty())
		return false;

	int 	nByte = (str.GetLength() + 1) * sizeof(TCHAR);
	HGLOBAL hText = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nByte);
	if (hText == NULL)
		return false;

	BYTE*	pText = (BYTE *) ::GlobalLock(hText);
	if (pText == NULL)
		return false;

	::memcpy(pText, (LPCTSTR)str, nByte);

	::GlobalUnlock(hText);

	::OpenClipboard(NULL);
	::EmptyClipboard();
	::SetClipboardData(CF_UNICODETEXT, hText);
	::CloseClipboard();
	return true;
}

CString GetExeDirectory()
{
	CString exePath;
	::GetModuleFileNameW(NULL, exePath.GetBuffer(MAX_PATH), MAX_PATH);
	::PathRemoveFileSpecW(exePath.GetBuffer(MAX_PATH));
	::PathAddBackslashW(exePath.GetBuffer(MAX_PATH));
	exePath.ReleaseBuffer();
	return exePath;
}



















