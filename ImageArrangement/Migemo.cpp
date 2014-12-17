/**
*	@file	Migemo.cpp
*/

#include "stdafx.h"
#include "Migemo.h"
#include <stdexcept>

#include <migemo.h>
#pragma comment(lib, "migemo.lib")
#define MIGEMODICT_NAME L"migemo-dict"

#include "Utility.h"

////////////////////////////////////////////////////////////////////////
// CMigemo

CMigemo::CMigemo() : m_pmigemo(NULL)
{
	CString dicPath = GetExeDirectory() + L"dict\\" MIGEMODICT_NAME;
	m_pmigemo = migemo_open((LPSTR)CW2A(dicPath));
	if (m_pmigemo == NULL)
		throw std::runtime_error("辞書データのオープンに失敗");

}


CMigemo::~CMigemo()
{
	migemo_close(m_pmigemo);
}


std::wstring	CMigemo::GetRegexPattern(const std::wstring& query)
{
	std::string utf8 = ConvertUTF8fromUTF16(query);
	unsigned char* ans = migemo_query(m_pmigemo, (unsigned char*)utf8.c_str());
	std::string utf8ans = (char*)ans;
	migemo_release(m_pmigemo, ans);

	std::wstring rx = ConvertUTF16fromUTF8(utf8ans);
	return rx;
}




