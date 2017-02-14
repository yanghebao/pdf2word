#include "StdAfx.h"
#include "Reg.h"

CReg::CReg(void)
{
	m_hKey = NULL;
	m_Index = 0;
}

CReg::~CReg(void)
{
	if(m_hKey != NULL)
	{
		RegCloseKey(m_hKey);
	}
}


BOOL CReg::Open(HKEY hkRoot, CString pszSubKey, REGSAM sam)
{
	return ERROR_SUCCESS == RegOpenKeyEx(hkRoot, pszSubKey, 0, sam, &m_hKey);
}


BOOL CReg::Create(HKEY hkRoot, CString pszSubKey, DWORD *dwDisposition)
{
	return ERROR_SUCCESS == RegCreateKeyEx(hkRoot, pszSubKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &m_hKey, dwDisposition);
}

BOOL CReg::GetValueSZ(CString szName, CString &szResult)
{
	DWORD dwSizeNeeded = 0;
	if (ERROR_SUCCESS == RegQueryValueEx(m_hKey, szName, NULL, NULL, NULL, &dwSizeNeeded))
	{
		if (ERROR_SUCCESS == RegQueryValueEx(m_hKey, szName, NULL, NULL, (LPBYTE)szResult.GetBuffer(dwSizeNeeded + sizeof(TCHAR)),
			&dwSizeNeeded))
		{
			szResult.ReleaseBuffer();
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}


BOOL CReg::SetValueSZ(CString szName, CString szValue)
{
	DWORD dwLen = (szValue.GetLength() + 1) * sizeof(TCHAR);
	return ERROR_SUCCESS == RegSetValueEx(m_hKey, szName, 0, REG_SZ, (LPBYTE)(LPCTSTR)szValue, dwLen);
}


HKEY CReg::GetHandle()
{
	return m_hKey;
}


BOOL CReg::GetValueDW(CString szName, DWORD &dwResult)
{
	DWORD dwSizeNeeded = sizeof(DWORD);
	if (ERROR_SUCCESS == RegQueryValueEx(m_hKey, szName, NULL, NULL, (LPBYTE)&dwResult, &dwSizeNeeded))
	{
		return TRUE;
	}
	else
	{
		dwResult = 0;
		return FALSE;
	}
}

BOOL CReg::SetValueDW(CString szName, DWORD dwValue)
{
	return ERROR_SUCCESS == RegSetValueEx(m_hKey, szName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
}


BOOL CReg::DeleteValue(CString szName)
{
	return ERROR_SUCCESS == RegDeleteValue(m_hKey, szName);
}

BOOL CReg::DeleteKey(CString szName)
{
	return ERROR_SUCCESS == RegDeleteKey(m_hKey, szName);
}
