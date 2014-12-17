/*!	@file	ConfigDialog.h
	@brief	設定ダイアログ

*/

#pragma once

#include <list>
#include <string>
#include <atldlgs.h>
#include <atlcrack.h>

struct CandidateData
{
	std::wstring	name;
	std::wstring	path;

	CandidateData(const std::wstring& name, const std::wstring& path) : name(name), path(path) {}
};

class CConfig
{
public:
	static std::list<CandidateData> s_candidateNameFolderList;

	enum FileOperateMode {
		kFileMove,
		kFileCopy,
	};
	static FileOperateMode	s_fileOperateMode;

	enum FileDeleteMode {
		kFileDeleteOnCycle,
		kFileDeleteDirect,
		kFileDeleteFromQue,
	};
	static FileDeleteMode	s_fileDeleteMode;

	static bool				s_bAddImageOnSubFolder;

	static void	LoadConfig();
	static void SaveConfig();

};


///////////////////////////////////////////////////////////
///  設定プロパティシート

class CConfigSheet : public CPropertySheetImpl<CConfigSheet>
{
public:
	INT_PTR	Show(HWND hWndParent);

    BEGIN_MSG_MAP_EX( CConfigSheet )
        CHAIN_MSG_MAP( CPropertySheetImpl<CConfigSheet> )
    END_MSG_MAP()

};






















