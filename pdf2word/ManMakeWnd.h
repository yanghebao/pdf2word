#pragma once

// 人工转换窗口 类
class CManMakeWnd: public WindowImplBase
{
public:
	explicit CManMakeWnd();

protected:
	virtual ~CManMakeWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("Man_Make_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_manmake.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init();
	virtual void    OnFinalMessage(HWND hWnd);
	// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

	//检测文件在服务器是否存在
	BOOL CheckFileInSvr(CString strFile);

public:
	CString m_strFilePath;
	CTabLayoutUI *m_lpStepTab;
	CTextUI *m_lpmoney;
	CTextUI *m_lpprogress;
	CButtonUI *m_lpbtnUpload;
	CButtonUI *m_lpbtnOK;
	CButtonUI *m_lpbtnPayOK;
	CEditUI *m_lpEmailEdt;
	CEditUI *m_lpMobileEdt;
	CEditUI *m_lpMachineCode;

	CString m_strFileMd5;
	int m_nFilePages;
	CString m_strFileName;
	CString m_strSign;
};
