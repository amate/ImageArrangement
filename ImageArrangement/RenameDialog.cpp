#include "stdafx.h"
#include "RenameDialog.h"
#include "Utility.h"
#include "Shlobj.h"

/// 有効なファイル名ならtrueを返す
inline bool IsValidateFileName(const CString& strFileName)
{
	return strFileName.FindOneOf(_T("\\/:*?\"<>|")) == -1;
}


////////////////////////////////////////////////////////////////////
// CRenameDialog

CRenameDialog::CRenameDialog(const CString& path) : m_filePath(path)
{
	m_fileParentFolder = AddBackslash(GetParentFolderPath(m_filePath));
	m_fileName = GetFileOrFolderName(m_filePath);
}


CRenameDialog::~CRenameDialog()
{
}

BOOL CRenameDialog::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_edit = GetDlgItem(IDC_EDIT_NAME);
	m_edit.SetWindowText(m_fileName);
	int dotPos = m_fileName.Find(L'.');
	if (dotPos != -1) {
		PostMessage(WM_SELTEXTWITHOUTEXT, dotPos);
	}

	return 0;
}


LRESULT CRenameDialog::OnSelTextWithoutExt(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_edit.SetSel(0, (int)wParam, TRUE);
	m_edit.SetFocus();
	return 0;
}


void CRenameDialog::OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CString newFileName;
	m_edit.GetWindowText(newFileName.GetBuffer(MAX_PATH), MAX_PATH);
	newFileName.ReleaseBuffer();
	if (newFileName.IsEmpty()) {
		MessageBox(L"ファイル名を入力してください。", NULL, MB_ICONERROR);
		return;
	}

	if (IsValidateFileName(newFileName) == false) {
		MessageBox(_T("有効なファイル名ではありません。\n「\\/:*?\"<>|」はファイル名に含めることはできません。"), NULL, MB_ICONERROR);
		return;
	}

	if (newFileName == m_fileName) {
		EndDialog(IDCANCEL);
		return;
	}

	m_filePath = m_fileParentFolder + newFileName;
	if (::PathFileExists(m_filePath)) {
		MessageBox(_T("同じ名前のファイルが存在します。"), NULL, MB_ICONERROR);
		return;
	}

	// リネーム
	BOOL bRet = ::MoveFileEx(m_fileParentFolder + m_fileName, m_filePath, 0);
	if (bRet == 0) {
		MessageBox(_T("リネームに失敗しました。"), NULL, MB_ICONERROR);
		EndDialog(IDCANCEL);
	} else {
		/* エクスプローラーにファイルの変更通知 */
		::SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH,
			static_cast<LPCTSTR>(m_fileParentFolder + m_fileName),
			static_cast<LPCTSTR>(m_filePath));
	}

	EndDialog(IDOK);
}

void CRenameDialog::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	EndDialog(IDCANCEL);
}

























