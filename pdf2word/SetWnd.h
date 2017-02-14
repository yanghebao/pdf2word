#pragma once

// 设置类
class CSetWnd: public WindowImplBase
{
public:
	explicit CSetWnd();

protected:
	virtual ~CSetWnd();   // 私有化析构函数，这样此对象只能通过new来生成，而不能直接定义变量。就保证了delete this不会出错

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("Update_Msg_wnd");  }
	virtual CDuiString GetSkinFile()                {   return _T("dlg_setting.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	void Notify(TNotifyUI& msg);
	void Init();
	virtual void    OnFinalMessage(HWND hWnd);
	// 	virtual LRESULT HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

	void SetConnectSetting();

	void SaveSettingData();

public:
	COptionUI *m_lpTabOpt1;
	COptionUI *m_lpTabOpt2;
	CTabLayoutUI *m_lpTabMain;

	COptionUI *m_lpCNV_Default;
	COptionUI *m_lpCNV_USE_SAT;
	COptionUI *m_lpCNV_TEXTBOX;
	COptionUI *m_lpCNV_TEXTFLOW;
	COptionUI *m_lpAdjustFontNames;
	COptionUI *m_lpAdjustSpacing;
	COptionUI *m_lpExtractImages;
	COptionUI *m_lpExtractText;
	COptionUI *m_lpShrinkFonts;
	COptionUI *m_lpDetectHeader;
	COptionUI *m_lpRemoveHiddenText;
	COptionUI *m_lpSingleColumn;
	COptionUI *m_lpMutileColumn;
	COptionUI *m_lpPleading;
	COptionUI *m_lpFormat_RTF;
	COptionUI *m_lpFormat_DOC_Native;
	COptionUI *m_lpFormat_DOCX;
	COptionUI *m_lpBookMark_None;
	COptionUI *m_lpBookMark_PAGESTART;
	COptionUI *m_lpBookMark_PDF;
	COptionUI *m_lpOCR_TEXTIMAGE;
	COptionUI *m_lpOCR_TEXT;
	COptionUI *m_lpOCR_IMAGE;
	CEditUI *m_lpConvertTimeout;
	COptionUI *m_lpAutoOpenFolderFinish;
};
