#pragma once

// 输入密码窗口类
class CInputPWWnd: public WindowImplBase
{
public:
	explicit CInputPWWnd();

protected:
	virtual ~CInputPWWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("InputPW_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_inputpassword.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init(int nIndex);
	virtual void    OnFinalMessage(HWND hWnd);
	// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	CTextUI *m_lpfilename;
	CTextUI *m_lptitle3;
	CEditUI *m_lppassword;
	int m_nIndex;
};
