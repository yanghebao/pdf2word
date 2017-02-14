#include "stdafx.h"
#include "HttpUploadFiles.h"
#include <atlbase.h>
#include <atlstr.h>

#include "ManMakeWnd.h"

#define UNREAD 0xFFFFFFFF

#define BOUNDARYPART L"--387C5529D89343DAB0A4B68591D754B1"

CHttpUploadFiles::CHttpUploadFiles(void)
{
    m_ReadInfo.eType = EHeader;
    m_ReadInfo.dwReadIndex = 0;
	m_dwAlreadySend = 0;
	m_nTimes = 0;
}


CHttpUploadFiles::~CHttpUploadFiles(void)
{
}

DWORD CHttpUploadFiles::GetDataSize()
{
    if ( m_strUploadFileHeaderUTF8.empty() ) {
        return 0;
    }

    DWORD dwFileSize = 0;
    HANDLE hFile = CreateFile( m_wstrFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
    do {
        if ( INVALID_HANDLE_VALUE == hFile ) {
            break;
        }

        LARGE_INTEGER lgFileSize = {0};
        if ( FALSE == GetFileSizeEx(hFile, &lgFileSize) ) {
            break;
        }

        if ( lgFileSize.HighPart > 0 || lgFileSize.LowPart > 0x00FFFFFF) {
            // 限制大小
            break;
        }
        dwFileSize = lgFileSize.LowPart;
    }while(0);
    if ( INVALID_HANDLE_VALUE != hFile ) {
        CloseHandle(hFile);
        hFile = NULL;
    }

    DWORD dwDataSize = 0;
    if ( 0 != dwFileSize ) {
        dwDataSize = dwFileSize + m_strUploadFileHeaderUTF8.length();
    }

    return dwDataSize;
}

BOOL CHttpUploadFiles::GetData( LPVOID lpBuffer, DWORD dwBufferSize, DWORD& dwWrite )
{
    if ( m_strUploadFileHeaderUTF8.empty() ) {
        return FALSE;
    }

    if ( EHeader == m_ReadInfo.eType ) {
        if ( FALSE == ReadFromString(m_strUploadFileHeaderUTF8, lpBuffer, dwBufferSize, m_ReadInfo.dwReadIndex, dwWrite ) ) {
            return FALSE;
        }
        m_ReadInfo.dwReadIndex += dwWrite;
        if ( m_ReadInfo.dwReadIndex == m_strUploadFileHeaderUTF8.length() ) {
            m_ReadInfo.eType = EFile;
            m_ReadInfo.dwReadIndex = 0;
            return TRUE;
        }
    }
    else if ( EFile == m_ReadInfo.eType ){
        OVERLAPPED ov;
        memset(&ov, 0, sizeof(ov));
        ov.Offset = m_ReadInfo.dwReadIndex;

        HANDLE hFile = CreateFile( m_wstrFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
        BOOL bContinue = FALSE;
        DWORD dwFileSize = 0;
        do {
            if ( INVALID_HANDLE_VALUE == hFile ) {
                dwWrite = 0;
                break;
            }

            LARGE_INTEGER lgFileSize = {0};
            if ( FALSE == GetFileSizeEx(hFile, &lgFileSize) ) {
                break;
            }

            if ( FALSE == ReadFile(hFile, lpBuffer, dwBufferSize, &dwWrite, &ov)) {
                break;
            }
            dwFileSize = lgFileSize.LowPart;
            bContinue = TRUE;
        } while (0);
        
        if ( INVALID_HANDLE_VALUE != hFile ) {
            CloseHandle(hFile);
            hFile = NULL;
        }
        m_ReadInfo.dwReadIndex += dwWrite;
        if ( m_ReadInfo.dwReadIndex == dwFileSize ) {
            m_ReadInfo.dwReadIndex = 0;
            bContinue = FALSE;
        }

        return bContinue;
    }

    return TRUE;
}

BOOL CHttpUploadFiles::TransDataToServer( const std::wstring wstrUrl, VecStParam& VecExtInfo, 
    const std::wstring& wstrFilePath,  const std::wstring& wstrFileKey)
{
    m_wstrBlockStart = L"--";
    m_wstrBlockStart += BOUNDARYPART;
    m_wstrBlockStart += L"\r\n";

    m_strBlockStartUTF8 = CW2A(m_wstrBlockStart.c_str(), CP_UTF8);

    m_wstrBlockEnd =  L"\r\n--";
    m_wstrBlockEnd += BOUNDARYPART;
    m_wstrBlockEnd +=  L"--\r\n";

    m_wstrNewHeader = L"Content-Type: multipart/form-data; boundary=";
    m_wstrNewHeader += BOUNDARYPART;
    m_wstrNewHeader += L"\r\n";

    m_wstrFilePath = wstrFilePath;
    
    std::wstring wstrFileName = m_wstrFilePath;
    int nPos = m_wstrFilePath.rfind('\\');
    if ( -1 == nPos ) {
        nPos = m_wstrFilePath.rfind('/');
    }
    if ( -1 != nPos ) {
        wstrFileName = m_wstrFilePath.substr( nPos + 1, m_wstrFilePath.length() - nPos - 1);
    }

    std::wstring wstrUploadFileHeader;
    wstrUploadFileHeader = m_wstrBlockStart;
    wstrUploadFileHeader += L"Content-Disposition: form-data; name=\"";
    wstrUploadFileHeader += wstrFileKey;
    wstrUploadFileHeader += L"\";";
    wstrUploadFileHeader += L"filename=\"";
    wstrUploadFileHeader += wstrFileName;
    wstrUploadFileHeader += L"\"\r\n";
    wstrUploadFileHeader += L"Content-Type:application/pdf \r\n\r\n";

    m_strUploadFileHeaderUTF8 = CW2A(wstrUploadFileHeader.c_str(), CP_UTF8);
    m_VecExtInfo.assign(VecExtInfo.begin(), VecExtInfo.end());

    return TransmiteData(wstrUrl, eUpload, 30 * 1000);
}

BOOL CHttpUploadFiles::ModifyRequestHeader( HINTERNET hRequest )
{
    return ::WinHttpAddRequestHeaders(hRequest, m_wstrNewHeader.c_str(), 
        m_wstrNewHeader.length(), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
}

std::wstring CHttpUploadFiles::GenerateExtInfo( const VecStParam& VecExtInfo )
{
    std::wstring wstrInfo = L"\r\n";
    for ( VecStParamCIter it = VecExtInfo.begin(); it != VecExtInfo.end(); it++ ) {
        wstrInfo += m_wstrBlockStart;
        wstrInfo += L"Content-Disposition:form-data;";
        wstrInfo += L"name=";
        wstrInfo += L"\"";
        wstrInfo += it->wstrKey;
        wstrInfo += L"\"";
        wstrInfo += L"\r\n\r\n";
        wstrInfo += it->wstrValue;
        wstrInfo += L"\r\n";
    }
    wstrInfo += m_wstrBlockEnd;
    return wstrInfo;
}

VOID CHttpUploadFiles::AddExtInfo( VecStParam& VecExtInfo )
{
    for ( VecStParamCIter it = m_VecExtInfo.begin(); it != m_VecExtInfo.end(); it++) {
        VecExtInfo.push_back(*it);
    }
}

BOOL CHttpUploadFiles::ReadFromString( const std::string& strContent,
    LPVOID lpBuffer, DWORD dwBufferSize, DWORD dwIndex, DWORD& dwWrite )
{
    if ( strContent.length() > dwIndex + dwBufferSize) {
        dwWrite = dwBufferSize;
    }
    else {
        dwWrite = strContent.length() - dwIndex;
    }

    if ( 0 != memcpy_s( lpBuffer, dwBufferSize, strContent.c_str() + dwIndex, dwWrite) ) {
        return FALSE;
    }

    return TRUE;
}

void CHttpUploadFiles::SetUserData(CString strUserData, void *lpUserData)
{
	m_strUserData = strUserData;
	m_lpUserData = lpUserData;
}

CString CHttpUploadFiles::GetUserData()
{
	return m_strUserData;
}

void CHttpUploadFiles::UserOperating(DWORD dwBufferSize, DWORD dwOnceBufSize)
{
	CManMakeWnd *lpWnd = (CManMakeWnd*)m_lpUserData;
	if (NULL == lpWnd)
	{
		return;
	}

	m_dwAlreadySend += dwOnceBufSize;

	//计算上传百分比
	CString strB;
	strB.Format(_T("正在上传PDF (%d%%)"), m_nTimes);
	lpWnd->m_lpprogress->SetText(strB.GetBuffer(0));
	m_nTimes ++;
}
