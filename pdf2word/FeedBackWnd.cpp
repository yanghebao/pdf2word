#include "stdafx.h"
#include "FeedBackWnd.h"
#include "MsgWnd.h"
#include "HttpTransByPost.h"

CFeedBackWnd::CFeedBackWnd() 
{
}

CFeedBackWnd::~CFeedBackWnd()
{
}

void CFeedBackWnd::Init()
{
	Create(g_hMainWnd, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	CenterWindow();

	m_edtFeedback = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("Feedback")));
	m_edtContact = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("Feedback")));
}

void CFeedBackWnd::OnFinalMessage( HWND /*hWnd*/ )
{
	delete this;
}

// LRESULT CFeedBackWnd::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
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

// 进行Url编码 UTF-8 
CString UrlEncode(CString strUnicode)
{
	LPCWSTR unicode = T2CW(strUnicode);
	int len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, 0, 0, 0, 0);
	if (!len)
		return strUnicode;
	char *utf8 = new char[len + 1];
	memset(utf8, 0, len + 1);
	char *utf8temp = utf8;
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8, len + 1, 0, 0);
	utf8[len] = NULL;   
	CString strTemp, strEncodeData; 
	while (*utf8 != 0) 
	{ 
		strTemp.Format(_T("%%%2x"), (BYTE)*utf8); 
		strEncodeData += strTemp; 
		++utf8; 
	} 

	delete []utf8temp;

	return CString(strEncodeData);
}

CString g_strTipMsg;
int g_nTipTag = -1;
unsigned __stdcall ShowMsgTipWndFunc(LPVOID pM)
{
	HWND hWnd = (HWND)pM;
	CMsgWnd *lpMsg = new CMsgWnd;
	lpMsg->Init((HWND)hWnd, g_strTipMsg.GetBuffer(0), g_nTipTag);
	lpMsg->ShowModal();

	_endthreadex(0);
	return 0;
}

void ShowMsgTipWnd(HWND hwnd, CString strMsg, int nTag)
{
	g_strTipMsg = strMsg;
	g_nTipTag = nTag;
	_beginthreadex(NULL, 0, ShowMsgTipWndFunc, (void*)hwnd, 0, NULL);
}

void CFeedBackWnd::Notify(TNotifyUI& msg)
{
	if(msg.sType == _T("click")) 
	{
		CString strName = msg.pSender->GetName();
		if (_T("btnSubmit") == strName)		//提交反馈信息
		{
			Close();

			// 获取信息
			CString strFeedback = m_edtFeedback->GetText();
			CString strPhone = m_edtContact->GetText();

			CString strVipType;
			strVipType.Format(_T("%d"), g_nVIPType);

			SYSTEMTIME st;
			GetSystemTime(&st);
			CTime tUTC(st);
			CString strTime;
			strTime.Format(_T("%d"), tUTC.GetTime());

			CString strSign;
			strSign.Format(_T("%s|quxiupdf2word"), strTime);
			CString strSignMd5 = MakeCStringMD5W(strSign);
 
			CString strUrl = _T("http://www.pdf00.com/feedback/add?content=[content]&contact=[contact]&time=[time]&sign=[sign]");
			strUrl.Replace(_T("[content]"), strVipType);
			strUrl.Replace(_T("[contact]"), g_strMachineCode);
			strUrl.Replace(_T("[time]"), strTime);
			strUrl.Replace(_T("[sign]"), strSignMd5);

			CString strAllInfo = strFeedback + _T("|") + strPhone;
			CStringA strAllInfoA = CStrW2CStrA(strAllInfo);

			CHttpTransByPost httpPost;
			VecStParam vecParam;
			char szBuf[1024] = "";
			strcpy(szBuf, strAllInfoA.GetBuffer(0));
			DWORD dwInfoLen = 1024;
			httpPost.TransDataToServer(strUrl.GetBuffer(0), 3000, vecParam, szBuf, dwInfoLen);

			char *lp = UTF_8ToGB2312(szBuf, dwInfoLen);
			CStringA strHtml_G = lp;
			delete []lp;
			CString strHtml = CStrA2CStrW(strHtml_G);

			// 谢谢反馈
			ShowMsgTipWnd(this->GetHWND(), _T("谢谢您的宝贵意见，我们会努力改进的！！"), -1);
// 			CMsgWnd *lpMsg = new CMsgWnd;
// 			lpMsg->Init(_T("谢谢您的宝贵意见，我们会努力改进的！！"), -1);
		}
		else if (_T("btnClose") == strName)
		{
			Close();
		}
	}

	__super::Notify(msg);
};
