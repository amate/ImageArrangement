/**
*	@file	CandidateWindow.h
*/

#pragma once

#include <list>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include <atlframe.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlsync.h>
#include "Migemo.h"

class CCandidateWindow : 
	public CDoubleBufferWindowImpl<CCandidateWindow>,
	public CMessageFilter
{
public:
	enum {
		kWindowWidth = 130,
		kEditHeight = 24,
		kLineHeight = 1,
		kCandidateHeight = 24,
		kLeftPadding = 2,

		WM_SETCANDIDATELIST = WM_APP + 100,
	};

	CCandidateWindow();
	~CCandidateWindow();

	void	SetViewWindow(HWND hWnd) {
		m_view = hWnd;
	}
	void	SetOnEnterFunc(std::function<void (const CString&, const CString&)> func) {
		m_funcOnEnter = func;
	}
	void	SetOnDeleteFunc(std::function<void()> func) {
		m_funcOnDelete = func;
	}

	void	ShowCandidateWindow(UINT nChar);

	// overrides
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
	void DoPaint(CDCHandle dc);

	BEGIN_MSG_MAP_EX(CCandidateWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ACTIVATE(OnActivate)
		MSG_WM_MOUSEACTIVATE(OnMouseActivate)
		COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnEditChanged)
		MESSAGE_HANDLER_EX(WM_SETCANDIDATELIST, OnSetCandidateList)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		CHAIN_MSG_MAP(CDoubleBufferWindowImpl<CCandidateWindow>)
	ALT_MSG_MAP(1)

	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther);
	int OnMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message);
	void OnEditChanged(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnSetCandidateList(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnRButtonDown(UINT nFlags, CPoint point);

private:
	void	_CreateCandidateList();
	void	_HideCandidateWindow();

	CWindow	m_view;
	std::function<void (const CString&, const CString&)> m_funcOnEnter;
	std::function<void()>	m_funcOnDelete;
	CContainedWindowT<CEdit>	m_edit;
	CMigemo	m_migemo;

	CFont	m_font;

	struct Candidate
	{
		bool	active;
		CRect	rcItem;
		CString	name;
		CString	path;

		Candidate(const std::wstring& name, const std::wstring& path) : 
			active(false), name(name.c_str()), path(path.c_str()) 
		{}
	};
	std::list<Candidate> m_candidateList;

	struct CandidateEnumerate {
		std::atomic_bool active;
		std::thread thread;

		CandidateEnumerate() {
			active = true;
		}
	};
	// CCritSecLock lock(m_csCandidateEnumerateThreadList);
	CCriticalSection m_csCandidateEnumerateThreadList;
	std::list<CandidateEnumerate>	m_candidateEnumerateThreadList;
};

