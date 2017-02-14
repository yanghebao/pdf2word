#include "stdafx.h"
#include "MainWnd.h"
#include "MenuWnd.h"
#include "MsgWnd.h"
#include "SetWnd.h"
#include "ManMakeWnd.h"
#include "VIPWnd.h"
#include "PageRangeWnd.h"
#include "InputPW.h"
#include "HttpTransByGet.h"
#include <UrlMon.h>


vector<Pdf_Info> g_vecPdfInfo;	//pdf文件list
int g_nVecCursor = 0; //每次增加完文件刷新list后 记录 g_vecPdfInfo 位置

vector<OrderInfo> g_vecManMakeInfo;	//人工转换订单列表

CString g_strSavePath;	// doc文件保存路径

Set_Connect g_tagConnectSetting;	//默认属性设置

int g_nNowRow = 0;	//当前正在转换的行

CIPdf2WordEvent g_IPdf2WordEvent;	// 转换事件对象

BOOL g_bIsSameAsPdf = FALSE;	//word文件和pdf文件保存在相同目录

VIP_Info g_tagVipInfo;	//会员信息

//初始化默认转换属性
void InitConnectSetting()
{
	g_tagConnectSetting.nConversionMethod = 0; //版面设置

	g_tagConnectSetting.bAdjustFontNames = TRUE;	//校正字体名称

	g_tagConnectSetting.bAdjustSpacing = TRUE;		//校正间距

	g_tagConnectSetting.bExtractImages = TRUE;		//包含图片

	g_tagConnectSetting.bExtractText = TRUE;		//包含文字

	g_tagConnectSetting.bShrinkFonts = TRUE;		//缩小字体

	g_tagConnectSetting.bDetectHeader = TRUE;		//检查头部

	g_tagConnectSetting.bSaveHiddenText = TRUE;	//保留隐藏文本

	g_tagConnectSetting.nDocumentType = 0;		//文档类型： 单列 多列 自适应

	g_tagConnectSetting.nPutoutExt = 1;			//输出格式: RTF文件 DOC文件 DOCX文件

	g_tagConnectSetting.nBkmkOpt = 1;			//输出书签：不转换书签 每页开头保留书签 和PDF一样

	g_tagConnectSetting.nOcrOpt = 0;			//OCR文档：保留图片和文字 仅保留文字 仅保留图片

	g_tagConnectSetting.nTimeout = 0;			//等待转换最长时间 分钟

	g_tagConnectSetting.bOpenPath = FALSE;			//转换成功后自动打开Word所在的目录
}

//设置转换属性
void SetConnectSetting(int nConversionMethod, BOOL bAdjustFontNames,
					   BOOL bAdjustSpacing, BOOL bExtractImages, BOOL bExtractText, BOOL bShrinkFonts,
					   BOOL bDetectHeader, BOOL bSaveHiddenText, int nDocumentType, int nPutoutExt,
					   int nBkmkOpt, int nOcrOpt, int nTimeout, int bOpenPath)
{
	g_tagConnectSetting.nConversionMethod = nConversionMethod; //版面设置

	g_tagConnectSetting.bAdjustFontNames = bAdjustFontNames;	//校正字体名称

	g_tagConnectSetting.bAdjustSpacing = bAdjustSpacing;		//校正间距

	g_tagConnectSetting.bExtractImages = bExtractImages;		//包含图片

	g_tagConnectSetting.bExtractText = bExtractText;		//包含文字

	g_tagConnectSetting.bShrinkFonts = bShrinkFonts;		//缩小字体

	g_tagConnectSetting.bDetectHeader = bDetectHeader;		//检查头部

	g_tagConnectSetting.bSaveHiddenText = bSaveHiddenText;	//保留隐藏文本

	g_tagConnectSetting.nDocumentType = nDocumentType;		//文档类型： 单列 多列 自适应

	g_tagConnectSetting.nPutoutExt = nPutoutExt;			//输出格式: RTF文件 DOC文件 DOCX文件

	g_tagConnectSetting.nBkmkOpt = nBkmkOpt;			//输出书签：不转换书签 每页开头保留书签 和PDF一样

	g_tagConnectSetting.nOcrOpt = nOcrOpt;			//OCR文档：保留图片和文字 仅保留文字 仅保留图片

	g_tagConnectSetting.nTimeout = nTimeout;			//等待转换最长时间 分钟

	g_tagConnectSetting.bOpenPath = bOpenPath;			//转换成功后自动打开Word所在的目录
}

//只需实现OnProgress方法，类的实现：  
CBindCallback::CBindCallback()  
{  

}  

CBindCallback::~CBindCallback()  
{  

}  

//////仅实现OnProgress成员即可  

LRESULT CBindCallback::OnProgress(ULONG ulProgress,  
								  ULONG ulProgressMax,  
								  ULONG ulSatusCode,  
								  LPCWSTR szStatusText)  
{  
	CProgressUI* m_prg = (CProgressUI*)m_lpRow->FindSubControl(_T("progress_down"));  
	m_prg->SetMinValue(0);
	m_prg->SetMaxValue(ulProgressMax);
	m_prg->SetValue(ulProgress);  

	return S_OK;  
}

//获取文件大小 (Mb)
CString M_GetFileSize(CString strFilePath)
{
	// 这种方法获取文件大小会照成内存泄漏而崩溃 原因未知 debug下 调试运行没有问题 直接运行就崩溃 release没有问题 蛋疼
	// 	struct _stat info;
	// 	_tstat(strFilePath.GetBuffer(0), &info);
	// 	_off_t filesize = info.st_size;

	WIN32_FIND_DATA fileInfo; 
	HANDLE hFind; 
	DWORD filesize; 
	hFind = FindFirstFile(strFilePath.GetBuffer(0) ,&fileInfo); 
	if(hFind != INVALID_HANDLE_VALUE) 
		filesize = fileInfo.nFileSizeLow; 

	float fSize = ((float)(filesize))/1024/1024;

	TCHAR szBuf[16] = _T("");
	_stprintf(szBuf, _T("%.2fMB"), fSize);

	CString strFileSize = szBuf;

	return strFileSize;
}

//获取g_nVecCursor 中元素有效的个数
int GetVecTrueNum()
{
	int nCount = 0;

	for (int i = 0; i < g_vecPdfInfo.size(); i++)
	{
		if (!g_vecPdfInfo[i].bIsDel)
		{
			nCount++;
		}
	}

	return nCount;
}

//下载转换好的word文件
unsigned __stdcall DownLoadWordFileFunc(LPVOID pM)
{
	CListContainerElementUI *lpRow = (CListContainerElementUI*) (((CButtonUI*)pM)->GetParent()->GetParent());
	int nIndex = lpRow->GetTag();

	CBindCallback cbc; 
	cbc.m_lpRow = lpRow;

	CButtonUI *lpBtnDownLoad = (CButtonUI*)pM;
	lpBtnDownLoad->SetVisible(FALSE);

	CString strOutputPath = g_strSavePath + g_vecManMakeInfo[nIndex].strFileName + _T(".doc");

	lpRow->FindSubControl(_T("progress_down"))->SetVisible(TRUE);	//显示进度条

	if (S_OK == URLDownloadToFile(NULL, g_vecManMakeInfo[nIndex].strDownLoadUrl.GetBuffer(0), strOutputPath.GetBuffer(0), 0, &cbc))
	{
		lpRow->FindSubControl(_T("btnOpenFile_down"))->SetVisible(TRUE);	//显示打开文件
		lpRow->FindSubControl(_T("btnOpenFolder_down"))->SetVisible(TRUE);	//显示打开目录
		g_vecManMakeInfo[nIndex].bIsDownLoadOK = TRUE;
		g_vecManMakeInfo[nIndex].strWordFile = strOutputPath;
	}
	else
	{
		lpBtnDownLoad->SetVisible(TRUE);
	}
	lpRow->FindSubControl(_T("progress_down"))->SetVisible(FALSE);	//隐藏进度条

	_endthreadex(0);
	return 0;
}

//检查是否是VIP用户
unsigned __stdcall CheckVipUserFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*) pM;

	while (1)
	{
		BOOL bRet = ThisUserIsVIP(g_strMachineCode);

		if (bRet)
		{
			if (_T("2") == g_tagVipInfo.strType)
			{
				lpWnd->m_lpVipBtn->SetText(_T("永久VIP"));
			}
			else
			{
				if (g_tagVipInfo.strNowDate > g_tagVipInfo.strEndDate)
				{
					lpWnd->m_lpVipBtn->SetText(_T("过期VIP"));
				}
				else
				{
					lpWnd->m_lpVipBtn->SetText(_T("年费VIP"));
				}
			}
			break;
		}
		Sleep(20 * 1000);
	}
	_endthreadex(0);
	return 0;
}

//检测BCL是否正确安装
unsigned __stdcall CheckBCLIsOkFunc(LPVOID pM)
{
	EasyConverterLib::IPDF2WordPtr pConvObj = NULL;
	::CoInitialize(NULL);

	//没有正确安装
	if (pConvObj.CreateInstance("EasyConverter.PDF2Word.5") != S_OK)
	{
		CString strBCL32 = g_strPath + _T("UninsFiles\\unins000.exe");
		ShellExecute(NULL, _T("open"), strBCL32.GetBuffer(0), _T("/repair"), NULL, SW_HIDE);
	}

	pConvObj = NULL; 
	::CoUninitialize();

	_endthreadex(0);
	return 0;
}

//添加PDF文件线程函数
unsigned __stdcall WhenAddFilesFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*) pM;

	lpWnd->RefreshFileList();

	_endthreadex(0);
	return 0;
}

//开始转换线程
unsigned __stdcall BeginConnectFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*)pM;
	CString strDocSavePath;

	lpWnd->m_lpbtnConvert->SetEnabled(FALSE);
	lpWnd->m_lpSelAllOpt->SetEnabled(FALSE);
// 	lpWnd->m_lpSelAllOpt1->SetEnabled(FALSE);
	lpWnd->m_lpbtnDelete->SetEnabled(FALSE);

	//先获取输出目录
	if (lpWnd->m_lpSavePathOpt->IsSelected())
	{
		g_bIsSameAsPdf = TRUE;
	}
	else
	{
		strDocSavePath = (lpWnd->m_lpSavePathEdt->GetText()).GetData();
		if (strDocSavePath.IsEmpty())
		{
			lpWnd->m_lpbtnConvert->SetEnabled(TRUE);
			lpWnd->m_lpSelAllOpt->SetEnabled(TRUE);
			lpWnd->m_lpbtnDelete->SetEnabled(TRUE);
			return 0;
		}
	}

	//判断有没有开通VIP
	UINT nRet = -1;

	//获取表格行
	int nRows = lpWnd->m_lpList->GetCount();

	HRESULT Hr = ::CoInitialize(NULL);
	if( FAILED(Hr) ) return 0;

	BOOL bInitSdkRet = InitConvertSDK();

	for (int i = 0; i < nRows; i++)
	{
		CListContainerElementUI *lpRow = (CListContainerElementUI*)lpWnd->m_lpList->GetItemAt(i);
		if (!g_bIsVip)		
		{
			//没有开通则提示只转5页
			CVIPWizardWnd *lpMsgWnd = new CVIPWizardWnd;
			lpMsgWnd->Init();
			nRet = lpMsgWnd->ShowModal();

			CLabelUI *lpFileState = (CLabelUI*)lpRow->FindSubControl(_T("statusText"));
			if (nRet == IDCANCEL)
			{
				lpFileState->SetVisible(TRUE);
				continue ;
			}
			else
			{
				lpFileState->SetVisible(FALSE);
			}
		}

		//判断是否是选中行
		if (lpWnd->ChkRowIsSelect(i))
		{
			//获取行对应的vector中索引
			int nIndex = lpWnd->GetRowInVectorIndex(i);

			//显示进度条
			CProgressUI *lpProgress = (CProgressUI*)lpRow->FindSubControl(_T("progress"));
			lpProgress->SetVisible(TRUE);

			//禁用删除按钮和 combo
			CComboUI *lpCombo = (CComboUI*)lpRow->FindSubControl(_T("comboPageRange"));
			lpCombo->SetEnabled(FALSE);
			CButtonUI *lpDelBtn = (CButtonUI*)lpRow->FindSubControl(_T("btnDeleteFile"));
			lpDelBtn->SetEnabled(FALSE);

			//隐藏 打开文件和打开目录按钮
			CButtonUI *lpOpenFileBtn = (CButtonUI*)lpRow->FindSubControl(_T("btnOpenFile"));
			lpOpenFileBtn->SetVisible(FALSE);

			CButtonUI *lpOpenPathBtn = (CButtonUI*)lpRow->FindSubControl(_T("btnOpenFolder"));
			lpOpenPathBtn->SetVisible(FALSE);

			int nStart;
			int nEnd;
			if (!g_vecPdfInfo[nIndex].strPageRange.IsEmpty())
			{
				int nPos = g_vecPdfInfo[nIndex].strPageRange.Find(_T("-"));
				CString strStart = g_vecPdfInfo[nIndex].strPageRange.Mid(0, nPos);
				CString strEnd = g_vecPdfInfo[nIndex].strPageRange.Mid(nPos + 1);

				nStart = _ttoi(strStart.GetBuffer(0));
				nEnd = _ttoi(strEnd.GetBuffer(0));
			}
			else
			{
				nStart = 1;
				nEnd = g_vecPdfInfo[nIndex].nFilePages;
			}
			lpProgress->SetMinValue(0);
			lpProgress->SetMaxValue(nEnd - nStart);

			// 开始转换
			g_nNowRow = i;

			BeginConvert(nIndex);

			//转换后去掉勾选
			COptionUI *lpCheck = (COptionUI*)lpRow->FindSubControl(_T("checkFile"));
			lpCheck->Selected(FALSE);
		}
	}

	g_IPdf2WordEvent.SetPDF2Word(NULL);

	lpWnd->m_lpbtnConvert->SetEnabled(TRUE);
	lpWnd->m_lpSelAllOpt->SetEnabled(TRUE);
// 	lpWnd->m_lpSelAllOpt1->SetEnabled(TRUE);
	lpWnd->m_lpbtnDelete->SetEnabled(TRUE);

	::CoUninitialize();		//

	_endthreadex(0);
	return 0;
}

//弹出设置转换属性窗口
unsigned __stdcall PopSetWndFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*)pM;

	CSetWnd *lpSetWnd = new CSetWnd;
	lpSetWnd->Init();
	lpSetWnd->ShowModal();

	//
	lpWnd->m_lpOutDocForma->SelectItem(g_tagConnectSetting.nPutoutExt);

	_endthreadex(0);
	return 0;
}


//NotVipWantToFunc
unsigned __stdcall NotVipWantToFunc(LPVOID pM)
{
	CMyWnd *lpWnd = (CMyWnd*)pM;

	CMsgWnd *lpMsg = new CMsgWnd;
	lpMsg->Init(lpWnd->GetHWND(), _T("抱歉，只有VIP用户才能设置转换的页数范围"), 0);
	lpMsg->ShowModal();

	_endthreadex(0);
	return 0;
}

//弹出设置转换页面范围
unsigned __stdcall PopPageRangeWndFunc(LPVOID pM)
{
	CComboUI *lpCbo = (CComboUI*) pM;

	//获取pdf文件页数
	int nIndex = lpCbo->GetParent()->GetParent()->GetTag();
	int nPages = g_vecPdfInfo[nIndex].nFilePages;

	CPageRangeWnd *lpMsg = new CPageRangeWnd;
	lpMsg->Init(nPages);
	if (IDOK == lpMsg->ShowModal())
	{
		int nCount = lpCbo->GetCount();
		g_vecPdfInfo[nIndex].strPageRange = g_strPageRange;
		if (nCount > 2)
		{
			CString strText;
			strText.Format(_T("{f 微软雅黑 12}%s{/f}"), g_strPageRange.GetBuffer(0));
			lpCbo->GetItemAt(lpCbo->GetCount()-1)->SetText(strText.GetBuffer(0));
			lpCbo->SelectItem(lpCbo->GetCount()-1);
		}
		else
		{
			// 创建一个item加入Combo
			CListLabelElementUI *lpItem = new CListLabelElementUI;
			CString strAtt;
			strAtt.Format(_T("height=\"18\" text=\"{f 微软雅黑 12}%s{/f}\" visible=\"true\" "), g_strPageRange);
			lpItem->ApplyAttributeList(strAtt.GetBuffer(0));
			lpCbo->Add(lpItem);
			lpCbo->SelectItem(lpCbo->GetCount()-1);
		}
	}
	else
	{
		lpCbo->SelectItem(0);
	}

	_endthreadex(0);
	return 0;
}

void CMyWnd::InitWindow()
{
	::SetWindowText(m_hWnd, _T("晨光PDF转WORD"));

	m_lpList = static_cast<CListUI*>(m_PaintManager.FindControl(_T("listFiles")));
	m_lpTopLeft = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("ListTopLeft")));
	m_lpListBottom = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("ListBottom")));
// 	m_lpListBottom->SetVisible(FALSE);

	m_lpList_man = static_cast<CListUI*>(m_PaintManager.FindControl(_T("listManMake")));
	m_lpDragFile = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnDragFile")));

	m_lpSelAllOpt = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("checkSelectAll2")));	//全选
// 	m_lpSelAllOpt1 = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("checkSelectAll1")));	//全选
	m_lpbtnDelete = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnDelete")));	//删除
	
	m_lpSavePathOpt = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("Save_PDFPath")));	//保存到pdf文件相同的目录

	m_lpSelSavePathOpt = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("Save_CustomPath")));	//自定义目录

	m_lpSavePathEdt = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("editPath")));	//保存路径edit
	m_lpSavePathEdt->SetText(g_strSavePath.GetBuffer(0));

	m_lpOpenPathBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnSetPath")));	//打开目录

	m_lpOutDocForma = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("comboOutDocFormat")));	

	m_lpVipBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("UserLogin")));	//开通vip

	m_lpbtnConvert = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnConvert")));	//开始转换

// 	m_lpFileState = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("statusText")));	//页数超过5页
// 
// 	m_lpbtnOpenFile  = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnOpenFile")));	//打开文件
// 
// 	m_lpbtnOpenFolder = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btnOpenFolder")));	//打开目录

	m_lpTabLayout = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("Tab_main")));

	m_lpTabLayout->SelectItem(0);	//测试

	m_lpWebTj = static_cast<CWebBrowserUI*>(m_PaintManager.FindControl(_T("webtj_main")));	//统计

	if (NULL != m_lpWebTj)
	{
		m_lpWebTj->SetVisible();
		//获取每个安装包下载的渠道ID
		CString strTjUrl = _T("http://update.bskrt.com/pdf/lc.html");
		m_lpWebTj->Navigate2(strTjUrl.GetBuffer(0));
		m_lpWebTj->SetVisible(FALSE);
	}

	//设置滚动条图片
	CScrollBarUI* pShortListVer = m_lpList->GetVerticalScrollBar();
	if (NULL != pShortListVer)
	{
		pShortListVer->SetAttribute(_T("width"),_T("13"));

		pShortListVer->SetBkImage(_T("scrollbk.png"));
		pShortListVer->SetButton1NormalImage(_T("file='scroll_top_button.png' source='0,0,13,12'"));
		pShortListVer->SetButton1HotImage(_T("file='scroll_top_button.png' source='13,0,26,12'"));
		pShortListVer->SetButton1PushedImage(_T("file='scroll_top_button.png' source='26,0,39,12'"));
		pShortListVer->SetButton2NormalImage(_T("file='scroll_bottom_button.png' source='0,0,13,12'"));
		pShortListVer->SetButton2HotImage(_T("file='scroll_bottom_button.png' source='13,0,26,12'"));
		pShortListVer->SetButton2PushedImage(_T("file='scroll_bottom_button.png' source='26,0,39,12'"));
		pShortListVer->SetThumbNormalImage(_T("file='scroll.png' source='0,0,13,327'"));
		pShortListVer->SetThumbHotImage(_T("file='scroll.png' source='13,0,26,327'"));
		pShortListVer->SetThumbPushedImage(_T("file='scroll.png' source='26,0,39,327'"));
	}

	//初始化转换属性
	InitConnectSetting();

	//启动线程检测是否是VIP
	_beginthreadex(NULL, 0, CheckVipUserFunc, (LPVOID)this, 0, NULL);

	//启动线程检测BCL是否正确安装
	_beginthreadex(NULL, 0, CheckBCLIsOkFunc, (LPVOID)this, 0, NULL);

	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
};

//删除列表后刷新
void CMyWnd::RefreshListWhenDel()
{
	if (GetVecTrueNum() <= 0)	// 这里要获取有效的文件个数
	{
		m_lpList->SetVisible(FALSE);
		m_lpTopLeft->SetVisible(FALSE);
		m_lpListBottom->SetVisible(FALSE);
		m_lpDragFile->SetVisible();

		//清空 vector 游标归0
		g_vecPdfInfo.clear();
		g_nVecCursor = 0;
	}
}

//增加文件后刷新要处理的文件列表
void CMyWnd::RefreshFileListWhenAddFiles()
{
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WhenAddFilesFunc, (LPVOID)this, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
}

void CMyWnd::RefreshFileList()
{
	if (GetVecTrueNum() > 0)
	{
		m_lpList->SetVisible();
		m_lpTopLeft->SetVisible();
		m_lpListBottom->SetVisible(TRUE);
		m_lpDragFile->SetVisible(FALSE);
	}
// 	else
// 	{
// 		m_lpList->SetVisible(FALSE);
// 		m_lpTopLeft->SetVisible(FALSE);
// 		m_lpListBottom->SetVisible(FALSE);
// 		m_lpDragFile->SetVisible(TRUE);
// 		return ;
// 	}

	for (int i = g_nVecCursor; i < g_vecPdfInfo.size(); i++)
	{
		if (g_vecPdfInfo[i].bIsDel)
		{
			continue;
		}

		CDialogBuilder builder;
		CListContainerElementUI* pUI =  static_cast<CListContainerElementUI*>( builder.Create( _T("file_list_item.xml") , 0 , NULL , &m_PaintManager , NULL ) );
		pUI->FindSubControl(_T("fileName"))->SetText(g_vecPdfInfo[i].strFileName.GetBuffer(0));	//文件名称
		pUI->FindSubControl(_T("fileName"))->SetToolTip(g_vecPdfInfo[i].strFileName.GetBuffer(0));

		pUI->FindSubControl(_T("fileSize"))->SetText(g_vecPdfInfo[i].strFileSize.GetBuffer(0));	//文件大小

		if (!g_bIsVip)
		{
			//如果页数超过5页 设置状态
			int nPage = _ttoi(g_vecPdfInfo[i].strFileSize.GetBuffer(0));
			if (nPage > 5)
			{
				pUI->FindSubControl(_T("statusText"))->SetVisible(TRUE);
				pUI->FindSubControl(_T("statusText"))->SetText(_T("页数超过5页")); //状态
			}
		}

		CString strPages;
		strPages.Format(_T("%d"), g_vecPdfInfo[i].nFilePages);
		pUI->FindSubControl(_T("pageCount"))->SetText(strPages.GetBuffer(0));	//文件页数

		pUI->SetTag(i);	//记录在vector中的索引

		m_lpList->Add(pUI);

		g_nVecCursor++;
	}

}

//获取字符串中指定key的值
CString GetVauleByKey(CString strList, CString strKey)
{
	CString strValue;

	int nPos = strList.Find(strKey.GetBuffer(0));
	if (-1 != nPos)
	{
		//找到结尾 , 或 }
		int nEnd = strList.Find(_T(','), nPos);

		if (-1 == nEnd)
		{
			nEnd = strList.Find(_T('}'), nPos);
		}

		strValue = strList.Mid(nPos + strKey.GetLength(), nEnd - nPos - strKey.GetLength());
		strValue.Replace(_T("\""), _T(""));
		strValue.Replace(_T(":"), _T(""));
		strValue.Replace(_T(","), _T(""));
		strValue.Replace(_T("}"), _T(""));
	}

	return strValue;
}

//刷新人工转换订单列表
void CMyWnd::RefreshManMakeList()
{
	//先全部清空
	//界面上的和g_vecManMakeInfo的记录
	m_lpList_man->RemoveAll();
	g_vecManMakeInfo.clear();

	//再重新获取所有订单
	SYSTEMTIME st;
	GetSystemTime(&st);
	CTime tUTC(st);
	CString strTime;
	strTime.Format(_T("%d"), tUTC.GetTime());

	CString strSign;
	strSign.Format(_T("%s|%s|quxiupdf2word"), g_strMachineCode, strTime);
	CString strSignMd5 = MakeCStringMD5W(strSign);

	CString strUrl = _T("http://www.pdf00.com/order/get_orders?mn=[mn]&time=[time]&sign=[sign]");
	strUrl.Replace(_T("[mn]"), g_strMachineCode);
	strUrl.Replace(_T("[time]"), strTime);
	strUrl.Replace(_T("[sign]"), strSignMd5);

	CString strListInfo = GetHtmlStr(strUrl);

	int nPos = strListInfo.Find(_T("\"order_num\""));

	while (nPos != -1)
	{
		int nEnd = strListInfo.Find(_T("}"), nPos);

		CString strOneOrder = strListInfo.Mid(nPos - 1, nEnd - nPos + 2);

		OrderInfo tagOrder;
		tagOrder.strOrderID = GetStrFormJson(strOneOrder, _T("order_num"), _T(""), 0);	//订单编号
		CString strFileName = GetVauleByKey(strOneOrder, _T("filename"));	//文件名
		CStringA strFileNameA = CStrW2CStrA(strFileName);
		char *lpFileName = strFileNameA.GetBuffer(0);
		CStringA strFileNameAA = UnEscape(lpFileName).c_str();
		tagOrder.strFileName = CStrA2CStrW(strFileNameAA);

		tagOrder.strPageCount = GetStrFormJson(strOneOrder, _T("fpage"), _T(""), 0);	//文件页数
		
		CString strOrderMoney;
		strOrderMoney.Format(_T("%s元"), GetStrFormJson(strOneOrder, _T("order_money"), _T(""), 0));	//订单金额
		tagOrder.strAmt = strOrderMoney;

		CString strOrder_state = GetStrFormJson(strOneOrder, _T("order_state"), _T(""), 0);	//未付款|已付款
		if (_T("0") == strOrder_state || strOrder_state.IsEmpty())
		{
			tagOrder.strOrderState = _T("未付款");
		}
		else
		{
			tagOrder.strOrderState = _T("已付款");
		}

		CString strViptype3_state = GetStrFormJson(strOneOrder, _T("viptype3_state"), _T(""), 0);	//待处理|已处理
		if (_T("0") == strViptype3_state || strViptype3_state.IsEmpty())
		{
			tagOrder.strJinDu = _T("待处理");
		}
		else
		{
			tagOrder.strJinDu = _T("已处理");
		}

		tagOrder.strDownLoadUrl = GetStrFormJson(strOneOrder, _T("fedit"), _T(""), 0);	//word文件下载地址
		g_vecManMakeInfo.push_back(tagOrder);

		nPos = strListInfo.Find(_T("\"order_num\""), nEnd);
	}

	//加入列表
	for (int i = 0; i < g_vecManMakeInfo.size(); i++)
	{
		CDialogBuilder builder;
		CListContainerElementUI* pUI =  static_cast<CListContainerElementUI*>( builder.Create( _T("file_list_item_manmake.xml") , 0 , NULL , &m_PaintManager , NULL ) );
		pUI->FindSubControl(_T("fileName_man"))->SetText(g_vecManMakeInfo[i].strFileName.GetBuffer(0));	//文件名称
		pUI->FindSubControl(_T("fileName_man"))->SetToolTip(g_vecManMakeInfo[i].strFileName.GetBuffer(0));

		pUI->FindSubControl(_T("pageCount_man"))->SetText(g_vecManMakeInfo[i].strPageCount.GetBuffer(0));	//文件页数

		pUI->FindSubControl(_T("money_man"))->SetText(g_vecManMakeInfo[i].strAmt.GetBuffer(0));	//金额

		pUI->FindSubControl(_T("PayState_man"))->SetText(g_vecManMakeInfo[i].strOrderState.GetBuffer(0));	//订单状态
		if (_T("未付款") == g_vecManMakeInfo[i].strOrderState)
		{
			pUI->FindSubControl(_T("btnpay_man"))->SetVisible(TRUE);
		}

		pUI->FindSubControl(_T("jingdu_man"))->SetText(g_vecManMakeInfo[i].strJinDu.GetBuffer(0));	//处理状态
		if (_T("已处理") == g_vecManMakeInfo[i].strJinDu)
		{
			pUI->FindSubControl(_T("btnDownFile_down"))->SetVisible(TRUE);
		}

// 		pUI->FindSubControl(_T("jingdu_man"))->SetText(g_vecManMakeInfo[i].strJinDu.GetBuffer(0));	//处理状态

		pUI->SetTag(i);	//记录在vector中的索引

		m_lpList_man->Add(pUI);

	}
}

//在线咨询客服
void SayHelloOnLineByQQ()
{
	// 获取qq启动路径
	HKEY hKey;
	TCHAR szProductType[MAX_PATH] = _T("");
	DWORD dwBufLen = MAX_PATH;
	LONG lRet;

	if(RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("SOFTWARE\\Tencent\\PlatForm_Type_List\\1"),
		0,
		KEY_QUERY_VALUE,
		&hKey) == ERROR_SUCCESS)
	{
		lRet = RegQueryValueEx(hKey,
			_T("TypePath"),
			NULL,
			NULL,
			(LPBYTE)szProductType,
			&dwBufLen);
	}
	else
	{
		// 没有qq
	}
	RegCloseKey(hKey);

	CString strQQTimvp = szProductType;
	if (strQQTimvp.IsEmpty())
	{
		//提示没有安装qq http://crm2.qq.com/page/portalpage/wpa.php?uin=4000868286&aty=0&a=0&curl=&ty=1

		return ;
	}

	// 启动客服qq聊天
	strQQTimvp.MakeLower();
	strQQTimvp.Replace(_T("qq.exe"), _T("Timwp.exe"));

	ShellExecute(NULL, _T("open"), strQQTimvp.GetBuffer(0), _T("tencent://message/?uin=3321139384&site=qq&menu=yes"), NULL, SW_SHOWNORMAL);
}

void CMyWnd::Notify(TNotifyUI& msg)
{
	CString strName = msg.pSender->GetName();
	if(msg.sType == _T("click")) 
	{
		if (_T("btnMainMenu") == strName)		//菜单
		{
			POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
			CMainMenuWnd *pMenu = new CMainMenuWnd();

			pMenu->Init(*this, pt);
			pMenu->ShowWindow(TRUE);
		}
		else if (_T("btnQQLine") == strName)	//联系客服
		{
			SayHelloOnLineByQQ();
		}
		else if (_T("btnMin") == strName)		//最小化
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if (_T("btnClose") == strName)		//关闭
		{
			Close();
// 			SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
		}
		else if (_T("btnDragFile") == strName || _T("btnAdd2") == strName)		// 添加pdf文件  
		{
			m_lpTabLayout->SelectItem(0);	
// 			if (g_vecPdfInfo.size() > 0)
// 			{
// 				m_lpListBottom->SetVisible(TRUE);
// 			}

			msg.pSender->SetEnabled(FALSE);
// 			_beginthreadex(NULL, 0, OpenAddPdfFileDlgFunc, (LPVOID)this, 0, NULL);
			OpenAddPdfFileDlg();
			msg.pSender->SetEnabled(TRUE);
		}
		else if (_T("btnConvertSetting") == strName)	// 点击设置转换效果
		{
			_beginthreadex(NULL, 0, PopSetWndFunc, (LPVOID)this, 0, NULL);
		}
		else if (_T("btnDeleteFile") == strName)	// 删除一行
		{
			//获取第几行
			int nIndex = (msg.pSender->GetParent()->GetParent())->GetTag();
			int nRow = m_lpList->GetItemIndex(msg.pSender->GetParent()->GetParent());
			m_lpList->RemoveAt(nRow);

			//标记Vector 中已删除
			if (nIndex >= 0)
			{
				g_vecPdfInfo[nIndex].bIsDel = TRUE;
			}

			// 删除后刷新数据
			RefreshListWhenDel();
		}
		else if (_T("btnOpenFile") == strName)	//打开文件
		{
			if (!g_bIsVip)
			{
				CMsgWnd *lpMsgWnd = new CMsgWnd;
				lpMsgWnd->Init(this->GetHWND(), _T("您不是VIP会员，所以只有转换了前5页文档"), 3);
				if (IDOK == lpMsgWnd->ShowModal())
				{
					int nIndex = (msg.pSender->GetParent()->GetParent())->GetTag();
					if (!g_vecPdfInfo[nIndex].strWordFilePath.IsEmpty())
					{
						ShellExecute(NULL, _T("open"), g_vecPdfInfo[nIndex].strWordFilePath.GetBuffer(0) , NULL, NULL, SW_SHOWNORMAL);
					}
				}
			}
			else
			{
				int nIndex = (msg.pSender->GetParent()->GetParent())->GetTag();
				if (!g_vecPdfInfo[nIndex].strWordFilePath.IsEmpty())
				{
					ShellExecute(NULL, _T("open"), g_vecPdfInfo[nIndex].strWordFilePath.GetBuffer(0) , NULL, NULL, SW_SHOWNORMAL);
				}
			}

		}
		else if (_T("btn_qq") == strName)	//订单页面 客服qq
		{
			SayHelloOnLineByQQ();
		}
		else if (_T("btn_RefreshOrder") == strName)		//刷新订单
		{
			RefreshManMakeList();
			ShowMsgTipWnd(this->GetHWND(), _T("订单刷新成功"), 0);
		}
		else if (_T("btnDownFile_down") == strName)		//下载word文件
		{
			//下载
			_beginthreadex(NULL, 0, DownLoadWordFileFunc, (LPVOID)(msg.pSender), 0, NULL);
		}
		else if (_T("btnOpenFile_down") == strName)	//打开Word文件
		{
			int nIndex = msg.pSender->GetParent()->GetParent()->GetTag();
			ShellExecute(NULL, _T("open"), g_vecManMakeInfo[nIndex].strWordFile.GetBuffer(0), NULL, NULL, SW_SHOWNORMAL);
		}
		else if (_T("btnOpenFolder_down") == strName)	//打开Word文件所在目录
		{
			int nIndex = msg.pSender->GetParent()->GetParent()->GetTag();
			int nPos = g_vecManMakeInfo[nIndex].strWordFile.ReverseFind(_T('\\'));
			if (-1 != nPos)
			{
				CString strFilePath = g_vecManMakeInfo[nIndex].strWordFile.Mid(0, nPos);
				ShellExecute(NULL, _T("open"), strFilePath.GetBuffer(0), NULL, NULL, SW_SHOWNORMAL);
			}
		}
		else if (_T("btnpay_man") == strName)	//订单支付
		{
			//订单号
			int nIndex = (msg.pSender->GetParent()->GetParent())->GetTag();
			CString strOrderID = g_vecManMakeInfo[nIndex].strOrderID;

			if (!strOrderID.IsEmpty())
			{
				SYSTEMTIME st;
				GetSystemTime(&st);
				CTime tUTC(st);
				CString strTime;
				strTime.Format(_T("%d"), tUTC.GetTime());

				CString strSign;
				strSign.Format(_T("%s|%s|quxiupdf2word"), strOrderID, strTime);
				CString strSignMd5 = MakeCStringMD5W(strSign);

				CString strUrl= _T("http://www.pdf00.com/order/re_order?ordernum=[ordernum]&time=[time]&sign=[sign]");
				strUrl.Replace(_T("[ordernum]"), strOrderID);
				strUrl.Replace(_T("[time]"), strTime);
				strUrl.Replace(_T("[sign]"), strSignMd5);

				ShellExecute(NULL, _T("open"), strUrl.GetBuffer(0), NULL, NULL, SW_SHOWNORMAL);
			}
		}
		else if (_T("btnDeleteFile_down") == strName)	//删除订单
		{
			//订单号
			int nIndex = (msg.pSender->GetParent()->GetParent())->GetTag();
			CString strOrderID = g_vecManMakeInfo[nIndex].strOrderID;

			//判断该订单能不能删除 已付款的订单不能删
			BOOL bDel = FALSE;
			if (_T("已付款") == g_vecManMakeInfo[nIndex].strOrderState)
			{
				CMsgWnd *lpMsgWnd = new CMsgWnd;
				ShowMsgTipWnd(this->GetHWND(), _T("已支付的订单不能删除"), 0);
				return ;
			}
			else
			{
				//提示确定删除
				CMsgWnd *lpMsgWnd = new CMsgWnd;
				lpMsgWnd->Init(this->GetHWND(), _T("您确定删除您的订单"), 4);
				if (IDOK == lpMsgWnd->ShowModal())
				{
					bDel = TRUE;
				}
			}

			if (!strOrderID.IsEmpty() && bDel)
			{
				SYSTEMTIME st;
				GetSystemTime(&st);
				CTime tUTC(st);
				CString strTime;
				strTime.Format(_T("%d"), tUTC.GetTime());

				CString strSign;
				strSign.Format(_T("%s|%s|%s|quxiupdf2word"), strOrderID,g_strMachineCode, strTime);
				CString strSignMd5 = MakeCStringMD5W(strSign);

				CString strUrl= _T("http://www.pdf00.com/order/hide_order?ordernum=[ordernum]&mn=[mn]&time=[time]&sign=[sign]");
				strUrl.Replace(_T("[ordernum]"), strOrderID);
				strUrl.Replace(_T("[mn]"), g_strMachineCode);
				strUrl.Replace(_T("[time]"), strTime);
				strUrl.Replace(_T("[sign]"), strSignMd5);

				//执行删除
				CString strUrlInfo = GetHtmlStr(strUrl);
				CString strCode = GetStrFormJson(strUrlInfo, _T("code"), _T(""), 1);
				if (_T("200") == strCode)
				{
					//成功 刷新界面
					RefreshManMakeList();
				}

			}
		}
		else if (_T("btnOpenFolder") == strName)	//打开文件所在目录
		{
			if (!g_bIsVip)
			{
				CMsgWnd *lpMsgWnd = new CMsgWnd;
				lpMsgWnd->Init(this->GetHWND(), _T("您不是VIP会员，所以只有转换了前5页文档"), 3);
				if (IDOK == lpMsgWnd->ShowModal())
				{
					int nIndex = (msg.pSender->GetParent()->GetParent())->GetTag();
					CString strFilePath = g_vecPdfInfo[nIndex].strWordFilePath;
					int nPos = strFilePath.ReverseFind(_T('\\'));
					CString strPath = strFilePath.Mid(0, nPos);
					ShellExecute(NULL, _T("open"), strPath.GetBuffer(0) , NULL, NULL, SW_SHOWNORMAL);
				}
			}
			else
			{
				int nIndex = (msg.pSender->GetParent()->GetParent())->GetTag();
				CString strFilePath = g_vecPdfInfo[nIndex].strWordFilePath;
				int nPos = strFilePath.ReverseFind(_T('\\'));
				CString strPath = strFilePath.Mid(0, nPos);
				ShellExecute(NULL, _T("open"), strPath.GetBuffer(0) , NULL, NULL, SW_SHOWNORMAL);
			}
		}
		else if (_T("btnOpenFolder2") == strName)	//右侧打开目录
		{
			ShellExecute(NULL, _T("open"), g_strSavePath.GetBuffer(0) , NULL, NULL, SW_SHOWNORMAL);
		}
		else if (_T("btnDelete") == strName)	//删除
		{
			//
			for (int i = m_lpList->GetCount() - 1; i >= 0; i--)
			{
				//判断有没有选中
				COptionUI *lpSel = ((COptionUI*)(((CListContainerElementUI*)(m_lpList->GetItemAt(i)))->FindSubControl(_T("checkFile"))));
				if (lpSel->IsSelected())
				{
					int nIndex = (lpSel->GetParent()->GetParent())->GetTag();
					m_lpList->RemoveAt(i);

					//标记Vector 中已删除
					if (nIndex >= 0)
					{
						g_vecPdfInfo[nIndex].bIsDel = TRUE;
					}
				}
			}

			//删除后刷新
			RefreshListWhenDel();
		}
		else if (_T("btnSetPath") == strName)	//打开选择保存doc文件目录
		{
			TCHAR chPath[255];  //用来存储路径的字符串
			CString strPath;
			BROWSEINFO bInfo;
			GetModuleFileName(NULL,chPath,MAX_PATH);
			strPath=chPath;
			ZeroMemory(&bInfo, sizeof(bInfo));
			bInfo.hwndOwner = m_hWnd;
			bInfo.lpszTitle = _T("请选择路径: ");    
			bInfo.ulFlags   = BIF_RETURNONLYFSDIRS|BIF_EDITBOX;
// 			bInfo.lpfn      = BrowseCallbackProc;
			bInfo.lParam    = (LPARAM)strPath.GetBuffer(strPath.GetLength());

			LPITEMIDLIST lpDlist;  //用来保存返回信息的IDList
			lpDlist = SHBrowseForFolder(&bInfo) ; //显示选择对话框
			if(lpDlist != NULL)  //用户按了确定按钮
			{
				SHGetPathFromIDList(lpDlist, chPath);//把项目标识列表转化成字符串
				strPath = chPath; //将TCHAR类型的字符串转换为CString类型的字符串
				if (strPath.GetLength() > 3)
				{
					g_strSavePath = strPath + _T("\\");
				}
				else
				{
					g_strSavePath = strPath;
				}
				m_lpSavePathEdt->SetText(g_strSavePath.GetBuffer(0));
			}
		}
		else if (_T("btnManMake") == strName)	// 人工转换
		{
			m_lpTabLayout->SelectItem(1);	//
			RefreshManMakeList();

			CManMakeWnd *lpWnd = new CManMakeWnd;
			lpWnd->Init();
			lpWnd->ShowModal();
		}
		else if (_T("UserLogin") == strName)	//升级VIP
		{
			if (!g_bIsVip)
			{
				CVIPWizardWnd *lpWnd = new CVIPWizardWnd;
				lpWnd->Init();
				lpWnd->m_lpCloseBtn->SetVisible(FALSE);
				lpWnd->m_lpExitBtn->SetVisible(TRUE);
				lpWnd->ShowModal();
			}
			else
			{
				CString strMsg;
				if (_T("2") == g_tagVipInfo.strType)
				{
					strMsg = _T("您已经是永久会员，可以终生使用");
					ShowMsgTipWnd(this->GetHWND(), strMsg.GetBuffer(0), 0);
// 					CMsgWnd *lpMsg = new CMsgWnd;
// 					lpMsg->Init(strMsg.GetBuffer(0), 0);
				}
				else
				{
					if (g_tagVipInfo.strNowDate > g_tagVipInfo.strEndDate)
					{
						strMsg.Format(_T("您是年费会员，到期时间 %s 已过期，需要续费才可以继续使用！"), g_tagVipInfo.strEndDate);
						ShowMsgTipWnd(this->GetHWND(), strMsg.GetBuffer(0), 2);
// 						CMsgWnd *lpMsg = new CMsgWnd;
// 						lpMsg->Init(strMsg.GetBuffer(0), 2);
					}
					else
					{
						strMsg.Format(_T("您是年费会员，到期时间 %s"), g_tagVipInfo.strEndDate);
						ShowMsgTipWnd(this->GetHWND(), strMsg.GetBuffer(0), 0);
// 						CMsgWnd *lpMsg = new CMsgWnd;
// 						lpMsg->Init(strMsg.GetBuffer(0), 0);
					}
				}
			}
		}
		else if (_T("btnConvert") == strName)	//开始转换
		{
			m_lpTabLayout->SelectItem(0);
			_beginthreadex(NULL, 0, BeginConnectFunc, (LPVOID)this, 0, NULL);
		}
	}
	else if(msg.sType == _T("itemselect"))
	{
		if (_T("comboPageRange") == strName)	//点击转换所有的页的时候
		{
			CComboUI *lpCombo = static_cast<CComboUI*>(msg.pSender);

			if (1 == lpCombo->GetCurSel())	// ... 设置选中范围
			{
				//判断有没有开通vip
				if (g_bIsVip)	//已开通
				{
					_beginthreadex(NULL, 0, PopPageRangeWndFunc, (LPVOID)lpCombo, 0, NULL);
				}
				else
				{
					lpCombo->SelectItem(0);
					_beginthreadex(NULL, 0, NotVipWantToFunc, (LPVOID)this, 0, NULL);
				}
			}
		}
		else if (_T("comboOutDocFormat") == strName)
		{
			CComboUI *lpCombo = static_cast<CComboUI*>(msg.pSender);
			int nIndex = lpCombo->GetCurSel();
			g_tagConnectSetting.nPutoutExt = nIndex;
		}
	}
	else if(msg.sType == _T("selectchanged"))
	{
		// 全选选中
		if (_T("checkSelectAll2") == strName)
		{
			if (((COptionUI*)msg.pSender)->IsSelected())
			{
				for (int i = 0; i < m_lpList->GetCount(); i++)
				{
					((COptionUI*)(((CListContainerElementUI*)(m_lpList->GetItemAt(i)))->FindSubControl(_T("checkFile"))))->Selected(TRUE);
				}
			}
			else
			{
				for (int i = 0; i < m_lpList->GetCount(); i++)
				{
					((COptionUI*)(((CListContainerElementUI*)(m_lpList->GetItemAt(i)))->FindSubControl(_T("checkFile"))))->Selected(FALSE);
				}
			}
		}
		else if (_T("Save_PDFPath") == strName)
		{
			if (m_lpSavePathOpt->IsSelected())	//保存到pdf目录
			{
				m_lpSavePathEdt->SetEnabled(FALSE);
				m_lpOpenPathBtn->SetEnabled(FALSE);
			}
		}
		else if (_T("Save_CustomPath") == strName)	
		{
			if (m_lpSelSavePathOpt->IsSelected())	//保存到自定义目录
			{
				m_lpSavePathEdt->SetEnabled(TRUE);
				m_lpOpenPathBtn->SetEnabled(TRUE);
			}
		}
	}

		__super::Notify(msg);
};

//打开选择pdf文件对话框
void CMyWnd::OpenAddPdfFileDlg()
{
// 	AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0);	//使用mfc CFIledlg	无法给定父窗口
// 
// 	//创建一个可以选择多个文件的CFileDialog  
// 	TCHAR* filters = _T("PDF文件(*.pdf)|*.pdf||");
// 	CFileDialog fileDlg(true,NULL,_T("*.pdf"),OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_HIDEREADONLY, filters);
// 
// 	TCHAR szLargeBuf[1024*10] = _T("");
// 
// 	fileDlg.m_ofn.lpstrTitle = _T("请选择要转换的PDF文件：");
// 	fileDlg.m_ofn.lpstrFile = szLargeBuf;			//多选文件时 默认长度只有260 选不了几个 所以自定义长度 
// 	fileDlg.m_ofn.nMaxFile = 1024*10;
// 
// 	if (fileDlg.DoModal () == IDOK)
// 	{
// 		CString strFilePath;
// 		CString strFileName;
// 
// 		//获取第一个文件的位置  
// 		POSITION pos_file;  
// 		pos_file = fileDlg.GetStartPosition(); 
// 
// 		while(NULL != pos_file)
// 		{
// 			//全路径
// 			strFilePath = fileDlg.GetNextPathName(pos_file);
// 			AddPdfFile2Vec(strFilePath);
// 		}
// 	}

	TCHAR szOpenFileNames[200*MAX_PATH] = _T("");
	TCHAR szPath[MAX_PATH] = _T("");
	TCHAR* p; 
	TCHAR szFileName[MAX_PATH] = _T("");

	OPENFILENAME file = {0};
	file.lpstrTitle = _T("请选择要转换的PDF文件：");
	file.lStructSize = sizeof(file);

	file.hwndOwner = this->GetHWND();
	file.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;  
	file.lpstrFile = szOpenFileNames;
	file.nMaxFile = 200*MAX_PATH;

	file.lpstrFilter = _T("PDF文件(*.pdf)\0*.pdf");
	file.nFilterIndex = 1;//默认选择第一个
	// 弹出打开文件的对话框
	if(::GetOpenFileName(&file))
	{
		// 获取文件夹路径szPath  
		lstrcpyn(szPath, szOpenFileNames, file.nFileOffset );  
		lstrcat(szPath, _T("\\"));   // 末尾加上反斜杠  

		p = szOpenFileNames + file.nFileOffset; //把指针移到第一个文件  

		while(*p)  
		{
			lstrcat(szFileName, szPath);  //给文件名加上路径    
			lstrcat(szFileName, p);       //加上文件名       

			AddPdfFile2Vec(szFileName);

			p += lstrlen(p) +1;           //移至下一个文件 
			memset(szFileName, 0, sizeof(TCHAR) * MAX_PATH);
		}
	}

}

//根据全路径增加文件信息到vec
void CMyWnd::AddPdfFile2Vec(CString strFilePath)
{
	CString strExt = strFilePath.Right(4);
	strExt.MakeLower();

	if (_T(".pdf") != strExt)
	{
		return ;
	}

	//判断文件是否已经存在
	for (int i = 0; i < g_vecPdfInfo.size(); i++)
	{
		CString strPath = g_vecPdfInfo[i].strFilePath;
		if ((strFilePath == strPath))
		{
			if (!(g_vecPdfInfo[i].bIsDel)) //存在且没有删除
			{
				return ;
			}
		}
	}

	//文件名
	CString strFileName  = strFilePath.Mid(strFilePath.ReverseFind(_T('\\'))+1);

	//获取文件大小
	CString strFileSize = M_GetFileSize(strFilePath);

	//获取文件页数
	int nPages = 0;

	//记录文件信息
	Pdf_Info pdfinfo;
	pdfinfo.strFilePath = strFilePath;
	pdfinfo.strFileName = strFileName;
	pdfinfo.strFileSize = strFileSize;
	pdfinfo.nFilePages = nPages;
	pdfinfo.bIsDel = FALSE;	// 新增肯定没删除
	pdfinfo.nTag = 0;	//新增可定是未转换

	g_vecPdfInfo.push_back(pdfinfo);

	//获取文件
	::CoInitialize(NULL);
	EasyConverterLib::IPDF2WordPtr pConvObj = NULL;
	try
	{
		pConvObj.CreateInstance ("EasyConverter.PDF2Word.5"); 
		pConvObj->LicenseKey = GetBCLKey().GetBuffer(0);	//正式版OEM license key 20161221 14个月

		nPages = pConvObj->GetNumberOfPages(strFilePath.GetBuffer(0));	
		g_vecPdfInfo[g_vecPdfInfo.size() - 1].nFilePages = nPages;
	}
	catch(_com_error &e) //表示有密码或文件损坏
	{
		//此处全部归于有密码
		{
			CInputPWWnd *lpWnd = new CInputPWWnd;
			lpWnd->Init(g_vecPdfInfo.size() - 1);
			if (IDCANCEL == lpWnd->ShowModal())
			{
				vector<Pdf_Info>::iterator it = g_vecPdfInfo.begin() + g_vecPdfInfo.size() - 1;
				g_vecPdfInfo.erase(it);
			}
		}
	}

	pConvObj = NULL; 
	::CoUninitialize();

	RefreshFileListWhenAddFiles();
};

//拖动文件到对话框
void CMyWnd::DropFileOnDlg(HDROP hDrop)
{
	int DropCount = DragQueryFile(hDrop, -1, NULL, 0);
	TCHAR szDropFilePath[MAX_PATH] = _T("");

	m_lpTabLayout->SelectItem(0);

	for (int i = 0; i < DropCount; i++)
	{
		DragQueryFile(hDrop, i, szDropFilePath, MAX_PATH);

		AddPdfFile2Vec(szDropFilePath);
	}

	DragFinish(hDrop);  
};

LRESULT CMyWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DROPFILES:	//拖动文件到对话框
		{
			DropFileOnDlg((HDROP)wParam);
			RefreshFileListWhenAddFiles();
		}
		break;

	default:
		break;
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
};

//判断行是否是选中的
BOOL CMyWnd::ChkRowIsSelect(int nRow)
{
	CListContainerElementUI *lpRow = (CListContainerElementUI*)m_lpList->GetItemAt(nRow);
	CCheckBoxUI *lpCheckBox =  (CCheckBoxUI*)lpRow->FindSubControl(_T("checkFile"));

	return lpCheckBox->IsSelected();
}

//获取行所对应vector 中的索引
int CMyWnd::GetRowInVectorIndex(int nRow)
{
	int nIndex = -1;

	CListContainerElementUI *lpRow = (CListContainerElementUI*)m_lpList->GetItemAt(nRow);

	nIndex = lpRow->GetTag();

	return nIndex;
}


