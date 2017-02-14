#pragma once

class CReg
{
public:
	CReg(void);
	~CReg(void);


	HKEY	m_hKey;
	DWORD	m_Index;

	//打开或创建
	BOOL	Open(HKEY hkRoot, CString pszSubKey, REGSAM sam = KEY_READ);
	BOOL	Create(HKEY hkRoot, CString pszSubKey, DWORD *dwDisposition = NULL);


	BOOL	GetValueSZ(CString szName, CString &szResult);
	BOOL	GetValueDW(CString szName, DWORD &dwResult);

	BOOL	SetValueSZ(CString szName, CString szValue);
	BOOL	SetValueDW(CString szName, DWORD dwValue);


	BOOL	DeleteKey(CString szName);
	BOOL	DeleteValue(CString szName);


	HKEY	GetHandle();
};
