#pragma once

#include "atlwin.h"
#include <atlcrack.h>
#include <atlctrls.h>
#include "resource.h"


class CRenameDialog :
	public CDialogImpl <CRenameDialog>
{
public:
	enum { IDD = IDD_RENAME };
	enum { WM_SELTEXTWITHOUTEXT = WM_APP + 1 };

	CString	m_filePath;

	CRenameDialog(const CString& path);
	~CRenameDialog();

	BEGIN_MSG_MAP_EX(CRenameDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER_EX(WM_SELTEXTWITHOUTEXT, OnSelTextWithoutExt)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	LRESULT OnSelTextWithoutExt(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	CEdit	m_edit;
	CString	m_fileParentFolder;
	CString m_fileName;
};

