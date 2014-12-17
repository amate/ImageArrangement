// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <deque>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlcrack.h>
#include "ImageArrangementView.h"
#include "CandidateWindow.h"
#include "resource.h"
#include "AboutDlg.h"
#include "ConfigDialog.h"


class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CMainFrame();


	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MSG_WM_MOVE(OnMove)
		MSG_WM_DROPFILES(OnDropFiles)
		COMMAND_ID_HANDLER_EX(ID_NEXT_IMAGE, OnPageImage)
		COMMAND_ID_HANDLER_EX(ID_PREV_IMAGE, OnPageImage)
		COMMAND_ID_HANDLER_EX(ID_EDIT_UNDO, OnUndo)
		COMMAND_ID_HANDLER_EX(ID_DELETEIMAGE, OnDeleteImage)
		COMMAND_ID_HANDLER_EX(ID_MOVELASTFOLDER, OnMoveLastFolder)
		COMMAND_ID_HANDLER_EX(ID_RENAME_IMAGE, OnRenameImage)
		COMMAND_ID_HANDLER_EX(ID_OPEN_IMAGE, OnOpenImage )
		COMMAND_ID_HANDLER_EX(ID_CLEARQUE, OnClearQue)
		COMMAND_ID_HANDLER_EX(ID_SHOWCONFIGDIALOG, OnShowConfigDialog)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	void OnMove(CPoint ptPos);
	void OnDropFiles(HDROP hDropInfo);
	void OnPageImage(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnUndo(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnDeleteImage(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnMoveLastFolder(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnRenameImage(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnOpenImage(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnClearQue(UINT uNotifyCode, int nID, CWindow wndCtl);

	void OnShowConfigDialog(UINT uNotifyCode, int nID, CWindow wndCtl);

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

private:
	bool	_PageImage(bool bNext);
	void	_ChangeWindowTitle(const CString& operateText = CString());
	bool	_IsValidPos(size_t nPos);
	void	_OnEnter(const CString& name, const CString& path);

	CImageArrangementView m_view;
	CCandidateWindow	m_candidateWindow;
	size_t				m_imagePos;
	std::deque<CString>	m_imageList;
	CString				m_lastMoveDestinyFolder;
	CString				m_lastMoveDestinyFolderName;

	struct MoveHistory
	{
		CString moveSrc;
		CString moveDestiny;
		CConfig::FileOperateMode fileOperateMode;

		MoveHistory(CConfig::FileOperateMode mode, const CString& src, const CString& dest) : 
			fileOperateMode(mode), moveSrc(src), moveDestiny(dest)
		{}
	};
	std::list<MoveHistory>	m_history;
};
