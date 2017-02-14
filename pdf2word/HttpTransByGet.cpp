#include "stdafx.h"
#include "HttpTransByGet.h"


CHttpTransByGet::CHttpTransByGet(void)
{
    m_lpData = NULL;
    m_dwDataSize = 0;
    m_dwWriteIndex = 0;
}


CHttpTransByGet::~CHttpTransByGet(void)
{
}

DWORD CHttpTransByGet::GetDataSize()
{
    return m_dwDataSize;
}

BOOL CHttpTransByGet::GetData( LPVOID lpBuffer, DWORD dwBufferSize, DWORD& dwWrite )
{
    BOOL bContinue = TRUE;
    dwWrite = 0;
    if ( m_dwDataSize > m_dwWriteIndex + dwBufferSize ) {
        dwWrite = dwBufferSize;
    }
    else {
        dwWrite = m_dwDataSize - m_dwWriteIndex;
        bContinue = FALSE;
    }

    if ( 0 != memcpy_s(lpBuffer, dwBufferSize, (LPBYTE)m_lpData + m_dwWriteIndex, dwWrite) ){
        bContinue = FALSE;
    }

    return bContinue;
}

BOOL CHttpTransByGet::TransDataToServer( const std::wstring& wstrUrl, DWORD dwTimeout, 
    VecStParam& vecParam, LPVOID lpData, DWORD &dwDataLenInBytes )
{
    m_lpData = lpData;
    m_dwDataSize = dwDataLenInBytes;
    m_vecParam.assign(vecParam.begin(), vecParam.end());
    m_dwWriteIndex = 0;
    BOOL bRet = TransmiteData(wstrUrl, eGet, dwTimeout);
	if (bRet)
	{
		ReceiveData((LPBYTE)m_lpData, m_dwDataSize);
	}

	return bRet;
}

std::wstring CHttpTransByGet::GenerateExtInfo( const VecStParam& VecExtInfo )
{
    std::wstring wstrExtInf;
    for ( VecStParamCIter it = VecExtInfo.begin(); it != VecExtInfo.end(); it++ ) {
        if ( false == wstrExtInf.empty() ) {
            wstrExtInf += L"&";
        }
        wstrExtInf += it->wstrKey;
        wstrExtInf += L"=";
        wstrExtInf += it->wstrValue;
    }
    return wstrExtInf;
}

BOOL CHttpTransByGet::ModifyRequestHeader( HINTERNET hRequest )
{
    std::wstring wstrHeader[] = { L"Content-type: application/x-www-form-urlencoded\r\n"};
    for ( size_t i = 0; i < ARRAYSIZE(wstrHeader); i++ ) {
        WinHttpAddRequestHeaders(hRequest, wstrHeader[i].c_str(), wstrHeader[i].length(), WINHTTP_ADDREQ_FLAG_ADD);
    }
    return TRUE;
}

VOID CHttpTransByGet::AddExtInfo( VecStParam& VecExtInfo )
{
    for ( VecStParamCIter it = m_vecParam.begin(); it != m_vecParam.end(); it++ ) {
        VecExtInfo.push_back(*it);
    }
}
