#pragma once
#include "HttpClientSyn.h"

class CHttpTransByPost:public CHttpClientSyn
{
public:
    CHttpTransByPost(void);
    ~CHttpTransByPost(void);
public:
	BOOL TransDataToServer( const std::wstring& wstrUrl, DWORD dwTimeout,   
		VecStParam& vecParam, LPVOID lpData, DWORD dwDataLenInBytes );  
private:
    DWORD GetDataSize();
    BOOL GetData(LPVOID lpBuffer, DWORD dwBufferSize, DWORD& dwWrite);
    std::wstring GenerateExtInfo(const VecStParam& VecExtInfo);
    BOOL ModifyRequestHeader(HINTERNET hRequest);
    VOID AddExtInfo(VecStParam& VecExtInfo);
private:
    LPVOID m_lpData;
    DWORD m_dwDataSize;
    DWORD m_dwWriteIndex;
    VecStParam m_vecParam;
};

