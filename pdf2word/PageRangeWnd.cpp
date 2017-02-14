#include "stdafx.h"
#include "PageRangeWnd.h"

CString g_strPageRange;	//设置页数范围

CPageRangeWnd::CPageRangeWnd() 
{
}

CPageRangeWnd::~CPageRangeWnd()
{
}

void CPageRangeWnd::Init(int nFilePages)
{
	Create(g_hMainWnd, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	CenterWindow();

	m_nFilePages = nFilePages;

	m_lpMinEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("editStart")));
	m_lpMaxEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("editEnd")));

	CString strPages;
	strPages.Format(_T("%d"), nFilePages);
	m_lpMinEdit->SetText(_T("1"));
	m_lpMaxEdit->SetText(strPages.GetBuffer(0));
}

void CPageRangeWnd::OnFinalMessage( HWND /*hWnd*/ )
{
	delete this;
}

void CPageRangeWnd::Notify(TNotifyUI& msg)
{
	if(msg.sType == _T("click")) 
	{
		CString strName = msg.pSender->GetName();
		if (_T("btnClose") == strName)		//关闭
		{
			Close(IDCANCEL);
		}
		else if (_T("btnOK") == strName)	//确定 
		{
			CString strStart = m_lpMinEdit->GetText();
			CString strEnd = m_lpMaxEdit->GetText();

			int nStart = _ttoi(strStart.GetBuffer(0));
			int nEnd = _ttoi(strEnd.GetBuffer(0));

			if (nStart > m_nFilePages || nEnd > m_nFilePages)
			{
				return ;
			}

			if (nStart > nEnd)
			{
				return;
			}

			g_strPageRange.Format(_T("%s-%s"), strStart, strEnd);

			Close(IDOK);
		}
	}

	__super::Notify(msg);
};
