#pragma once

// 公共消息提示类
class CMsgWnd: public WindowImplBase
{
public:
	int m_nType;

	explicit CMsgWnd();

protected:
	virtual ~CMsgWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("Update_Msg_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_message.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init(HWND hParent, CString strMsg, int nTag);
	virtual void    OnFinalMessage(HWND hWnd);
// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);
};
