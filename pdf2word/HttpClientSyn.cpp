#include "stdafx.h"
#include "HttpClientSyn.h"
#include <atlbase.h>
#include <atlconv.h>

#pragma comment(lib,"Winhttp.lib")

#define MAXSTATICPARAMCOUNT 32

CHttpClientSyn::CHttpClientSyn()
{
    m_hSession = NULL;
    m_hConnect = NULL;
    m_hRequest = NULL;
    m_dwTimeout = 0;
    m_lpReceiveData = NULL;
    m_dwReceiveDataLength = 0;
}


CHttpClientSyn::~CHttpClientSyn(void)
{
    ClearEvn();
}

BOOL CHttpClientSyn::InitializeHttp( const std::wstring& wstrUrl, DWORD dwTimeout, EType eType)
{
    BOOL bSuc = FALSE;
    do {
        URL_COMPONENTS urlCom;
        memset(&urlCom, 0, sizeof(urlCom));
        urlCom.dwStructSize = sizeof(urlCom);
        WCHAR wchScheme[64] = {0};
        urlCom.lpszScheme = wchScheme;
        urlCom.dwSchemeLength = ARRAYSIZE(wchScheme);
        WCHAR wchHostName[1024] = {0};
        urlCom.lpszHostName = wchHostName;
        urlCom.dwHostNameLength = ARRAYSIZE(wchHostName);
        WCHAR wchUrlPath[1024] = {0};
        urlCom.lpszUrlPath = wchUrlPath;
        urlCom.dwUrlPathLength = ARRAYSIZE(wchUrlPath);
        WCHAR wchExtraInfo[1024] = {0};
        urlCom.lpszExtraInfo = wchExtraInfo;
        urlCom.dwExtraInfoLength = ARRAYSIZE(wchExtraInfo);

        if ( FALSE == WinHttpCrackUrl( wstrUrl.c_str(), wstrUrl.length(), ICU_ESCAPE, &urlCom) ) {
            break;
        }

        std::wstring wstrExtraInfo;
		if (eGet == eType) {
			wstrExtraInfo = urlCom.lpszExtraInfo;
			ParseParams(wstrExtraInfo);
			AddExtInfo(m_VecExtInfo);
		}

        m_hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 ); 
        if ( NULL == m_hSession ) {
            break;
        }

        if ( FALSE == WinHttpSetTimeouts(m_hSession, dwTimeout, dwTimeout, dwTimeout, dwTimeout) ) {
            break;
        }

        m_hConnect = WinHttpConnect( m_hSession, urlCom.lpszHostName, urlCom.nPort, 0 );
        if ( NULL == m_hConnect ) {
            break;
        }

		m_wstrUrlPath = urlCom.lpszUrlPath;
		if (eGet != eType) {
			m_wstrUrlPath += urlCom.lpszExtraInfo;
		}

        bSuc = TRUE;
    } while (0);
    return bSuc;
}

VOID CHttpClientSyn::UninitializeHttp()
{
    if ( NULL != m_hRequest ) {
        WinHttpCloseHandle(m_hRequest);
        m_hRequest = NULL;
    }

    if ( NULL != m_hConnect ) {
        WinHttpCloseHandle(m_hConnect);
        m_hConnect = NULL;
    }

    if ( NULL != m_hSession ) {
        WinHttpCloseHandle(m_hSession);
        m_hSession = NULL;
    }
}

BOOL CHttpClientSyn::ReceiveData()
{
    BOOL bSuc = FALSE;
    DWORD dwReceivedBufferLength = 0;
    LPBYTE lpReceivedBuffer = NULL;

    do {
        if ( FALSE == WinHttpReceiveResponse( m_hRequest, NULL) ) {
            break;
        }

        DWORD dwRetLength = 0;
        do {
            bSuc = FALSE;
            if ( FALSE == WinHttpQueryDataAvailable(m_hRequest, &dwRetLength ) ) {
                break;
            }

            if ( 0 == dwRetLength ) {
                bSuc = TRUE;
                break;
            }

            LPBYTE lpReceivedData = new BYTE[dwRetLength];
            if ( NULL == lpReceivedData ) {
                break;
            }
            memset(lpReceivedData, 0, dwRetLength);

            DWORD dwRead = 0;
            if ( FALSE == WinHttpReadData(m_hRequest, lpReceivedData, dwRetLength, &dwRead) ) {
                break;
            }

            if ( 0 == dwRead ) {
                break;
            }

            DWORD dwReceivedBufferLengthNew = dwReceivedBufferLength + dwRetLength;
            LPBYTE lpReceivedBufferNew = new BYTE[dwReceivedBufferLengthNew];
            if ( NULL == lpReceivedBufferNew ) {
                break;
            }
            memset(lpReceivedBufferNew, 0, dwReceivedBufferLengthNew);

            if ( NULL != lpReceivedBuffer ) {
                memcpy_s( lpReceivedBufferNew, dwReceivedBufferLengthNew, lpReceivedBuffer, dwReceivedBufferLength);
                delete [] lpReceivedBuffer;
                lpReceivedBuffer = NULL;
            }
            if ( NULL != lpReceivedData ) {
                memcpy_s( lpReceivedBufferNew + dwReceivedBufferLength, 
                    dwReceivedBufferLengthNew - dwReceivedBufferLength, lpReceivedData, dwRetLength);
                delete [] lpReceivedData;
                lpReceivedData = NULL;
            }
            lpReceivedBuffer = lpReceivedBufferNew;
            dwReceivedBufferLength = dwReceivedBufferLengthNew;

            bSuc = TRUE;
        } while ( dwRetLength > 0);
    } while (0);

    if ( bSuc ) {
        m_lpReceiveData = lpReceivedBuffer;
        m_dwReceiveDataLength = dwReceivedBufferLength;

		//xx? 测试
// 		CStringA = (char*)lpReceivedBuffer;
// 		WritePrivateProfileString(_T("Info"), _T("upload"), , _T("C:\\1.ini"));
    }

    return bSuc;
}

BOOL CHttpClientSyn::ReceiveData( LPBYTE lpBuffer, DWORD& dwBufferSize )
{
    BOOL bSuc = FALSE;
    do {
        if ( NULL == m_lpReceiveData ) {
            ::SetLastError(ERROR_WINHTTP_NOT_INITIALIZED);
            break;
        }
        if ( NULL == lpBuffer ) {
            ::SetLastError(ERROR_INVALID_ADDRESS);
            break;
        }

        if ( m_dwReceiveDataLength > dwBufferSize ) {
            dwBufferSize = m_dwReceiveDataLength;
            ::SetLastError(ERROR_INSUFFICIENT_BUFFER);
            break;
        }
        
        errno_t e =  memcpy_s( lpBuffer, dwBufferSize, m_lpReceiveData, m_dwReceiveDataLength);
        if ( 0 != e ) {
            break;
        }

        ClearEvn();

        bSuc = TRUE;
    } while (0);
    return bSuc;
}

VOID CHttpClientSyn::ClearEvn()
{
    UninitializeHttp();
    m_dwTimeout = 0;
    if ( NULL != m_lpReceiveData ) {
        delete [] m_lpReceiveData;
        m_lpReceiveData = NULL;
    }
    m_dwReceiveDataLength = 0;
}

BOOL CHttpClientSyn::TransmiteData( const std::wstring& wstrUrl, EType eType, DWORD dwTimeout)
{
    BOOL bSuc = FALSE;
    do {
        if ( FALSE == InitializeHttp(wstrUrl, dwTimeout, eType)) {
            break;
        }
        if ( FALSE == TransmiteData(eType) ) {
            break;
        }
        ReceiveData();
        UninitializeHttp();
        bSuc = TRUE;
    } while (0);
    return bSuc;
}

BOOL CHttpClientSyn::TransmiteData(EType eType)
{
    BOOL bSuc = FALSE;
    switch (eType) {
    case eGet:{
        bSuc = TransmiteDataToServerByGet();
        }break;
    case ePost:{
        bSuc = TransmiteDataToServerByPost();
        }break;
    case eUpload:{
        bSuc = TransmiteDataToServerByUpload();
        }break;
    default: break;
    }
    return bSuc;
}

BOOL CHttpClientSyn::GetData( LPVOID lpBuffer, DWORD dwBufferSize, DWORD& dwWrite )
{
    return FALSE;
}

BOOL CHttpClientSyn::ModifyRequestHeader( HINTERNET hRequest )
{
    return TRUE;
}

VOID CHttpClientSyn::ParseParams(const std::wstring& wstrExtraInfo)
{
    int nPos = 0;
    nPos = wstrExtraInfo.find('?');
    if ( -1 == nPos ) {
        return;
    }
    std::wstring wstrParam = wstrExtraInfo;
    int nStaticMaxParamCount = MAXSTATICPARAMCOUNT;
    do{
        wstrParam = wstrParam.substr(nPos + 1, wstrExtraInfo.length() - nPos - 1);
        nPos = wstrParam.find('&', nPos);
        std::wstring wstrKeyValuePair;

        if ( -1 == nPos ) {
            wstrKeyValuePair = wstrParam;
        }
        else {
            wstrKeyValuePair = wstrParam.substr(0, nPos);
        }
        
        int nSp = wstrKeyValuePair.find('=');
        if ( -1 != nSp ) {
            StParam stParam;
            stParam.wstrKey = wstrKeyValuePair.substr(0, nSp);
            stParam.wstrValue = wstrKeyValuePair.substr( nSp + 1, wstrKeyValuePair.length() - nSp - 1);
            m_VecExtInfo.push_back(stParam);
        }
    }while(-1 != nPos && nStaticMaxParamCount > 0);
}

BOOL CHttpClientSyn::TransmiteDataToServerByGet()
{
    BOOL bSuc = FALSE;
    do {

        std::wstring wstrUrlPathAppend = m_wstrUrlPath;
        // 采用Get方式时，要将参数放在OpenRequest中
        if ( false == wstrUrlPathAppend.empty() ) {
            wstrUrlPathAppend += L"?";
        }
        wstrUrlPathAppend += GenerateExtInfo(m_VecExtInfo);

        m_hRequest = WinHttpOpenRequest(m_hConnect, L"GET",
            wstrUrlPathAppend.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if ( NULL == m_hRequest ) {
            break;
        }

        ModifyRequestHeader(m_hRequest);

        if ( FALSE == WinHttpSendRequest( m_hRequest, 
            WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) )
        {
            break;
        }

        bSuc = TRUE;
    } while (0);
    return bSuc;

}

BOOL CHttpClientSyn::TransmiteDataToServerByPost()
{
    BOOL bSuc = FALSE;
    do {
        m_hRequest = WinHttpOpenRequest(m_hConnect, L"Post",
            m_wstrUrlPath.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if ( NULL == m_hRequest ) {
            break;
        }

        ModifyRequestHeader(m_hRequest);

        std::wstring wstrExtInfo = GenerateExtInfo(m_VecExtInfo);
        std::string strExtInfo = CW2A(wstrExtInfo.c_str(), CP_UTF8);

        DWORD dwTotal = strExtInfo.length();
        dwTotal += GetDataSize();

        if ( FALSE == WinHttpSendRequest( m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, dwTotal, 0)) {
            break;
        }

        if ( 0 != strExtInfo.length() ) {
            // 默认可以一次全部写完
            if ( FALSE == WinHttpWriteData(m_hRequest, strExtInfo.c_str(), strExtInfo.length(), NULL ) ) {
                break;
            }
        }

        // 静态分配一个数组
        BYTE buffer[1024]= {0};
        BOOL bContinue = FALSE;
        BOOL bSendOK = FALSE;

        do {
            DWORD dwBufferLength = sizeof(buffer);
            SecureZeroMemory(buffer, dwBufferLength);
            DWORD dwWriteSize = 0;
            bContinue = GetData(buffer, dwBufferLength, dwWriteSize);
            if ( 0 != dwWriteSize ) {
                bSendOK= WinHttpWriteData( m_hRequest, buffer, dwWriteSize, NULL);
            }
            else {
                bSendOK = TRUE;
            }
        } while ( bContinue && bSendOK );

        bSuc = bSendOK;

    } while (0);
    return bSuc;
}

void CHttpClientSyn::UserOperating(DWORD dwBufferSize, DWORD dwOnceBufSize)
{

}

BOOL CHttpClientSyn::TransmiteDataToServerByUpload()
{
    BOOL bSuc = FALSE;
    do {
        m_hRequest = WinHttpOpenRequest(m_hConnect, L"Post",
            m_wstrUrlPath.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if ( NULL == m_hRequest ) {
            break;
        }

        ModifyRequestHeader(m_hRequest);

        std::wstring wstrExtInfo = GenerateExtInfo(m_VecExtInfo);
        std::string strExtInfo = CW2A(wstrExtInfo.c_str(), CP_UTF8);

        DWORD dwTotal = strExtInfo.length();
        dwTotal += GetDataSize();
		DWORD dwOne = dwTotal / 100 + 1;	//按百次算 每次要传送的数据大小

        if ( FALSE == WinHttpSendRequest( m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, dwTotal, 0)) {
            break;
        }

        // 静态分配一个数组
        BYTE buffer[1024 * 205]= {0};
        BOOL bContinue = FALSE;
        BOOL bSendOK = FALSE;

        do {
			DWORD dwBufferLength = dwOne > sizeof(buffer) ? sizeof(buffer) : dwOne;

            SecureZeroMemory(buffer, dwBufferLength);
            DWORD dwWriteSize = 0;
            bContinue = GetData(buffer, dwBufferLength, dwWriteSize);
            if ( 0 != dwWriteSize ) {
                bSendOK= WinHttpWriteData( m_hRequest, buffer, dwWriteSize, NULL);
            }
            else {
                bSendOK = TRUE;
            }
			UserOperating(dwTotal, dwWriteSize);
        } while ( bContinue && bSendOK );

        if ( 0 != strExtInfo.length() ) {
            if ( FALSE == WinHttpWriteData(m_hRequest, strExtInfo.c_str(), strExtInfo.length(), NULL ) ) {
                break;
            }
        }
        bSuc = bSendOK;
    } while (0);
    return bSuc;
}
