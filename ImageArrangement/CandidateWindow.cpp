/**
*	@file	CandidateWindow.cpp
*/

#include "stdafx.h"
#include "CandidateWindow.h"
#include "ConfigDialog.h"
#include <regex>
#include <chrono>
using namespace std::chrono;

#include "resource.h"

////////////////////////////////////////////////////////////////////
// CCandidateWindow

CCandidateWindow::CCandidateWindow()
{
}


CCandidateWindow::~CCandidateWindow()
{
}

BOOL CCandidateWindow::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		UINT nKey = (UINT)pMsg->wParam;

		CRect rcClient;
		m_view.GetClientRect(&rcClient);
		rcClient.top = kEditHeight;
		rcClient.right = kWindowWidth;

		if (nKey == VK_DOWN) {
			if (m_candidateList.size() <= 1)
				return FALSE;

			for (auto it = m_candidateList.begin(); it != m_candidateList.end(); ++it) {
				if (it->active) {
					it->active = false;
					if (std::next(it) != m_candidateList.end()) {
						auto itnext = std::next(it);
						itnext->active = true;
						rcClient.InflateRect(1, 1);
						if (rcClient.PtInRect(itnext->rcItem.TopLeft()) == FALSE ||
							rcClient.PtInRect(itnext->rcItem.BottomRight()) == FALSE)
						{
							rcClient.InflateRect(-1, -1);
							for (auto& candidate : m_candidateList)
								candidate.rcItem.SetRectEmpty();

							int top = rcClient.bottom - kCandidateHeight;
							auto distance = std::distance(m_candidateList.begin(), itnext);
							auto rit = std::next(m_candidateList.rbegin(), m_candidateList.size() - distance - 1);
							for (; rit != m_candidateList.rend(); ++rit) {
								CRect rcItem;
								rcItem.top = top;
								rcItem.left = 0;
								rcItem.right = kWindowWidth;
								rcItem.bottom = top + kCandidateHeight;
								top -= kCandidateHeight + kLineHeight;
								rit->rcItem = rcItem;
							}						
						}
					} else {
						m_candidateList.begin()->active = true;
						rcClient.InflateRect(1, 1);
						if (rcClient.PtInRect(m_candidateList.begin()->rcItem.TopLeft()) == FALSE ||
							rcClient.PtInRect(m_candidateList.begin()->rcItem.BottomRight()) == FALSE)
						{
							rcClient.InflateRect(-1, -1);
							int top = kEditHeight;
							for (auto& candidate : m_candidateList) {
								CRect rcItem;
								rcItem.top = top;
								rcItem.left = 0;
								rcItem.right = kWindowWidth;
								rcItem.bottom = top + kCandidateHeight;
								top += kCandidateHeight + kLineHeight;
								candidate.rcItem = rcItem;
							}
						}
					}
					InvalidateRect(rcClient, FALSE);
					return TRUE;
				}

			}
		} else if (nKey == VK_UP) {
			if (m_candidateList.size() <= 1)
				return FALSE;
			for (auto it = m_candidateList.rbegin(); it != m_candidateList.rend(); ++it) {
				if (it->active) {
					it->active = false;
					if (std::next(it) != m_candidateList.rend()) {
						auto itnext = std::next(it);
						itnext->active = true;
						rcClient.InflateRect(1, 1);
						if (rcClient.PtInRect(itnext->rcItem.TopLeft()) == FALSE ||
							rcClient.PtInRect(itnext->rcItem.BottomRight()) == FALSE)
						{
							rcClient.InflateRect(-1, -1);
							for (auto& candidate : m_candidateList)
								candidate.rcItem.SetRectEmpty();

							int top = kEditHeight;
							auto distance = std::distance(m_candidateList.rbegin(), itnext);
							auto rit = std::next(m_candidateList.begin(), m_candidateList.size() - distance - 1);
							for (; rit != m_candidateList.end(); ++rit) {
								CRect rcItem;
								rcItem.top = top;
								rcItem.left = 0;
								rcItem.right = kWindowWidth;
								rcItem.bottom = top + kCandidateHeight;
								top += kCandidateHeight + kLineHeight;
								rit->rcItem = rcItem;
							}
						}
					} else {
						m_candidateList.rbegin()->active = true;
						rcClient.InflateRect(1, 1);
						if (rcClient.PtInRect(m_candidateList.rbegin()->rcItem.TopLeft()) == FALSE ||
							rcClient.PtInRect(m_candidateList.rbegin()->rcItem.BottomRight()) == FALSE)
						{
							rcClient.InflateRect(-1, -1);
							int top = rcClient.bottom - kCandidateHeight;
							for (auto rit = m_candidateList.rbegin(); rit != m_candidateList.rend(); ++rit) {
								auto& candidate = *rit;
								CRect rcItem;
								rcItem.top = top;
								rcItem.left = 0;
								rcItem.right = kWindowWidth;
								rcItem.bottom = top + kCandidateHeight;
								top -= kCandidateHeight + kLineHeight;
								candidate.rcItem = rcItem;
							}
						}
					}
					InvalidateRect(rcClient, FALSE);
					return TRUE;
				}

			}
		} else if (nKey == VK_RIGHT) {
			if (GetFocus() != m_edit) {
				m_view.GetParent().SendMessage(WM_COMMAND, ID_NEXT_IMAGE);
				return TRUE;
			}

		} else if (nKey == VK_LEFT) {
			if (GetFocus() != m_edit) {
				m_view.GetParent().SendMessage(WM_COMMAND, ID_PREV_IMAGE);
				return TRUE;
			}

		} else if (nKey == VK_ESCAPE) {
			if (IsWindowVisible()) {
				_HideCandidateWindow();
				return TRUE;
			}
		} else if (nKey == VK_RETURN) {
			if (IsWindowVisible()) {
				for (auto& candidate : m_candidateList) {
					if (candidate.active) {
						m_funcOnEnter(candidate.name, candidate.path);
						break;
					}
				}
				_HideCandidateWindow();
				return TRUE;
			}
		} else if (nKey == VK_DELETE) {
			if (GetFocus() != m_edit) {
				m_funcOnDelete();
				return TRUE;
			}
		}

	} else if (pMsg->message == WM_MOUSEWHEEL) {
		short zDelta = (short)HIWORD(pMsg->wParam);
		if (zDelta < 0) {
			m_view.GetParent().SendMessage(WM_COMMAND, ID_NEXT_IMAGE);
		} else {
			m_view.GetParent().SendMessage(WM_COMMAND, ID_PREV_IMAGE);
		}
		return TRUE;
	}
	return FALSE;
}

void CCandidateWindow::DoPaint(CDCHandle dc)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	dc.FillSolidRect(&rcClient, RGB(255, 255, 255));

	HFONT hPrevFont = dc.SelectFont(m_font);

	static COLORREF BorderColor = ::GetSysColor(COLOR_3DLIGHT);
	CPen hPen = ::CreatePen(PS_SOLID, 1, BorderColor);
	HPEN hOldPen = dc.SelectPen(hPen);

	for (auto it = m_candidateList.begin(); it != m_candidateList.end(); ++it) {
		auto& candidate = *it;

		dc.SetBkMode(TRANSPARENT);
		if (candidate.active) {
			static COLORREF SelectColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			dc.SetTextColor(SelectColor);
			dc.FillRect(&candidate.rcItem, COLOR_HIGHLIGHT);	// ‘I‘ð—ñ•`‰æ
		} else {
			dc.SetTextColor(0);
		}

		CRect rcText = candidate.rcItem;
		rcText.left = kLeftPadding;
		dc.DrawText(candidate.name, candidate.name.GetLength(), &rcText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_VCENTER);

		// ‰º‚Éƒ‰ƒCƒ“‚ðˆø‚­
		dc.MoveTo(CPoint(candidate.rcItem.left, candidate.rcItem.bottom));
		dc.LineTo(candidate.rcItem.right, candidate.rcItem.bottom);		
	}

	dc.SelectPen(hOldPen);
	dc.SelectFont(hPrevFont);
}


int CCandidateWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect rcEdit;
	rcEdit.right = kWindowWidth - 1;
	rcEdit.bottom = kEditHeight;
	m_edit.Create(this, 1, m_hWnd, &rcEdit, nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER);
	CLogFont lf;
	lf.SetMenuFont();
	m_font = lf.CreateFontIndirect();
	m_edit.SetFont(m_font);

	::ImmAssociateContext(m_edit, NULL);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

	return 0;
}

void CCandidateWindow::OnDestroy()
{
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
}

void CCandidateWindow::OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther)
{
	if (nState == WA_INACTIVE) {
		_HideCandidateWindow();
	}
	return;
}

void	CCandidateWindow::_HideCandidateWindow()
{
	ShowWindow(FALSE);
	m_edit.SetWindowText(L"");
}

int CCandidateWindow::OnMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message)
{
	return MA_NOACTIVATE;
}

void CCandidateWindow::OnEditChanged(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	_CreateCandidateList();
}

LRESULT CCandidateWindow::OnSetCandidateList(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_candidateList = std::move(*(std::list<Candidate>*)wParam);
	if (m_candidateList.size())
		m_candidateList.front().active = true;

	CRect rcClient;
	m_view.GetClientRect(&rcClient);

	int	top = kEditHeight;
	for (auto& candidate : m_candidateList) {
		CRect rcItem;
		rcItem.top = top;
		rcItem.left = 0;
		rcItem.right = kWindowWidth;
		rcItem.bottom = top + kCandidateHeight;
		top = rcItem.bottom + kLineHeight;
		candidate.rcItem = rcItem;
	}

	int windowHeight;
	if (m_candidateList.size()) {
		windowHeight = std::min(rcClient.bottom, m_candidateList.back().rcItem.bottom);
	} else {
		windowHeight = kEditHeight;
	}
	SetWindowPos(NULL, 0, 0, kWindowWidth, windowHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

	Invalidate(FALSE);
	return 0;
}

void CCandidateWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
	for (auto& candidate : m_candidateList) {
		if (candidate.rcItem.PtInRect(point)) {
			::ShellExecute(NULL, L"open", candidate.path, nullptr, nullptr, SW_NORMAL);
			return;
		}
	}
}


void	CCandidateWindow::ShowCandidateWindow(UINT nChar)
{
	if (nChar == L'\b') {
		CString text;
		m_edit.GetWindowText(text.GetBuffer(256), 256);
		text.ReleaseBuffer();
		if (text.GetLength() > 0) {
			text.Delete(text.GetLength() - 1);
			m_edit.SetWindowText(text);
		}
		return;
	}
	if (iswgraph((WCHAR)nChar) == 0)
		return;

	ShowWindow(SW_SHOWNOACTIVATE);
	WCHAR str[] = { (WCHAR)nChar, L'\0' };
	m_edit.AppendText(str);
}

void	CCandidateWindow::_CreateCandidateList()
{
	auto begin = steady_clock::now();

	CCritSecLock lock(m_csCandidateEnumerateThreadList);
	for (auto& enumThread : m_candidateEnumerateThreadList) {
		enumThread.active = false;
	}

	CString query;
	m_edit.GetWindowText(query.GetBuffer(256), 256);
	query.ReleaseBuffer();
	if (query.IsEmpty()) {
		int windowHeight = kEditHeight;
		SetWindowPos(NULL, 0, 0, kWindowWidth, windowHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		return;
	}
	m_candidateEnumerateThreadList.emplace_front();
	auto itThis = m_candidateEnumerateThreadList.begin();
	lock.Unlock();

	std::wstring pattern = m_migemo.GetRegexPattern((LPCWSTR)query);

	itThis->thread = std::thread([this, pattern, itThis]() {
		auto funcThrowOnInactive = [itThis]() {
			if (itThis->active == false)
				throw std::runtime_error("inactive");
		};
		try {
			
			funcThrowOnInactive();
			std::list<Candidate> candidateList;

			auto cndList = CConfig::s_candidateNameFolderList;
			std::wregex rxfirstMatch(L"^" + pattern, std::regex_constants::icase);
			for (auto it = cndList.begin(); it != cndList.end();) {
				funcThrowOnInactive();
				auto& candidateData = *it;
				std::wsmatch result;
				if (std::regex_search(candidateData.name, result, rxfirstMatch)) {
					candidateList.emplace_back(candidateData.name, candidateData.path);
					it = cndList.erase(it);
					continue;
				}
				++it;
			}
			std::wregex rx(pattern, std::regex_constants::icase);
			for (auto& candidateData : cndList) {
				funcThrowOnInactive();
				std::wsmatch result;
				if (std::regex_search(candidateData.name, result, rx)) {
					candidateList.emplace_back(candidateData.name, candidateData.path);
				}
			}
			funcThrowOnInactive();
			SendMessage(WM_SETCANDIDATELIST, (WPARAM)&candidateList);
		}
		catch (...) {
			//MessageBox(_T("regexPatternError"));
			//ShowWindow(FALSE);
		}
		CCritSecLock lock(m_csCandidateEnumerateThreadList);
		itThis->thread.detach();
		m_candidateEnumerateThreadList.erase(itThis);
	});
	auto duringTime = duration_cast<milliseconds>(steady_clock::now() - begin).count();
	ATLTRACE(L"duringTime : %lld ms [%s]\n", duringTime, (LPCWSTR)query);
}













