#pragma once

// 用户反馈窗口类
class CFeedBackWnd: public WindowImplBase
{
public:
	explicit CFeedBackWnd();

protected:
	virtual ~CFeedBackWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("FeedBack_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_feedback.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init();
	virtual void    OnFinalMessage(HWND hWnd);
	// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	CRichEditUI *m_edtFeedback;
	CEditUI *m_edtContact;
};
