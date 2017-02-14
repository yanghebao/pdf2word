#pragma once

// 关于我们窗口类
class CAboutWnd: public WindowImplBase
{
public:
	explicit CAboutWnd();

protected:
	virtual ~CAboutWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("About_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_about.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init();
	virtual void    OnFinalMessage(HWND hWnd);
	// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

};
