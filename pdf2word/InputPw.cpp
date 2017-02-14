#include "stdafx.h"
#include "InputPW.h"


CInputPWWnd::CInputPWWnd() 
{
}

CInputPWWnd::~CInputPWWnd()
{
}

// nIndex 表示在g_vec 中的索引
void CInputPWWnd::Init(int nIndex)
{
	Create(g_hMainWnd, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	CenterWindow();

	m_nIndex = nIndex;

	m_lpfilename = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("filename")));
	m_lpfilename->SetText(g_vecPdfInfo[nIndex].strFileName.GetBuffer(0));
	m_lppassword = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("password")));
	m_lptitle3 = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("title3")));

//	CLabelUI *lpProduct = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("ProductName")));
}

void CInputPWWnd::OnFinalMessage( HWND /*hWnd*/ )
{
	delete this;
}

void CInputPWWnd::Notify(TNotifyUI& msg)
{
	if(msg.sType == _T("click")) 
	{
		CString strName = msg.pSender->GetName();
		if (_T("btnClose") == strName)		//关闭对话框
		{
			Close(IDCANCEL);
		}
		else if (_T("btnOK") == strName)
		{
			//获取用户输入的密码
			CString strPassWord = m_lppassword->GetText();

			int nPages = 0;
			::CoInitialize(NULL);
			EasyConverterLib::IPDF2WordPtr pConvObj = NULL;
			try
			{
				pConvObj.CreateInstance("EasyConverter.PDF2Word.5"); 
				pConvObj->LicenseKey = GetBCLKey().GetBuffer(0);	//正式版OEM license key 20161221 14个月


				nPages = pConvObj->GetNumberOfPages(g_vecPdfInfo[m_nIndex].strFilePath.GetBuffer(0), strPassWord.GetBuffer(0));	//密码

				g_vecPdfInfo[m_nIndex].strPassWord = strPassWord;
				g_vecPdfInfo[m_nIndex].nFilePages = nPages;

				Close();
			}
			catch(_com_error &e) //表示有密码有误
			{
				//提示密码不正确
				{
					m_lptitle3->SetVisible(TRUE);
					m_lptitle3->SetText(_T("密码输入不正确,请重新输入"));
				}
			}

			pConvObj = NULL; 
			::CoUninitialize();
		}
	}

	__super::Notify(msg);
};
