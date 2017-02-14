#include "stdafx.h"
#include "AboutWnd.h"

CAboutWnd::CAboutWnd() 
{
}

CAboutWnd::~CAboutWnd()
{
}

void CAboutWnd::Init()
{
	Create(g_hMainWnd, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	CenterWindow();

	CLabelUI *lpProduct = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("ProductName")));
	lpProduct->SetText(_T("晨光PDF转Word"));
// 	m_lpTabOpt1 = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("tabLayout")));
}

void CAboutWnd::OnFinalMessage( HWND /*hWnd*/ )
{
	delete this;
}

// LRESULT CAboutWnd::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
// {
// 	LRESULT lRes = 0;
// 	BOOL bHandled = TRUE;
// 
// 	switch( uMsg )
// 	{
// 	case WM_KILLFOCUS:    
// // 		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); 
// 		break; 
// 
// 	default:
// 		bHandled = FALSE;
// 	}
// 
// 	if(bHandled || m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes)) 
// 	{
// 		return lRes;
// 	}
// 
// 	return __super::HandleMessage(uMsg, wParam, lParam);
// }

void CAboutWnd::Notify(TNotifyUI& msg)
{
	if(msg.sType == _T("click")) 
	{
		CString strName = msg.pSender->GetName();
		if (_T("btnClose") == strName)		//关闭对话框
		{
			Close();
		}
		else if (_T("buttonSite") == strName)
		{
			//打开网页
			CButtonUI *lpBtnUrl = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("buttonSite")));
			CString strUrl = lpBtnUrl->GetText();

			ShellExecute(NULL, _T("open"), strUrl.GetBuffer(0), NULL, NULL, SW_SHOWNORMAL);
		}
	}

	__super::Notify(msg);
};
