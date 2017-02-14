#include "stdafx.h"
#include "MenuWnd.h"
#include "MsgWnd.h"
#include "SetWnd.h"
#include "FeedBackWnd.h"
#include "AboutWnd.h"

//弹出设置转换属性窗口
extern unsigned __stdcall PopSetWndFunc(LPVOID pM);

CMainMenuWnd::CMainMenuWnd() 
{

}

CMainMenuWnd::~CMainMenuWnd()
{

}

void CMainMenuWnd::Init(HWND hWndParent, POINT ptPos)
{
    Create(hWndParent, _T(""), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
    ::ClientToScreen(hWndParent, &ptPos);
    ::SetWindowPos(*this, NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void CMainMenuWnd::OnFinalMessage( HWND /*hWnd*/ )
{
    delete this;
}

LRESULT CMainMenuWnd::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lRes = 0;
    BOOL bHandled = TRUE;

    switch( uMsg )
    {
    case WM_KILLFOCUS:    
        lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); 
        break; 

    default:
        bHandled = FALSE;
    }

    if(bHandled || m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes)) 
    {
        return lRes;
    }

    return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMainMenuWnd::OnKillFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    Close();
    bHandled = FALSE;
    return __super::OnKillFocus(uMsg, wParam, lParam, bHandled); 
}

/*
函数功能：对指定文件在指定的目录下创建其快捷方式
函数参数：
lpszFileName    指定文件，为NULL表示当前进程的EXE文件。
lpszLnkFileDir  指定目录，不能为NULL。
lpszLnkFileName 快捷方式名称，为NULL表示EXE文件名。
wHotkey         为0表示不设置快捷键
lpszComLine		参数
pszDescription  备注
iShowCmd        运行方式，默认为常规窗口
*/
BOOL CreateFileShortcut(LPTSTR lpszFileName, LPTSTR lpszLnkFileDir, LPTSTR lpszLnkFileName, LPTSTR lpszWorkDir, 
						WORD wHotkey, LPTSTR lpszIco, LPTSTR lpszComLine, LPTSTR lpszDescription, int iShowCmd = SW_SHOWNORMAL)
{
	if (lpszLnkFileDir == NULL)
		return FALSE;

	HRESULT hr;
	IShellLink     *pLink;  //IShellLink对象指针
	IPersistFile   *ppf; //IPersisFil对象指针

	CoInitialize(NULL);

	//创建IShellLink对象
	hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
	if (FAILED(hr))
		return FALSE;

	//从IShellLink对象中获取IPersistFile接口
	hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if (FAILED(hr))
	{
		pLink->Release();
		return FALSE;
	}

	//目标
	if (lpszFileName == NULL)
		pLink->SetPath(_tpgmptr);
	else
	{
		pLink->SetPath(lpszFileName);
	}

	if (lpszIco != NULL)
	{
		pLink->SetIconLocation(lpszIco, 0);
	}

	//工作目录
	if (lpszWorkDir != NULL)
		pLink->SetPath(lpszWorkDir);

	//快捷键
	if (wHotkey != NULL)
		pLink->SetHotkey(wHotkey);

	//参数
	if (lpszComLine != NULL)
	{
		pLink->SetArguments(lpszComLine);
	}
	//备注
	if (lpszDescription != NULL)
		pLink->SetDescription(lpszDescription);

	//显示方式
	pLink->SetShowCmd(iShowCmd);

	//快捷方式的路径 + 名称
	TCHAR szBuffer[MAX_PATH];
	if (lpszLnkFileName != NULL) //指定了快捷方式的名称
		_stprintf(szBuffer, _T("%s\\%s"), lpszLnkFileDir, lpszLnkFileName);
	else  
	{
		//没有指定名称，就从取指定文件的文件名作为快捷方式名称。
		TCHAR *pstr;
		if (lpszFileName != NULL)
			pstr = _tcsrchr(lpszFileName, _T('\\'));
		else
			pstr = _tcsrchr(_tpgmptr, _T('\\'));

		if (pstr == NULL)
		{    
			ppf->Release();
			pLink->Release();
			return FALSE;
		}
		//注意后缀名要从.exe改为.lnk
		_stprintf(szBuffer, _T("%s\\%s"), lpszLnkFileDir, pstr);
		int nLen = _tcsclen(szBuffer);
		szBuffer[nLen - 3] = _T('l');
		szBuffer[nLen - 2] = _T('n');
		szBuffer[nLen - 1] = _T('k');
	}

	//保存快捷方式到指定目录下
#ifdef _UNICODE
	hr = ppf->Save(szBuffer, TRUE);
#else
	WCHAR  wsz[MAX_PATH];  //定义Unicode字符串
	MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, wsz, MAX_PATH);
	hr = ppf->Save(wsz, TRUE);
#endif

	ppf->Release();
	pLink->Release();

	CoUninitialize();

	return SUCCEEDED(hr);
}

//设置桌面快捷方式
void SetLnkOnDesk()
{
	TCHAR szPath[MAX_PATH] = _T("");
	SHGetSpecialFolderPath(0,szPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
	CString strDeskPath = szPath;

	CreateFileShortcut(NULL, strDeskPath.GetBuffer(0),
					_T("晨光PDF转Word.lnk"), NULL, 0, NULL, NULL, _T("晨光PDF转Word"));
}

unsigned __stdcall PopFeedBackWndFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*)pM;

	CFeedBackWnd *lpFeedback = new CFeedBackWnd;
	lpFeedback->Init();
	lpFeedback->ShowModal();

	_endthreadex(0);
	return 0;
}

unsigned __stdcall PopAboutWndFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*)pM;

	CAboutWnd *lpAboutWnd = new CAboutWnd;
	lpAboutWnd->Init();
	lpAboutWnd->ShowModal();

	_endthreadex(0);
	return 0;
}

//
void CMainMenuWnd::Notify(TNotifyUI& msg)
{
	if(msg.sType == _T("itemclick")) 
	{
		CString strName = msg.pSender->GetName();
		if (_T("menuCreateSC") == strName)		//设置桌面快捷方式
		{
			SetLnkOnDesk();
			Close();
		}
		else if (_T("menuUpdate") == strName)	//检查更新
		{
// 			Close();

			ShowMsgTipWnd(g_hMainWnd, _T("已经是最新版本"), 0);
// 			CMsgWnd *lpMsgWnd = new CMsgWnd;
// 			lpMsgWnd->Init(g_hMainWnd, _T("已经是最新版本"), 0);
// 			lpMsgWnd->ShowWindow();
		}
		else if (_T("menuSetting") == strName)		//转换效果设置
		{
			_beginthreadex(NULL, 0, PopSetWndFunc, (LPVOID)g_MainWnd, 0, NULL);
		}
		else if (_T("menuFeedback") == strName)		//意见反馈
		{
// 			Close();
			_beginthreadex(NULL, 0, PopFeedBackWndFunc, (LPVOID)g_MainWnd, 0, NULL);
		}
		else if (_T("menuAbout") == strName)		//关于我们
		{
// 			Close();
			_beginthreadex(NULL, 0, PopAboutWndFunc, (LPVOID)g_MainWnd, 0, NULL);
		}
	}

	__super::Notify(msg);
};
