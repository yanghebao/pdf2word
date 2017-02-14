#pragma once

// 设置转换页数窗口类
class CPageRangeWnd: public WindowImplBase
{
public:
	explicit CPageRangeWnd();

protected:
	virtual ~CPageRangeWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("PageRange_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_pagerange.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init(int nFilePages);
	virtual void    OnFinalMessage(HWND hWnd);
	// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	int m_nFilePages;
	CEditUI *m_lpMinEdit;
	CEditUI *m_lpMaxEdit;
};
