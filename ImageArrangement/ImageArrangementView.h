// ImageArrangementView.h : interface of the CImageArrangementView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlscrl.h>
#include "resource.h"
#include "GdiplusUtil.h"


class CImageArrangementView : public CDoubleBufferWindowImpl<CImageArrangementView>
{
public:
	DECLARE_WND_CLASS(NULL)

	enum {
		kGifTimerId = 1,
	};

	void	SetShowCandidateWindowFunc(std::function<void(UINT)> func) {
		m_funcShowCandidateWindow = func;
	}

	bool	SetImage(const CString& path);

	// overrides
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	void DoPaint(CDCHandle dc);

	BEGIN_MSG_MAP(CImageArrangementView)
		MSG_WM_CHAR( OnChar )
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(CDoubleBufferWindowImpl<CImageArrangementView>)
	END_MSG_MAP()

	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnTimer(UINT_PTR nIDEvent);

private:
	std::unique_ptr<Gdiplus::Image>	m_pImage;
	std::function<void(UINT)>	m_funcShowCandidateWindow;

	UINT_PTR	m_TimerID;
	int m_nFrameCount;
	int	m_nFramePosition;
	std::vector<int>	m_vecDelayTime;
};
