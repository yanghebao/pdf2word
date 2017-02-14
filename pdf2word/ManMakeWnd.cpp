#include "stdafx.h"
#include "ManMakeWnd.h"
#include "MsgWnd.h"
#include "HttpUploadFiles.h"

#import "progid:EasyConverter.PDF2Word.5"

//检验邮箱地址是否合法
BOOL IsValidEmail(TCHAR *lpszEmail)
{
	if(lpszEmail == NULL)
	{
		return FALSE;
	}

	int iAtPos = 0;
	int iLastDotPos = 0;
	int i = 0;
	int iAtTimes = 0;

	while(*(lpszEmail + i) != _T('\0'))
	{
		TCHAR ch = *(lpszEmail + i);
		if(!_istprint(ch) || _istspace(ch))  //空格和控制字符是非法的，限制得还比较宽松
		{
			iAtTimes = 0;
			break;
		}
		if(ch == _T('@'))
		{
			iAtPos = i;
			iAtTimes++;
		}
		else if(ch == _T('.'))
		{
			iLastDotPos = i;
		}
		i++;
	}

	if(i > 64 || iAtPos < 1 || (iLastDotPos - 2) < iAtPos ||
		(i - iLastDotPos) < 3 || (i - iLastDotPos) > 5 || iAtTimes > 1 || iAtTimes == 0) //对@以及域名依靠位置来判断，限制长度为64
	{
		return FALSE;
	}
	return TRUE;
}

CManMakeWnd::CManMakeWnd() 
{
	m_strFileMd5 = _T("");
	m_nFilePages = 0;
	m_strFileName = _T("");
	m_strSign = _T("");
}

CManMakeWnd::~CManMakeWnd()
{
}

void CManMakeWnd::Init()
{
	Create(g_hMainWnd, _T(""), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	CenterWindow();

	m_lpStepTab = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tabStep")));
	m_lpmoney = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("money")));
	m_lpmoney->SetText(_T("请您先上传要人工转换的PDF文档"));
	m_lpprogress = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("progress")));
	m_lpbtnUpload = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnUpload")));
	m_lpbtnOK = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnOK")));
	m_lpbtnPayOK = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnPayOK")));

	m_lpEmailEdt = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("contact")));
	m_lpMobileEdt = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("mobile")));
	m_lpMachineCode = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("MachineCode")));

	TCHAR szBuf[1024] = _T("");
	GetPrivateProfileString(_T("INFO"), _T("UserEmail"), _T(""), szBuf, 1024, g_strMainIni.GetBuffer(0));
	m_lpEmailEdt->SetText(szBuf);
	GetPrivateProfileString(_T("INFO"), _T("UserPhone"), _T(""), szBuf, 1024, g_strMainIni.GetBuffer(0));
	m_lpMobileEdt->SetText(szBuf);

	m_lpMachineCode->SetText(g_strMachineCode.GetBuffer(0));
}

void CManMakeWnd::OnFinalMessage( HWND /*hWnd*/ )
{
	delete this;
}

// LRESULT CManMakeWnd::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
// {
// 	LRESULT lRes = 0;
// 	BOOL bHandled = TRUE;
// 
// 	switch( uMsg )
// 	{
// 	case WM_KILLFOCUS:    
// // 		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); 
// 		break; 
// 
// 	default:
// 		bHandled = FALSE;
// 	}
// 
// 	if(bHandled || m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes)) 
// 	{
// 		return lRes;
// 	}
// 
// 	return __super::HandleMessage(uMsg, wParam, lParam);
// }

//获取PDF文件页数 返回-1表示有密码
int GetPdfFilePages(CString strFilePath)
{
	int nPages = 0;
	//获取文件
	::CoInitialize(NULL);
	EasyConverterLib::IPDF2WordPtr pConvObj = NULL;
	try
	{
		pConvObj.CreateInstance("EasyConverter.PDF2Word.5"); 
		pConvObj->LicenseKey = GetBCLKey().GetBuffer(0);	//正式版OEM license key 20161221 14个月

		nPages = pConvObj->GetNumberOfPages(strFilePath.GetBuffer(0));	
	}
	catch(_com_error &e) //表示有密码或文件损坏
	{
		//此处全部归于有密码
		{
			nPages = -1;
		}
	}

	pConvObj = NULL; 
	::CoUninitialize();

	return nPages;
}

//计算文件收费
int GetPDFPrice(CString strFilePath)
{
	int nPages = 0;
	int nPrice = 0;

	nPages = GetPdfFilePages(strFilePath);

	if (nPages < 0)
	{
		return nPrice;
	}

	if (nPages > 0 && nPages <= 100)
	{
		nPrice = 10;
	}
	else if (nPages > 100 && nPages <= 200)
	{
		nPrice = 20;
	}
	else if (nPages > 200 && nPages <= 300)
	{
		nPrice = 30;
	}
	else
	{
		nPrice = 50;
	}

	return nPrice;
}

//文件上传成功后填写数据库
BOOL UpdataWhenUpFileSuccess(CString strFileMd5, int nPages, CString strFileDownLoadUrl)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	CTime tUTC(st);
	CString strTime;
	strTime.Format(_T("%d"), tUTC.GetTime());

	CString strSign;
	strSign.Format(_T("%s|%d|%s|quxiupdf2word"), strFileMd5, nPages, strTime);
	CString strSignMd5 = MakeCStringMD5W(strSign);

	CString strPage;
	strPage.Format(_T("%d"), nPages);
	CString strUrl = _T("http://www.pdf00.com/files/add?filemd5=[filemd5]&page=[page]&url=[url]&time=[time]&sign=[sign]");
	strUrl.Replace(_T("[filemd5]"), strFileMd5);
	strUrl.Replace(_T("[page]"), strPage);
	strUrl.Replace(_T("[url]"), strFileDownLoadUrl);
	strUrl.Replace(_T("[time]"), strTime);
	strUrl.Replace(_T("[sign]"), strSignMd5);

// 	WritePrivateProfileString(_T("Info"), _T("Updata"), strUrl.GetBuffer(0), _T("C:\\1.ini")); //xx? 测试
	CString strUrlInfo = GetHtmlStr(strUrl);
	if (strUrlInfo.IsEmpty())
	{
		return FALSE;
	}

	//获取返回值
	CString strCode = GetStrFormJson(strUrlInfo, _T("code"), _T(""), 1);
	if (_T("200") == strCode)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

//上传文件线程
unsigned __stdcall UploadFile2SvrFunc(LPVOID pM)
{
	CManMakeWnd *lpWnd = (CManMakeWnd*)pM;

	CString strFile = lpWnd->m_strFilePath;
	CString strFileMd5 = MakeFileMD5W(strFile);

	lpWnd->m_lpbtnOK->SetEnabled(FALSE);
	lpWnd->m_lpbtnUpload->SetEnabled(FALSE);

	//获取文件名
	CString strFileName;
	strFileName = strFile.Mid(strFile.ReverseFind(_T('\\')) + 1);
	lpWnd->m_lpbtnUpload->SetText(strFileName.GetBuffer(0));
	lpWnd->m_strFileName = strFileName;

	SYSTEMTIME st;
	GetSystemTime(&st);
	CTime tUTC(st);
	CString strTime;
	strTime.Format(_T("%d"), tUTC.GetTime());

	CString strMd5;
	strMd5.Format(_T("%s|%s|quxiupdf2word"), strFileMd5, strTime);
	CString md5 = MakeCStringMD5W(strMd5);

	CString strUrl = _T("http://upload.pdf00.com/php/controller.php?action=uploadfile&encode=utf-8&filemd5=[filemd5]&time=[time]&sign=[md5]");
	strUrl.Replace(_T("[filemd5]"), strFileMd5);
	strUrl.Replace(_T("[time]"), strTime);
	strUrl.Replace(_T("[md5]"), md5);

	lpWnd->m_lpprogress->SetText(_T("正在上传PDF (0%%)"));
	CHttpUploadFiles httpUpFile;
	httpUpFile.SetUserData(strFile, (void*)lpWnd);

	VecStParam vecParam;

	CString strKey = _T("upfile");
	BOOL bUpFile = httpUpFile.TransDataToServer(strUrl.GetBuffer(0),
							vecParam, strFile.GetBuffer(0), strKey.GetBuffer(0));

	CString strReceive;
	BYTE szBuf[1024] = {0};
	DWORD dwReceive = 1024;
	httpUpFile.ReceiveData(szBuf, dwReceive);
	char *lp = UTF_8ToGB2312((char*)szBuf, dwReceive);
	CStringA strReceiveA = lp;
	delete []lp;
	CString strReceiveW = CStrA2CStrW(strReceiveA);

	//分解数据
	CString strState = GetStrFormJson(strReceiveW, _T("state"), _T(""), 0);
	CString strDownUrl = GetStrFormJson(strReceiveW, _T("url"), _T(""), 0);

	if (bUpFile && (_T("SUCCESS") == strState))
	{
		CString strMsg;
		strMsg.Format(_T("上传完成 (100%%)"));
		lpWnd->m_lpprogress->SetText(strMsg.GetBuffer(0));
		lpWnd->m_lpbtnOK->SetEnabled(TRUE);

		//设置文件的金额
		int nPrice = GetPDFPrice(strFile);
		if (nPrice <= 0)
		{
			lpWnd->m_lpbtnOK->SetEnabled(TRUE);
			lpWnd->m_lpbtnUpload->SetEnabled(TRUE);
			lpWnd->m_lpbtnUpload->SetText(_T("上传要转换的文件"));
			lpWnd->m_lpprogress->SetText(_T("批量或超过20M的PDF请先与客服联系"));
		}
		CString strPrice;
		strPrice.Format(_T("%d元"), nPrice);
		lpWnd->m_lpmoney->SetText(strPrice.GetBuffer(0));

		//上传成功后设置数据库
		int nPages = GetPdfFilePages(strFile.GetBuffer(0));

		if (!UpdataWhenUpFileSuccess(strFileMd5, nPages, strDownUrl))
		{
			//写入数据库失败
			CMsgWnd *lpMsg = new CMsgWnd;
			lpMsg->Init(lpWnd->GetHWND(), _T("文件上传失败，请再次上传或与管理员联系"), -1);
			lpMsg->ShowModal();

			lpWnd->m_lpbtnOK->SetEnabled(TRUE);
			lpWnd->m_lpbtnUpload->SetEnabled(TRUE);
			lpWnd->m_lpbtnUpload->SetText(_T("上传要转换的文件"));
			lpWnd->m_lpprogress->SetText(_T("批量或超过20M的PDF请先与客服联系"));
		}
		
	}
	else
	{
		lpWnd->m_lpbtnOK->SetEnabled(TRUE);
		lpWnd->m_lpbtnUpload->SetEnabled(TRUE);
		lpWnd->m_lpbtnUpload->SetText(_T("上传要转换的文件"));
		lpWnd->m_lpprogress->SetText(_T("批量或超过20M的PDF请先与客服联系"));
	}

	lpWnd->m_strFileMd5 = strFileMd5;

	_endthreadex(0);
	return 0;
}

//检测文件在服务器是否存在
BOOL CManMakeWnd::CheckFileInSvr(CString strFile)
{
	CString strfileMd5 = MakeFileMD5W(strFile);

	SYSTEMTIME st;
	GetSystemTime(&st);
	CTime tUTC(st);
	CString strTime;
	strTime.Format(_T("%d"), tUTC.GetTime());

	CString strSign;
	strSign.Format(_T("%s|%s|quxiupdf2word"), strfileMd5, strTime);
	CString strSignMd5 = MakeCStringMD5W(strSign);

	CString strUrl = _T("http://www.pdf00.com/files/check_fileext?filemd5=[filemd5]&time=[time]&sign=[sign]");
	strUrl.Replace(_T("[filemd5]"), strfileMd5);
	strUrl.Replace(_T("[time]"), strTime);
	strUrl.Replace(_T("[sign]"), strSignMd5);

	CString strUrlInfo = GetHtmlStr(strUrl);
	if (strUrlInfo.IsEmpty())
	{
		return FALSE;
	}

	//获取返回值
	CString strCode = GetStrFormJson(strUrlInfo, _T("code"), _T(""), 1);
	if (_T("500") == strCode)
	{
		m_strFileMd5 = strfileMd5;
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

void CManMakeWnd::Notify(TNotifyUI& msg)
{
	if(msg.sType == _T("click")) 
	{
		CString strName = msg.pSender->GetName();
		if (_T("btnClose") == strName)
		{
			Close();
		}
		else if (_T("btnQQ") == strName)	//在线客服咨询
		{
			SayHelloOnLineByQQ();
		}
		else if (_T("btnNext") == strName)	// 下一步
		{
			//切换
			m_lpStepTab->SelectItem(1);

			//隐藏
			msg.pSender->SetVisible(FALSE);

			//显示支付
			static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnOK")))->SetVisible(TRUE);
		}
		else if (_T("btnOK") == strName)	// 支付
		{
			//先上传文件
			if (m_strFileMd5.IsEmpty())
			{
				ShowMsgTipWnd(this->GetHWND(), _T("请您先上传要人工转换的PDF文档"), -1);
// 				CMsgWnd *lpMsg = new CMsgWnd;
// 				lpMsg->Init(_T("请您先上传要人工转换的PDF文档"), -1);
// 				lpMsg->ShowModal();
				return ;
			}

			//检查输入邮箱格式合法性
			CString strEmail = m_lpEmailEdt->GetText();
			CString strPhone = m_lpMobileEdt->GetText();
			if (strEmail.IsEmpty() || strPhone.IsEmpty())
			{
				ShowMsgTipWnd(this->GetHWND(), _T("请您填写电子邮箱和手机"), -1);
// 				CMsgWnd *lpMsg = new CMsgWnd;
// 				lpMsg->Init(_T("请您填写电子邮箱和手机"), -1);
// 				lpMsg->ShowModal();
				return ;
			}

			//检查邮箱合法性
			if (!IsValidEmail(strEmail.GetBuffer(0)))
			{
				ShowMsgTipWnd(this->GetHWND(), _T("很抱歉，您的邮箱地址是无效的，请重新输入"), -1);
// 				CMsgWnd *lpMsg = new CMsgWnd;
// 				lpMsg->Init(_T("很抱歉，您的邮箱地址是无效的，请重新输入"), -1);
// 				lpMsg->ShowModal();
				return ;
			}
			
			//手机
			if (strPhone.GetLength() < 11)
			{
				ShowMsgTipWnd(this->GetHWND(), _T("很抱歉，您的手机号码是无效的，请重新输入"), -1);
// 				CMsgWnd *lpMsg = new CMsgWnd;
// 				lpMsg->Init(_T("很抱歉，您的手机号码是无效的，请重新输入"), -1);
// 				lpMsg->ShowModal();
				return ;
			}

			CString strPayUrl = _T("http://www.pdf00.com/pay/?viptype=3&mn=[mn]&filemd5=[FileMd5]&email=[email]&phone=[phone]&time=[time]&filename=[filename]&sign=[sign]");

			CString strFileName = UrlEncode(m_strFileName);

			SYSTEMTIME st;
			GetSystemTime(&st);
			CTime tUTC(st);
			CString strTime;
			strTime.Format(_T("%d"), tUTC.GetTime());

			CString strSign;
			strSign.Format(_T("%s|%s|quxiupdf2word"), m_strFileMd5, strTime);
			CString strSignMd5 = MakeCStringMD5W(strSign);
			
			strPayUrl.Replace(_T("[mn]"), g_strMachineCode);
			strPayUrl.Replace(_T("[FileMd5]"), m_strFileMd5);
			strPayUrl.Replace(_T("[email]"), strEmail);
			strPayUrl.Replace(_T("[phone]"), strPhone);
			strPayUrl.Replace(_T("[time]"), strTime);
			strPayUrl.Replace(_T("[filename]"), strFileName);
			strPayUrl.Replace(_T("[sign]"), strSignMd5);

// 			WritePrivateProfileString(_T("Info"), m_strFileName.GetBuffer(0), strPayUrl.GetBuffer(0), _T("C:\\1.ini"));
			ShellExecute(NULL, _T("open"), strPayUrl.GetBuffer(0), NULL, NULL, SW_SHOWNORMAL);

			//记录手机和邮箱
			WritePrivateProfileString(_T("Info"), _T("UserEmail"), strEmail.GetBuffer(0), g_strMainIni.GetBuffer(0));
			WritePrivateProfileString(_T("Info"), _T("UserPhone"), strPhone.GetBuffer(0), g_strMainIni.GetBuffer(0));

			m_lpbtnOK->SetText(_T("重新支付"));
			m_lpbtnPayOK->SetVisible(TRUE);
		}
		else if (_T("btnPayOK") == strName)	//我已支付成功
		{
			//刷新订单列表
			g_MainWnd->RefreshManMakeList();

			Close();
		}
		else if (_T("btnUpload") == strName)	//上传文件
		{
			// 打开文件打开对话框，如果选中文件，则NewGame  
			OPENFILENAME ofn;      // 公共对话框结构。  
			TCHAR szFile[MAX_PATH]; // 保存获取文件名称的缓冲区。            

			// 初始化选择文件对话框。  
			ZeroMemory(&ofn, sizeof(ofn));  
			ofn.lStructSize = sizeof(ofn);  
			ofn.hwndOwner = this->GetHWND();  
			ofn.lpstrFile = szFile;  
			ofn.lpstrTitle = _T("请选择要转换的PDF文件：");
			//  
			//  
			ofn.lpstrFile[0] = _T('\0');  
			ofn.nMaxFile = sizeof(szFile);  
			ofn.lpstrFilter = _T("PDF文件(*.pdf)\0*.pdf");  
			ofn.nFilterIndex = 1;  
			ofn.lpstrFileTitle = NULL;  
			ofn.nMaxFileTitle = 0;  
// 			ofn.lpstrInitialDir = _T("Record//");  
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;  

			// 显示打开选择文件对话框。  
			if (GetOpenFileName(&ofn))  
			{  
				CString strFile = szFile;
				m_strFilePath = strFile;

				//判断文件有没有超过20M
				CString strFileSize = M_GetFileSize(strFile.GetBuffer(0));
				float fFileSize = _tstof(strFileSize.GetBuffer(0));
				if (fFileSize >= 20.0)
				{
					//提示超过20M
					ShowMsgTipWnd(this->GetHWND(), _T("您好，超过20M的PDF文件无法直接上传，请联系在线客服处理"), -1);
// 					CMsgWnd *lpMsg = new CMsgWnd;
// 					lpMsg->Init(this->GetHWND(), _T("您好，超过20M的PDF文件无法直接上传，请联系在线客服处理"), -1);
// 					lpMsg->ShowModal();
					return ;
				}

				//判断文件有没有密码
				int nPages = GetPdfFilePages(strFile.GetBuffer(0));
				if (nPages < 0)
				{
					//有密码
					ShowMsgTipWnd(this->GetHWND(), _T("很抱歉，该PDF是加密保护文件，无法直接人工转换，请先咨询客服或先提交人工破解服务"), -1);
// 					CMsgWnd *lpMsg = new CMsgWnd;
// 					lpMsg->Init(this->GetHWND(), _T("很抱歉，该PDF是加密保护文件，无法直接人工转换，请先咨询客服或先提交人工破解服务"), -1);
// 					lpMsg->ShowModal();
					return ;
				}
				m_nFilePages = nPages;

				//检测文件是否存在服务器
				if (!CheckFileInSvr(strFile))
				{
					//上传文件
					_beginthreadex(NULL, 0, UploadFile2SvrFunc, (LPVOID)this, 0, NULL);
				}
				else
				{
					//服务器存在文件
					CString strFileName;
					strFileName = strFile.Mid(strFile.ReverseFind(_T('\\')) + 1);
					m_lpbtnUpload->SetText(strFileName.GetBuffer(0));
					m_lpbtnUpload->SetEnabled(FALSE);
					m_strFileName = strFileName;

					CString strMsg;
					strMsg.Format(_T("上传完成 (100%%)"));
					m_lpprogress->SetText(strMsg.GetBuffer(0));
					m_lpbtnOK->SetEnabled(TRUE);

					//设置文件的金额
					int nPrice = GetPDFPrice(strFile);
					if (nPrice <= 0)
					{
						m_lpbtnOK->SetEnabled(TRUE);
						m_lpbtnUpload->SetEnabled(TRUE);
						m_lpbtnUpload->SetText(_T("上传要转换的文件"));
						m_lpprogress->SetText(_T("批量或超过20M的PDF请先与客服联系"));
					}
					CString strPrice;
					strPrice.Format(_T("%d元"), nPrice);
					m_lpmoney->SetText(strPrice.GetBuffer(0));

				}

			} 

		}
		
	}

	__super::Notify(msg);
};
