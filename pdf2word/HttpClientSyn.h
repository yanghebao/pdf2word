#pragma once
#include <string>
#include <WINDOWS.H>
#include <winhttp.h>
#include <vector>

typedef struct _StParam_{
    std::wstring wstrKey;
    std::wstring wstrValue;
}StParam,*PStParam;

typedef std::vector<StParam> VecStParam;
typedef VecStParam::iterator VecStParamIter;
typedef VecStParam::const_iterator VecStParamCIter;

enum EType{
    eGet,
    ePost,
    eUpload,
};

class CHttpClientSyn
{
public:
    CHttpClientSyn();
    ~CHttpClientSyn(void);

public:
    BOOL TransmiteData(const std::wstring& wstrUrl, EType eType, DWORD dwTimeout);

    VOID ClearEvn();

    BOOL ReceiveData(LPBYTE lpBuffer, DWORD& dwBufferSize);

	virtual void UserOperating(DWORD dwBufferSize, DWORD dwOnceBufSize);	//by wg

private:
    BOOL InitializeHttp( const std::wstring& wstrUrl, DWORD dwTimeout, EType eType);
    VOID UninitializeHttp();

    BOOL ReceiveData();

    BOOL TransmiteData(EType eType);

    VOID ParseParams(const std::wstring& wstrExtraInfo);

    BOOL TransmiteDataToServerByGet();
    BOOL TransmiteDataToServerByPost();
    BOOL TransmiteDataToServerByUpload();

private: 
    virtual DWORD GetDataSize() = 0;
    virtual BOOL GetData(LPVOID lpBuffer, DWORD dwBufferSize, DWORD& dwWrite);
    virtual BOOL ModifyRequestHeader(HINTERNET hRequest) = 0;
    virtual VOID AddExtInfo(VecStParam& VecExtInfo) = 0;
    virtual std::wstring GenerateExtInfo(const VecStParam& VecExtInfo) = 0;
private: 
    HINTERNET m_hSession;
    HINTERNET m_hConnect;
    HINTERNET m_hRequest;
    DWORD m_dwTimeout;
    LPBYTE m_lpReceiveData;
    DWORD m_dwReceiveDataLength;
    std::wstring m_wstrUrlPath;
    VecStParam m_VecExtInfo;
};

