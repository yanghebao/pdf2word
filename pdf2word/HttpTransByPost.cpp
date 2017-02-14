#include "stdafx.h"
#include "HttpTransByPost.h"


CHttpTransByPost::CHttpTransByPost(void)
{
    m_lpData = NULL;
    m_dwDataSize = 0;
    m_dwWriteIndex = 0;
}


CHttpTransByPost::~CHttpTransByPost(void)
{
}

DWORD CHttpTransByPost::GetDataSize()  
{  
	return m_dwDataSize;  
}  

BOOL CHttpTransByPost::GetData( LPVOID lpBuffer, DWORD dwBufferSize, DWORD& dwWrite )  
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

BOOL CHttpTransByPost::TransDataToServer( const std::wstring& wstrUrl, DWORD dwTimeout,   
										 VecStParam& vecParam, LPVOID lpData, DWORD dwDataLenInBytes )  
{  
	m_lpData = lpData;  
	m_dwDataSize = dwDataLenInBytes;  
	m_vecParam.assign(vecParam.begin(), vecParam.end());  
	m_dwWriteIndex = 0;  
	return TransmiteData(wstrUrl, ePost, dwTimeout);  
} 

std::wstring CHttpTransByPost::GenerateExtInfo( const VecStParam& VecExtInfo )
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

BOOL CHttpTransByPost::ModifyRequestHeader( HINTERNET hRequest )
{
    std::wstring wstrHeader[] = { L"Content-type: application/x-www-form-urlencoded\r\n"};
    for ( size_t i = 0; i < ARRAYSIZE(wstrHeader); i++ ) {
        WinHttpAddRequestHeaders(hRequest, wstrHeader[i].c_str(), wstrHeader[i].length(), WINHTTP_ADDREQ_FLAG_ADD);
    }
    return TRUE;
}

VOID CHttpTransByPost::AddExtInfo( VecStParam& VecExtInfo )
{
    for ( VecStParamCIter it = m_vecParam.begin(); it != m_vecParam.end(); it++ ) {
        VecExtInfo.push_back(*it);
    }
}
