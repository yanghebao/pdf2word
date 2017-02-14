#pragma once

#include "PDF2WordWrapper.h"


// 主窗口类
class CMyWnd : public WindowImplBase
{
public:
	CListUI *m_lpList;
	CListUI *m_lpList_man;
	CContainerUI *m_lpTopLeft;
	CButtonUI *m_lpDragFile;
	CVerticalLayoutUI *m_lpListBottom;
	COptionUI *m_lpSelAllOpt;
// 	COptionUI *m_lpSelAllOpt1;
	CButtonUI *m_lpbtnDelete;
	COptionUI *m_lpSavePathOpt;
	COptionUI *m_lpSelSavePathOpt;
	CEditUI *m_lpSavePathEdt;
	CButtonUI *m_lpOpenPathBtn;
	CComboUI *m_lpOutDocForma;
	CButtonUI *m_lpVipBtn;
	CButtonUI *m_lpbtnConvert;
	CLabelUI *m_lpFileState;
	CButtonUI *m_lpbtnOpenFile;
	CButtonUI *m_lpbtnOpenFolder;
	CTabLayoutUI *m_lpTabLayout;

	CWebBrowserUI *m_lpWebTj;

	int m_nPageNumber;
	int m_nPageCount;

public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("My_Wnd_Frame");  }
	virtual CDuiString GetSkinFile()                {   return _T("main.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("xx?");  }	

	virtual void InitWindow();

	//获取行所对应vector 中的索引
	int GetRowInVectorIndex(int nRow);

	//判断行是否是选中的
	BOOL ChkRowIsSelect(int nRow);

	//删除列表后刷新
	void RefreshListWhenDel();

	//增加文件后刷新要处理的文件列表
	void RefreshFileList();
	void RefreshFileListWhenAddFiles();

	//刷新人工转换订单列表
	void RefreshManMakeList();

	virtual void Notify(TNotifyUI& msg);

	//打开选择pdf文件对话框
	void OpenAddPdfFileDlg();

	//根据全路径增加文件信息到vec
	void AddPdfFile2Vec(CString strFilePath);

	//拖动文件到对话框
	void DropFileOnDlg(HDROP hDrop);

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);


};

unsigned __stdcall ProgressFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*) pM;

// 	CListContainerElementUI *lpRow = (CListContainerElementUI*)lpWnd->m_lpList->GetItemAt(g_nNowRow);
// 	CProgressUI *lpProgress = (CProgressUI*)lpRow->FindSubControl(_T("progress"));
// 	lpProgress->SetMinValue(1);
// 	lpProgress->SetMinValue(lpWnd->m_nPageCount);
// 	lpProgress->SetValue(lpWnd->m_nPageNumber + 1);

	_endthreadex(0);
	return 0;
}

extern CMyWnd *g_MainWnd;
extern int g_nNowRow;	//当前正在转换的行


class CIPdf2WordEvent : public CPDF2WordWrapper
{
public:
	EasyConverterLib::cnvResponse OnPageStart(long nPageNumber, long nPageCount, BSTR bstrFileName)
	{

		CListContainerElementUI *lpRow = (CListContainerElementUI*)g_MainWnd->m_lpList->GetItemAt(g_nNowRow);
		CProgressUI *lpProgress = (CProgressUI*)lpRow->FindSubControl(_T("progress"));
		lpProgress->SetValue(nPageNumber + 1);

		if (0)
			return EasyConverterLib::CNV_CANCEL;
		else
			return EasyConverterLib::CNV_CONTINUE;
	}

	void OnConversionFinished(long nResult)
	{
		CListContainerElementUI *lpRow = (CListContainerElementUI*)g_MainWnd->m_lpList->GetItemAt(g_nNowRow);
		CProgressUI *lpProgress = (CProgressUI*)lpRow->FindSubControl(_T("progress"));
		lpProgress->SetVisible(FALSE);

		CButtonUI *lpbtnOpenFile = (CButtonUI*)lpRow->FindSubControl(_T("btnOpenFile"));
		CButtonUI *lpbtnOpenFolder = (CButtonUI*)lpRow->FindSubControl(_T("btnOpenFolder"));
		lpbtnOpenFile->SetVisible(TRUE);
		lpbtnOpenFolder->SetVisible(TRUE);

		//启用删除按钮和 combo
		CComboUI *lpCombo = (CComboUI*)lpRow->FindSubControl(_T("comboPageRange"));
		lpCombo->SetEnabled(TRUE);
		CButtonUI *lpDelBtn = (CButtonUI*)lpRow->FindSubControl(_T("btnDeleteFile"));
		lpDelBtn->SetEnabled(TRUE);

		if (nResult)
		{
		}

	}

};

class CBindCallback : public IBindStatusCallback    
{  
public:  
	CBindCallback();  
	virtual ~CBindCallback();  

	//接受显示进度窗口的句柄  
	CListContainerElementUI* m_lpRow;  

	//IBindStatusCallback的方法。除了OnProgress     外的其他方法都返回E_NOTIMPL   

	STDMETHOD(OnStartBinding)  
		( DWORD dwReserved,  
		IBinding __RPC_FAR *pib)  
	{ return E_NOTIMPL; }  

	STDMETHOD(GetPriority)  
		( LONG __RPC_FAR *pnPriority)  
	{ return E_NOTIMPL; }  

	STDMETHOD(OnLowResource)  
		( DWORD reserved)  
	{ return E_NOTIMPL; }  

	//OnProgress在这里  
	STDMETHOD(OnProgress)  
		( ULONG ulProgress,      
		ULONG ulProgressMax,  
		ULONG ulStatusCode,  
		LPCWSTR wszStatusText);  

	STDMETHOD(OnStopBinding)  
		( HRESULT hresult,  
		LPCWSTR szError)  
	{ return E_NOTIMPL; }  

	STDMETHOD(GetBindInfo)  
		( DWORD __RPC_FAR *grfBINDF,  
		BINDINFO __RPC_FAR *pbindinfo)  
	{ return E_NOTIMPL; }  

	STDMETHOD(OnDataAvailable)  
		( DWORD grfBSCF,  
		DWORD dwSize,  
		FORMATETC __RPC_FAR *pformatetc,  
		STGMEDIUM __RPC_FAR *pstgmed)  
	{ return E_NOTIMPL; }  

	STDMETHOD(OnObjectAvailable)  
		( REFIID riid,  
		IUnknown __RPC_FAR *punk)  
	{ return E_NOTIMPL; }  

	// IUnknown方法.IE 不会调用这些方法的  

	STDMETHOD_(ULONG,AddRef)()  
	{ return 0; }  

	STDMETHOD_(ULONG,Release)()  
	{ return 0; }  

	STDMETHOD(QueryInterface)  
		( REFIID riid,  
		void __RPC_FAR *__RPC_FAR *ppvObject)  
	{ return E_NOTIMPL; }  
};  
