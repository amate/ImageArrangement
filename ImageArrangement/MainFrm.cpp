/**
*	@file	MainFrm.cpp
*/

#include "stdafx.h"
#include "MainFrm.h"
#include <vector>
#include <algorithm>
#include <boost\format.hpp>
#include <imm.h>
#pragma comment(lib, "imm32.lib")
#include "Utility.h"
#include "ptreeWrapper.h"
#include "Migemo.h"
#include "ConfigDialog.h"
#include "DropHanler.h"
#include "AppConst.h"
#include "RenameDialog.h"

using boost::wformat;

void	ForEachFileFolder(const CString& folderPath, std::function<void(const CString&, DWORD)> callback)
{
	if (::PathIsDirectory(folderPath) == FALSE)
		throw std::runtime_error("path is not folder");

	CString directory = folderPath;
	if (directory[directory.GetLength() - 1] != L'\\')
		directory += L'\\';

	CString FolderFind = directory + L'*';

	WIN32_FIND_DATAW wfd = {};
	HANDLE h = ::FindFirstFileW(FolderFind, &wfd);
	if (h == INVALID_HANDLE_VALUE)
		throw std::runtime_error("FindFirstFileW failed");

	// Now scan the directory
	do {
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// ignore the current and parent directory entries
			if (::lstrcmp(wfd.cFileName, _T(".")) == 0 || ::lstrcmp(wfd.cFileName, _T("..")) == 0)
				continue;
		}
		callback(directory + wfd.cFileName, wfd.dwFileAttributes);

	} while (::FindNextFile(h, &wfd));

	::FindClose(h);
}

bool	PathIsImageFile(const CString& path)
{
	std::wstring ext = PathFindExtention((LPCWSTR)path);
	if (ext == L"jpg" || ext == L"jpeg" || ext == L"png" || ext == L"gif" || ext == L"bmp")
		return true;
	else
		return false;
}

bool	MoveFile(const CString& srcFilePath, const CString& destinyFilePath)
{
	BOOL b = ::MoveFileEx(srcFilePath, destinyFilePath, MOVEFILE_COPY_ALLOWED);
	if (b == 0) {		
		return false;
	}
	::SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, (LPCVOID)(LPCWSTR)srcFilePath, nullptr);
	::SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, (LPCVOID)(LPCWSTR)destinyFilePath, nullptr);
	return true;
}


//////////////////////////////////////////////////////////////////////////
// CMainFrame

CMainFrame::CMainFrame() : m_imagePos(0)
{}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	::ImmAssociateContext(m_view, NULL);

	m_candidateWindow.Create(m_hWnd, CRect(0, 0, 100, 200), nullptr, WS_POPUP | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL, WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_candidateWindow, 0, 200, LWA_ALPHA);
	m_candidateWindow.SetViewWindow(m_view);
	m_candidateWindow.SetOnEnterFunc([this](const CString& name, const CString& path) {
		_OnEnter(name, path);
	});
	m_candidateWindow.SetOnDeleteFunc([this](){
		OnDeleteImage(0, 0, NULL);
	});

	m_view.SetShowCandidateWindowFunc( std::bind(&CCandidateWindow::ShowCandidateWindow, &m_candidateWindow, std::placeholders::_1) );

	DragAcceptFiles();

	auto pt = ptreeWrapper::LoadIniPtree(L"settings.ini");
	CRect rc;
	rc.top = pt.get<int>(L"MainFrame.top", 0);
	rc.left = pt.get<int>(L"MainFrame.left", 0);
	rc.right = pt.get<int>(L"MainFrame.right", 0);
	rc.bottom = pt.get<int>(L"MainFrame.bottom", 0);
	if (rc != CRect())
		SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);

	CConfig::LoadConfig();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (IsIconic() == FALSE) {
		auto pt = ptreeWrapper::LoadIniPtree(L"settings.ini");
		CRect rc;
		GetWindowRect(&rc);
		pt.put(L"MainFrame.top", rc.top);
		pt.put(L"MainFrame.left", rc.left);
		pt.put(L"MainFrame.right", rc.right);
		pt.put(L"MainFrame.bottom", rc.bottom);
		ptreeWrapper::SaveIniPtree(L"settings.ini", pt);
	}

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}


void CMainFrame::OnMove(CPoint ptPos)
{
	CPoint ptClientScreen;
	m_view.ClientToScreen(&ptClientScreen);
	m_candidateWindow.SetWindowPos(NULL, ptClientScreen.x, ptClientScreen.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	std::list<CString> dropFiles;
	std::list<CString> dropFolders;
	DropHanler drop(hDropInfo);
	UINT nCount = drop.GetCount();
	for (UINT i = 0; i < nCount; ++i) {
		if (::PathIsDirectory(drop[i])) {
			dropFolders.emplace_back(drop[i]);
		} else if (PathIsImageFile(drop[i])) {
			dropFiles.emplace_back(drop[i]);
		}
	}
	dropFiles.sort([](const CString& path1, const CString& path2) {
		return ::StrCmpLogicalW(path1, path2) < 0;
	});

	while (dropFolders.size()) {
		std::list<CString> subFolders;
		dropFolders.sort([](const CString& path1, const CString& path2) {
			return ::StrCmpLogicalW(path1, path2) < 0;
		});
		for (auto& folder : dropFolders) {
			std::list<CString> folderFiles;
			ForEachFileFolder(folder, [&folderFiles, &subFolders](const CString& path, DWORD attributes) {
				if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
					if (PathIsImageFile(path)) {
						folderFiles.emplace_back(path);
					}
				} else if (CConfig::s_bAddImageOnSubFolder) {
					subFolders.emplace_back(path);
				}
			});
			folderFiles.sort([](const CString& path1, const CString& path2) {
				return ::StrCmpLogicalW(path1, path2) < 0;
			});
			dropFiles.splice(dropFiles.end(), folderFiles);
		}
		dropFolders = std::move(subFolders);
	}

	bool bFirst = m_imageList.empty();
	if (dropFiles.size() > 0) {
		for (auto& path : dropFiles)
			m_imageList.emplace_back(path);

		if (bFirst) {
			m_imagePos = 0;
			m_view.SetImage(m_imageList.front());
		}
		_ChangeWindowTitle( (wformat(L"%d のファイルが追加されました。") % dropFiles.size()).str().c_str() );
	}
}

void	CMainFrame::_ChangeWindowTitle(const CString& operateText /*= CString()*/)
{
	CString fileName;
	size_t imagePos = m_imagePos + 1;
	if (_IsValidPos(m_imagePos)) {
		fileName = GetFileOrFolderName(m_imageList[m_imagePos]);
	} else {
		imagePos = 0;
	}
	if (operateText.GetLength() > 0 && m_imageList.empty()) {
		SetWindowText(APP_NAME);
		return;
	}

	CString title = (wformat(L"[%d / %d] %s {%s}")
		% imagePos % m_imageList.size() % (LPCWSTR)fileName % (LPCWSTR)operateText).str().c_str();
	SetWindowText(title);
}

bool	CMainFrame::_IsValidPos(size_t nPos)
{
	return (0 <= nPos && nPos < m_imageList.size());
}

bool	CMainFrame::_PageImage(bool bNext)
{
	if (m_imageList.empty()) {
		m_view.SetImage(L"");
		m_imagePos = 0;
		return false;
	} else if (m_imageList.size() == 1) {
		m_imagePos = 0;
		if (m_view.SetImage(m_imageList.front())) {
			return true;
		} else {
			m_imageList.erase(m_imageList.begin());
			return false;
		}		
	}
	if (bNext) {
		if (_IsValidPos(m_imagePos + 1)) {
			++m_imagePos;
		} else {
			m_imagePos = 0;
		}
		for (;;) {
			if (_IsValidPos(m_imagePos) == false) {
				m_imagePos = 0;
				if (_IsValidPos(m_imagePos) == false) {
					m_view.SetImage(L"");
					return false;
				}
			}
			if (m_view.SetImage(m_imageList[m_imagePos]))
				return true;
			
			m_imageList.erase(std::next(m_imageList.begin(), m_imagePos));
		}
	} else {
		if (_IsValidPos(m_imagePos - 1)) {
			--m_imagePos;
		} else {
			m_imagePos = m_imageList.size() - 1;
		}
		for (;;) {
			if (_IsValidPos(m_imagePos) == false) {
				m_imagePos = m_imageList.size() - 1;
				if (_IsValidPos(m_imagePos) == false) {
					m_imagePos = 0;
					m_view.SetImage(L"");
					return false;
				}
			}
			if (m_view.SetImage(m_imageList[m_imagePos]))
				return true;

			m_imageList.erase(std::next(m_imageList.begin(), m_imagePos));
			--m_imagePos;
		}
	}

	;
}

void CMainFrame::OnPageImage(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_imageList.size() <= 1)
		return;

	switch (nID)
	{
	case ID_NEXT_IMAGE:
	{
		_PageImage(true);
	}
	break;

	case ID_PREV_IMAGE:
	{
		_PageImage(false);
	}
	break;
	}
	_ChangeWindowTitle();
}

void CMainFrame::OnUndo(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_history.empty())
		return;

	auto history = m_history.front();
	m_history.pop_front();

	if (CConfig::s_fileOperateMode == CConfig::kFileMove) {
		if (MoveFile(history.moveDestiny, history.moveSrc) == false) {
			MessageBox(L"移動に失敗しました。", L"エラー", MB_ICONERROR);
			return;
		}
	} else {
		if (::DeleteFile(history.moveDestiny) == FALSE) {
			MessageBox(L"削除に失敗しました。", L"エラー", MB_ICONERROR);
			return;
		}
	}

	if (_IsValidPos(m_imagePos) == false)
		m_imagePos = 0;

	m_imageList.emplace(std::next(m_imageList.begin(), m_imagePos), history.moveSrc);
	m_view.SetImage(m_imageList[m_imagePos]);

	CString name = GetFileOrFolderName(history.moveSrc);
	if (CConfig::s_fileOperateMode == CConfig::kFileMove) {
		CString srcFolderName = GetFileOrFolderName(GetParentFolderPath(history.moveSrc));		
		_ChangeWindowTitle((wformat(L"\"%1%\" を \"%2%\" へ戻しました。") % (LPCWSTR)name % (LPCWSTR)srcFolderName).str().c_str());
	} else {
		_ChangeWindowTitle((wformat(L"\"%1%\" を コピー先から削除しました。") % (LPCWSTR)name).str().c_str());
	}
}

void CMainFrame::OnDeleteImage(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (_IsValidPos(m_imagePos) == false)
		return;

	CString srcFilePath = m_imageList[m_imagePos];
	CString srcFileName = GetFileOrFolderName(srcFilePath);

	m_imageList.erase(std::next(m_imageList.begin(), m_imagePos));
	--m_imagePos;
	_PageImage(true);

	if (CConfig::s_fileDeleteMode == CConfig::kFileDeleteOnCycle) {
		std::wstring delFile = (LPCWSTR)srcFilePath;
		delFile.append(L"\0\0", 2);
		SHFILEOPSTRUCT sfos = {};
		sfos.wFunc = FO_DELETE;
		sfos.pFrom = delFile.c_str();
		sfos.fFlags = FOF_SILENT | FOF_ALLOWUNDO;
		int ret = ::SHFileOperation(&sfos);
		ATLASSERT(ret == 0);
		if (ret != 0)
			MessageBox(L"ごみ箱へ削除に失敗");
	} else if (CConfig::s_fileDeleteMode == CConfig::kFileDeleteDirect) {
		BOOL bRet = ::DeleteFile(srcFilePath);
		if (bRet == 0)
			MessageBox(L"ファイルの削除に失敗。");
	}

	if (CConfig::s_fileDeleteMode == CConfig::kFileDeleteOnCycle) {
		_ChangeWindowTitle((wformat(L"\"%1%\" を \"ごみ箱\" へ移動しました。") % (LPCWSTR)srcFileName).str().c_str());
	} else if (CConfig::s_fileDeleteMode == CConfig::kFileDeleteDirect) {
		_ChangeWindowTitle((wformat(L"\"%1%\" を削除しました。") % (LPCWSTR)srcFileName).str().c_str());
	} else {
		_ChangeWindowTitle((wformat(L"\"%1%\" をキューから取り除きました。") % (LPCWSTR)srcFileName).str().c_str());
	}
}

void	CMainFrame::_OnEnter(const CString& name, const CString& path)
{
	if (_IsValidPos(m_imagePos) == false)
		return;

	CString srcFilePath = m_imageList[m_imagePos];
	CString srcFileName = GetFileOrFolderName(srcFilePath);
	m_lastMoveDestinyFolderName = name;
	m_lastMoveDestinyFolder = AddBackslash(path);
	CString destinyPath = m_lastMoveDestinyFolder + srcFileName;
	if (::PathFileExists(destinyPath)) {
		int count = 1;
		CString baseName = GetPathBaseName(srcFileName);
		CString ext = PathFindExtention((LPCWSTR)srcFileName).c_str();
		CString destinyFolderPath = AddBackslash(path);
		for (;;) {
			CString destinyFileName;
			destinyFileName.Format(L"%s(%d).%s", (LPCWSTR)baseName, count, (LPCWSTR)ext);
			destinyPath = destinyFolderPath + destinyFileName;
			if (::PathFileExists(destinyPath) == FALSE)
				break;
			++count;
		}
	}

	m_imageList.erase(std::next(m_imageList.begin(), m_imagePos));
	--m_imagePos;
	_PageImage(true);

	CString operateMode;
	if (CConfig::s_fileOperateMode == CConfig::kFileMove) {
		operateMode = L"移動";
		if (MoveFile(srcFilePath, destinyPath) == false) {
			_ChangeWindowTitle((wformat(L"\"%1%\" の移動に失敗。") % (LPCWSTR)srcFileName).str().c_str());

			MessageBox(L"移動に失敗しました。", L"エラー", MB_ICONERROR);
			return;
		}
	} else {
		operateMode = L"コピー";
		if (::CopyFile(srcFilePath, destinyPath, TRUE) == FALSE) {
			_ChangeWindowTitle((wformat(L"\"%1%\" のコピーに失敗。") % (LPCWSTR)srcFileName).str().c_str());

			MessageBox(L"コピーに失敗しました。", L"エラー", MB_ICONERROR);
			return;
		}
	}

	m_history.emplace_front(CConfig::s_fileOperateMode, srcFilePath, destinyPath);

	_ChangeWindowTitle((wformat(L"\"%1%\" を \"%2%\" へ%3%しました。") % (LPCWSTR)srcFileName % (LPCWSTR)name % (LPCWSTR)operateMode).str().c_str());
}

void CMainFrame::OnMoveLastFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_lastMoveDestinyFolder.IsEmpty())
		return;

	_OnEnter(m_lastMoveDestinyFolderName, m_lastMoveDestinyFolder);
}

void CMainFrame::OnRenameImage(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (_IsValidPos(m_imagePos) == false)
		return;

	CString oldFileName = GetFileOrFolderName(m_imageList[m_imagePos]);
	CRenameDialog dlg(m_imageList[m_imagePos]);
	if (dlg.DoModal(m_hWnd) == IDOK) {
		m_imageList[m_imagePos] = dlg.m_filePath;
		CString newFileName = GetFileOrFolderName(dlg.m_filePath);
		_ChangeWindowTitle((wformat(L"\"%1%\" を \"%2%\" へリネームしました。") % (LPCWSTR)oldFileName % (LPCWSTR)newFileName).str().c_str());
	}
}

void CMainFrame::OnOpenImage(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (_IsValidPos(m_imagePos) == false)
		return;

	::ShellExecute(NULL, NULL, m_imageList[m_imagePos], NULL, NULL, SW_NORMAL);
}

void CMainFrame::OnClearQue(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_view.SetImage(L"");
	m_imageList.clear();
	m_imagePos = 0;
	m_lastMoveDestinyFolder.Empty();
	m_lastMoveDestinyFolderName.Empty();

	_ChangeWindowTitle(L"キューをクリアしました。");
}

void CMainFrame::OnShowConfigDialog(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CConfigSheet configDlg;
	configDlg.Show(m_hWnd);
}













