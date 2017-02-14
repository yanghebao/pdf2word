//MyDiskInfo.hÍ·ÎÄ¼þ
#pragma once

class MyDiskInfo
{
public:
	MyDiskInfo(void);
public:
	~MyDiskInfo(void);
public: 
	int GetDiskInfo(int driver=0); 
	TCHAR m_szModelNumber[64]; 
	TCHAR m_szSerialNumber[64]; 

	CString GetDiskID();		// add by wg at 20161228
};
