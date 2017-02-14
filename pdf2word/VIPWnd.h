#pragma once

// 开通VIP窗口类
class CVIPWizardWnd: public WindowImplBase
{
public:
	explicit CVIPWizardWnd();

protected:
	virtual ~CVIPWizardWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("VIP_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_VIPWizard.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init();
	virtual void    OnFinalMessage(HWND hWnd);
	// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	CButtonUI *m_lpCloseBtn;
	CButtonUI *m_lpExitBtn;
	CTabLayoutUI *m_lpTab;
	CButtonUI *m_lpPay;
	CButtonUI *m_lpPayOK;
	COptionUI *m_lpOneYearOpt;
	COptionUI *m_lpMoreYearOpt;
	CTextUI *m_lpMat;

	int m_nPayType; //1年费 2永久

};
