// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
//为了使用CString 
#include <afx.h>

// Windows 头文件:
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: 在此处引用程序需要的其他头文件
#include <shellapi.h>
#include <vector>
#include <process.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <afxdlgs.h>	//使用CFileDialog
using namespace std;


//duilib 库
#include "..\\..\\..\\duilib\\DuiLib\\UIlib.h"
#pragma comment(lib, "..\\..\\..\\Lib\\DuiLib_d.lib")
using namespace DuiLib;

//BCL sdk
// #import "progid:EasyConverter.PDF2Word.5"
// #import "progid:EasyConverter.PDF2Word.5"

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <comdef.h>
#include <regex>	//为了使用正则表达式

#include "Reg.h"
#include "MD5_MBCS.h"

//Json字符串操作 jsoncpp库 d:\xwork\jsoncpp\include\json\json.h
#include "..\\..\\..\\jsoncpp\\include\\json\\json.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\..\\Lib\\json_d.lib")
#else
#pragma comment(lib, "..\\..\\..\\Lib\\json.lib")
#endif

////////////////////////////////
extern HWND g_hMainWnd;		//主窗口句柄
extern HINSTANCE g_hInstance;

extern CString g_strPath;
extern CString g_strMainIni;

#include "MainWnd.h"
extern CMyWnd *g_MainWnd;


typedef struct tag_PdfFile_info 
{
	CString strFilePath;	//全路径

	CString strFileName;	//文件名

	CString strFileSize;	//文件大小 (Mb)

	int nFilePages; //文件页数

	BOOL bIsDel;	//是否已经从表格中移除了

	int nTag;		// 已转还是待转 0表示待转, 1表示已转

	CString strPageRange;	// 格式: 1-100

	CString strPassWord;	//文件密码

	CString strWordFilePath;	//doc文件全路径

} Pdf_Info;
extern vector<Pdf_Info> g_vecPdfInfo;

extern CString g_strSavePath;	// doc文件保存路径

extern CString g_strPageRange;	//设置页数范围

//转换设置属性结构
typedef struct tag_Set_Connect 
{
	int nConversionMethod;	//版面设置

	BOOL bAdjustFontNames;	//校正字体名称

	BOOL bAdjustSpacing;	//校正间距

	BOOL bExtractImages;	//包含图片

	BOOL bExtractText;		//包含文字

	BOOL bShrinkFonts;		//缩小字体

	BOOL bDetectHeader;		//检查头部

	BOOL bSaveHiddenText;	//保留隐藏文本

	int nDocumentType;		//文档类型： 单列 多列 自适应

	int nPutoutExt;			//输出格式: RTF文件 DOC文件 DOCX文件

	int nBkmkOpt;			//输出书签：不转换书签 每页开头保留书签 和PDF一样

	int nOcrOpt;			//OCR文档：保留图片和文字 仅保留文字 仅保留图片

	int nTimeout;			//等待转换最长时间 分钟

	BOOL bOpenPath;			//转换成功后自动打开Word所在的目录

} Set_Connect;

extern Set_Connect g_tagConnectSetting;	//默认属性设置

//人工转换订单属性列表
typedef struct tag_order_Info
{
	CString strOrderID;			//订单编号

	CString strFileName;		//文件名

	CString strPageCount;		//文件页数

	CString strAmt;				//金额

	CString strOrderState;		//订单状态	//已支付|未支付

	CString strJinDu;			//处理进度	//待处理

	CString strDownLoadUrl;		//转换后文件下载地址

	BOOL bIsDownLoadOK;			//已经下载Word文件了

	CString strWordFile;		//下载下来的word文件群路径

} OrderInfo;

extern vector<OrderInfo> g_vecManMakeInfo;	//人工转换订单列表

//初始化默认转换属性
extern void InitConnectSetting();

//设置转换属性
extern void SetConnectSetting(int nConversionMethod, BOOL bAdjustFontNames,
					   BOOL bAdjustSpacing, BOOL bExtractImages, BOOL bExtractText, BOOL bShrinkFonts,
					   BOOL bDetectHeader, BOOL bSaveHiddenText, int nDocumentType, int nPutoutExt,
					   int nBkmkOpt, int nOcrOpt, int nTimeout, int bOpenPath);


//会员信息
typedef struct tag_Vip_Info
{
	CString strType;	//0非会员 1年费 2永久

	CString strPayDate;	//支付时间

	CString strEndDate;	//到期时间

	CString strNowDate;	//当前时间

} VIP_Info;
extern VIP_Info g_tagVipInfo;	//会员信息

//sdk
// extern EasyConverterLib::IPDF2WordPtr g_pConvObj;

//转换一个文件函数
extern void BeginConvert(int nIndex);

//初始化SDK
extern BOOL InitConvertSDK();

//获取PDF文件页数 返回-1表示有密码
extern int GetPdfFilePages(CString strFilePath);

//获取文件大小 (Mb)
extern CString M_GetFileSize(CString strFilePath);

extern CProgressUI *g_lpProcess;	//当前转换进度条

extern int g_nNowRow;	//当前正在转换的行

extern CIPdf2WordEvent g_IPdf2WordEvent;	// 转换事件对象

extern BOOL g_bOnlyFilePages;	//只转5页

extern BOOL g_bIsSameAsPdf;	//word文件和pdf文件保存在相同目录

extern BOOL g_bIsVip;

extern int g_nVIPType;	// 0:非会员 1:年费 2:永久

extern EasyConverterLib::IPDF2WordPtr g_pConvObj;

// #include <Wininet.h>
// #pragma comment(lib,"Wininet.lib")
// 
// /*  
// * 本地路径 服务器地址 服务器路径 数据分割信息 端口  
// * 通过以上传入信息 将二进制数据传入web服务器  
// *  
// */    
// extern BOOL Upload(IN CString& FilePath,IN CString& ServerName,IN CString& ObjectName, IN CString& HTTPBoundary,IN INTERNET_PORT &nPort);

// libcurl	需要 Ws2_32.lib Wldap32.lib
// #include "..\\..\\..\\curl-7.26.0\\include\\curl\\curl.h"
// #pragma comment(lib, "..\\..\\..\\Lib\\libcurl_imp.lib")


// 进行Url编码 UTF-8 
extern CString UrlEncode(CString strUnicode);

//获取网页信息
extern CString GetHtmlStr(CString listurl);

extern char* UTF_8ToGB2312(char *pText, int pLen);

extern CStringA CStrW2CStrA(const CStringW &cstrSrcW);

extern CStringW CStrA2CStrW(const CStringA &cstrSrcA);

//判断这个用户是否是VIP strThisMachineCode 表示本地保存的机器码
extern BOOL ThisUserIsVIP(CString strThisMachineCode);

extern CString g_strMachineCode; //机器码

//获取json字符串值 nType = 0|字符串 1|int
extern CString GetStrFormJson(CString strJson, CString strKey1, CString strKey2, int nType);

//在线咨询客服
extern void SayHelloOnLineByQQ();

//支付函数
extern void PayForVip(int nPayType);

extern CString GetBCLKey();

extern void ShowMsgTipWnd(HWND hwnd, CString strMsg, int nTag);

extern string UnEscape(char* p_unicode_escape_chars);
