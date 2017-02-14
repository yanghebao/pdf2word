#pragma once

// 主界面菜单类
class CMainMenuWnd: public WindowImplBase
{
public:
    explicit CMainMenuWnd();

protected:
    virtual ~CMainMenuWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("Main_menu_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("main_menu.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init(HWND hWndParent, POINT ptPos);
    virtual void    OnFinalMessage(HWND hWnd);
    virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnKillFocus   (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
