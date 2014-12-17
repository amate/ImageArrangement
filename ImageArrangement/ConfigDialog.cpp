/*!	@file	ConfigDialog.h
	@brief	設定ダイアログ

*/

#include "stdafx.h"
#include "ConfigDialog.h"
#include <vector>
#include <algorithm>
#include <atlctrls.h>

#include "resource.h"
#include "DropHanler.h"
#include "ptreeWrapper.h"

//////////////////////////////////////////////
// CConfig
std::list<CandidateData> CConfig::s_candidateNameFolderList;
CConfig::FileOperateMode	CConfig::s_fileOperateMode = kFileMove;
CConfig::FileDeleteMode		CConfig::s_fileDeleteMode = kFileDeleteOnCycle;
bool				CConfig::s_bAddImageOnSubFolder = false;


void	CConfig::LoadConfig()
{
	auto pt = ptreeWrapper::LoadIniPtree(_T("settings.ini"));

	s_fileOperateMode = (FileOperateMode)pt.get<int>(L"Setting.FileOperateMode", s_fileOperateMode);
	s_fileDeleteMode = (FileDeleteMode)pt.get<int>(L"Setting.FileDeleteMode", s_fileDeleteMode);

	s_bAddImageOnSubFolder = pt.get<bool>(L"Setting.AddImageOnSubFolder", s_bAddImageOnSubFolder);

	const int exFolderCount = pt.get(L"CandidateFolder.Count", 0);
	if (exFolderCount) {
		for (int i = 0; i < exFolderCount; ++i) {
			CString name = L"CandidateFolder.";
			name.Append(i);
			std::wstring value = pt.get((LPCWSTR)name, L"");
			auto sepPos = value.find(L'|');
			if (sepPos != std::wstring::npos) {
				s_candidateNameFolderList.emplace_back(value.substr(0, sepPos), value.substr(sepPos + 1));
			}
		}
	}
}

void	CConfig::SaveConfig()
{
	auto pt = ptreeWrapper::LoadIniPtree(_T("settings.ini"));

	pt.put<int>(L"Setting.FileOperateMode", s_fileOperateMode);
	pt.put<int>(L"Setting.FileDeleteMode", s_fileDeleteMode);

	pt.put<bool>(L"Setting.AddImageOnSubFolder", s_bAddImageOnSubFolder);

	const auto erasecount = pt.erase(L"CandidateFolder");
	const size_t exFolderCount = s_candidateNameFolderList.size();
	pt.put(L"CandidateFolder.Count", exFolderCount);
	int i = 0;
	for (auto& candidateData : s_candidateNameFolderList) {
		CString name = L"CandidateFolder.";
		name.Append(i);
		++i;
		CString value;
		value.Format(_T("%s|%s"), candidateData.name.c_str(), candidateData.path.c_str());
		pt.put((LPCWSTR)name, (LPCWSTR)value);
	}
	ATLVERIFY(ptreeWrapper::SaveIniPtree(_T("settings.ini"), pt));
}


//////////////////////////////////////////////
/// 全般
class CGeneralPropertyPage : public CPropertyPageImpl<CGeneralPropertyPage>
{
public:
	enum { IDD = IDD_PROPPAGE_GENERAL };

	CGeneralPropertyPage() : m_candidateFolderList(this, 1) {}


	BEGIN_MSG_MAP_EX( CGeneralPropertyPage )
		MSG_WM_INITDIALOG( OnInitDialog )
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_CLEARALLFOLDER, OnClearFolder)
		NOTIFY_HANDLER_EX(IDC_LIST_CANDIDATEFOLDER, LVN_ENDLABELEDIT, OnListEndLabelEdit)
		CHAIN_MSG_MAP( CPropertyPageImpl<CGeneralPropertyPage> )
	ALT_MSG_MAP(1)
		MSG_WM_DROPFILES(OnDropFiles)
		MSG_WM_KEYDOWN(OnListViewKeyDown)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		m_candidateFolderList.SubclassWindow(GetDlgItem(IDC_LIST_CANDIDATEFOLDER));
		m_candidateFolderList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

		m_candidateFolderList.InsertColumn(0, _T("名前"), LVCFMT_LEFT, 130);
		m_candidateFolderList.InsertColumn(1, _T("パス"), LVCFMT_LEFT, 400);

		for (auto& candidateData : CConfig::s_candidateNameFolderList) {
			int nIndex = m_candidateFolderList.InsertItem(m_candidateFolderList.GetItemCount(), candidateData.name.c_str());

			LVITEM	Item = { 0 };
			Item.mask = LVIF_TEXT;
			Item.iItem = nIndex;
			Item.iSubItem = 1;
			Item.pszText = (LPWSTR)candidateData.path.c_str();
			BOOL bRet = m_candidateFolderList.SetItem(&Item);
		}
		m_candidateFolderList.DragAcceptFiles();

		m_cmbFileOperateMode = GetDlgItem(IDC_COMBO_FILEOPERATEMODE);
		m_cmbFileOperateMode.AddString(L"移動");
		m_cmbFileOperateMode.AddString(L"コピー");
		m_cmbFileOperateMode.SetCurSel(CConfig::s_fileOperateMode);

		
		m_cmbFileDeleteMode = GetDlgItem(IDC_COMBO_FILEDELETEMODE);
		m_cmbFileDeleteMode.AddString(L"ごみ箱へ削除");
		m_cmbFileDeleteMode.AddString(L"削除");
		m_cmbFileDeleteMode.AddString(L"キューから取り除く");
		m_cmbFileDeleteMode.SetCurSel(CConfig::s_fileDeleteMode);

		CButton(GetDlgItem(IDC_CHECK_ADDIMAGEONSUBFOLDER)).SetCheck(CConfig::s_bAddImageOnSubFolder);

		return TRUE;
	}

	void OnDestroy()
	{
		m_candidateFolderList.UnsubclassWindow(TRUE);
	}

	BOOL OnApply()
	{
		CConfig::s_candidateNameFolderList.clear();

		const int count = m_candidateFolderList.GetItemCount();
		for (int i = 0; i < count; ++i) {
			CString name;
			m_candidateFolderList.GetItemText(i, 0, name);
			CString path;
			m_candidateFolderList.GetItemText(i, 1, path);

			CConfig::s_candidateNameFolderList.emplace_back((LPCWSTR)name, (LPCWSTR)path);
		}
		CConfig::s_fileOperateMode = (CConfig::FileOperateMode)m_cmbFileOperateMode.GetCurSel();
		CConfig::s_fileDeleteMode = (CConfig::FileDeleteMode)m_cmbFileDeleteMode.GetCurSel();

		CConfig::s_bAddImageOnSubFolder = CButton(GetDlgItem(IDC_CHECK_ADDIMAGEONSUBFOLDER)).GetCheck() != 0;

		CConfig::SaveConfig();

		return TRUE;
	}

	void OnClearFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		m_candidateFolderList.DeleteAllItems();
	}

	LRESULT OnListEndLabelEdit(LPNMHDR pnmh)
	{
		auto pdi = (LPNMLVDISPINFOW)pnmh;
		if (pdi->item.pszText == nullptr || pdi->item.pszText[0] == L'\0')
			return FALSE;

		return TRUE;
	}

	void OnDropFiles(HDROP hDropInfo)
	{
		std::vector<CString> vecFolder;
		DropHanler drop(hDropInfo);
		UINT nCount = drop.GetCount();
		for (UINT i = 0; i < nCount; ++i) {
			if (::PathIsDirectory(drop[i])) 
				vecFolder.emplace_back(drop[i]);
		}
		std::sort(vecFolder.begin(), vecFolder.end(), [](const CString& path1, const CString& path2) {
			return ::StrCmpLogicalW(path1, path2) < 0;
		});

		for (auto& folder : vecFolder) {
			LPCWSTR name = ::PathFindFileName(folder);

			int nIndex = m_candidateFolderList.InsertItem(m_candidateFolderList.GetItemCount(), name);

			LVITEM	Item = { 0 };
			Item.mask = LVIF_TEXT;
			Item.iItem = nIndex;
			Item.iSubItem = 1;
			Item.pszText = (LPWSTR)(LPCTSTR)folder;
			BOOL bRet = m_candidateFolderList.SetItem(&Item);
		}
	}

	void OnListViewKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (nChar == VK_DELETE) {
			int nSel = m_candidateFolderList.GetSelectedIndex();
			if (nSel == -1)
				return;
			m_candidateFolderList.DeleteItem(nSel);
			if (m_candidateFolderList.SelectItem(nSel) == FALSE)
				m_candidateFolderList.SelectItem(nSel - 1);
		}

	}


private:
	CContainedWindowT<CListViewCtrl>	m_candidateFolderList;
	CComboBox	m_cmbFileOperateMode;
	CComboBox	m_cmbFileDeleteMode;
};





/// プロパティシートを表示
INT_PTR	CConfigSheet::Show(HWND hWndParent)
{
	SetTitle(_T("設定"));
	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;

	CGeneralPropertyPage	generalPage;
	AddPage(generalPage);
	
	return DoModal(hWndParent);
}


