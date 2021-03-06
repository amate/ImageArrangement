/**
*	@file	ptreeWrapper.h
*/

#pragma once

#include <fstream>
#include <codecvt>
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\xml_parser.hpp>
#include <boost\property_tree\ini_parser.hpp>
#include <atlmisc.h>
#include "Utility.h"

class ptreeWrapper
{
public:
	using wptree = boost::property_tree::wptree;

	static wptree LoadIniPtree(const WTL::CString& fileName)
	{
		wptree pt;

		WTL::CString filterPath = GetExeDirectory() + fileName;
		std::wifstream	fs(filterPath);
		if (!fs) {
			return pt;
		}
		fs.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));

		try {
			boost::property_tree::read_ini(fs, pt);
		}
		catch (...) {
		}
		return pt;
	}

	static bool SaveIniPtree(const WTL::CString& fileName, const wptree& pt)
	{
		WTL::CString filterPath = GetExeDirectory() + fileName;
		std::wofstream	fs(filterPath);
		if (!fs)
			return false;
		fs.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));

		try {
			boost::property_tree::write_ini(fs, pt);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	static wptree BuildPtreeFromText(const std::wstring& text)
	{
		std::wstringstream ss(text);
		wptree pt;
		try {
			boost::property_tree::read_xml(ss, pt);
		}
		catch (...) {

		}
		return pt;
	}

};



























