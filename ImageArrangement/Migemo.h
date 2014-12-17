/**
*	@file	Migemo.h
*/

#pragma once

#include <string>

struct _migemo;

//////////////////////////////////////////////////
// CMigemo

class CMigemo
{
public:
	CMigemo();
	~CMigemo();

	std::wstring	GetRegexPattern(const std::wstring& query);

private:
	_migemo*	m_pmigemo;
};

















