/**
*	@file	ImageArrangementView.cpp
*/

#include "stdafx.h"
#include "ImageArrangementView.h"
#include <stdexcept>
#include <fstream>
#include "Utility.h"

/// 最大サイズに収まるように画像の比率を考えて縮小する
CSize	CalcActualSize(Gdiplus::Image* image, int MaxImageWidth, int MaxImageHeight)
{
	CSize ActualSize;
	int nImageWidth = image->GetWidth();
	int nImageHeight = image->GetHeight();
	const int kMaxImageWidth = MaxImageWidth;//CThumbnailTooltipConfig::s_MaxThumbnailSize.cx;
	const int kMaxImageHeight = MaxImageHeight;//CThumbnailTooltipConfig::s_MaxThumbnailSize.cy;
	if (nImageWidth > kMaxImageWidth || nImageHeight > kMaxImageHeight) {
		if (nImageHeight > kMaxImageHeight) {
			ActualSize.cx = (int)((kMaxImageHeight / (double)nImageHeight) * nImageWidth);
			ActualSize.cy = kMaxImageHeight;
			if (ActualSize.cx > kMaxImageWidth) {
				ActualSize.cy = (int)((kMaxImageWidth / (double)ActualSize.cx) * ActualSize.cy);
				ActualSize.cx = kMaxImageWidth;
			}
		} else {
			ActualSize.cy = (int)((kMaxImageWidth / (double)nImageWidth) * nImageHeight);
			ActualSize.cx = kMaxImageWidth;
		}
	} else {
		ActualSize.cy = MaxImageHeight;
		ActualSize.cx = (int)((nImageWidth / (double)nImageHeight) * MaxImageHeight);
		if (ActualSize.cx > MaxImageWidth) {
			ActualSize.cx = MaxImageWidth;
			ActualSize.cy = (int)((nImageHeight / (double)nImageWidth) * MaxImageWidth);
		}
		//ActualSize.SetSize(nImageWidth, nImageHeight);
	}
	return ActualSize;
}

CComPtr<IStream>	CreateIStreamFromFile(const CString& filePath)
{
	std::ifstream fs((LPCWSTR)filePath, std::ios::in | std::ios::binary);
	if (!fs)
		return nullptr;

	fs.seekg(0, std::ios::end);
	auto filesize = fs.tellg().seekpos();
	fs.clear();
	fs.seekg(0, std::ios::beg);

	HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)filesize);
	if (hMem == NULL)
		return nullptr;

	char* pImage = (char*)::GlobalLock(hMem);
	fs.read(pImage, filesize);
	::GlobalUnlock(hMem);

	CComPtr<IStream> spStream;
	HRESULT hr = ::CreateStreamOnHGlobal(hMem, TRUE, &spStream);
	ATLASSERT(SUCCEEDED(hr));
	return spStream;
}


/////////////////////////////////////////////////////////////
// CImageArrangementView

bool	CImageArrangementView::SetImage(const CString& path)
{
	if (m_TimerID) {
		KillTimer(m_TimerID);
	}
	m_TimerID = 0;
	m_nFrameCount = 0;
	m_nFramePosition = 0;

	if (path.IsEmpty() || ::PathFileExists(path) == FALSE) {
		m_pImage.reset();
		Invalidate(FALSE);
		return false;
	}

	CComPtr<IStream> spFileStream = CreateIStreamFromFile(path);
	if (spFileStream == nullptr) {
		m_pImage.reset();
		Invalidate(FALSE);
		return false;
	}

	m_pImage.reset(Gdiplus::Image::FromStream(spFileStream));

	if (PathFindExtention((LPCWSTR)path)  == L"gif") {
		GUID	guid;
		m_pImage->GetFrameDimensionsList(&guid, 1);
		m_nFrameCount = (int)m_pImage->GetFrameCount(&guid);
		if (m_nFrameCount > 1) {
			m_vecDelayTime.clear();

			UINT propItemSize = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay);			
			Gdiplus::PropertyItem* propItems = (Gdiplus::PropertyItem*)new BYTE[propItemSize];
			m_pImage->GetPropertyItem(PropertyTagFrameDelay, propItemSize, propItems);
			for (int i = 0; i < m_nFrameCount; ++i) {
				int nDelay = ((long*)propItems->value)[i] * 10;
				if (nDelay == 0)
					nDelay = 100;
				m_vecDelayTime.push_back(nDelay);
			}
			delete[] (BYTE*)propItems;

			m_TimerID = SetTimer(kGifTimerId, m_vecDelayTime.front());
		}
	}
	Invalidate(FALSE);
	UpdateWindow();
	return true;
}

void CImageArrangementView::DoPaint(CDCHandle dc)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	dc.FillSolidRect(&rcClient, RGB(0, 0, 0));

	if (m_pImage) {
		Gdiplus::Image& image = *m_pImage;
		CSize imagesize = CalcActualSize(&image, rcClient.Width(), rcClient.Height());
		Gdiplus::Rect rcImage;
		rcImage.X = (rcClient.Width() - imagesize.cx) / 2;
		rcImage.Y = (rcClient.Height() - imagesize.cy) / 2;
		rcImage.Width = imagesize.cx;
		rcImage.Height = imagesize.cy;

		if (m_TimerID) {
			GUID	guid;
			image.GetFrameDimensionsList(&guid, 1);
			image.SelectActiveFrame(&guid, m_nFramePosition);
		}

		Gdiplus::Graphics graphics(dc);
		graphics.DrawImage(&image, rcImage);
	} else {
		CLogFont lf;
		lf.SetMenuFont();
		CFont font = lf.CreateFontIndirect();
		HFONT hPrevFont = dc.SelectFont(font);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(255, 255, 255));

		dc.DrawText(L"画像ファイルまたはフォルダをドロップしてください。", -1, rcClient, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		dc.SelectFont(hPrevFont);
	}
}


void CImageArrangementView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_funcShowCandidateWindow(nChar);
}

void CImageArrangementView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == kGifTimerId && m_pImage) {
		++m_nFramePosition;
		if (m_nFramePosition == m_nFrameCount)
			m_nFramePosition = 0;
		m_TimerID = SetTimer(kGifTimerId, m_vecDelayTime[m_nFramePosition]);
		Invalidate(FALSE);
	}
}


















