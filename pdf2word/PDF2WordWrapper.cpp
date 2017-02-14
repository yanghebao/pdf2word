// PDF2WordWrapper.cpp: implementation of the CPDF2WordWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "sample.h"


#include "PDF2WordWrapper.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

_ATL_FUNC_INFO info1 = {CC_STDCALL, VT_I4, 3, {VT_I4, VT_I4, VT_BSTR} };
_ATL_FUNC_INFO info2 = {CC_STDCALL, VT_EMPTY, 1, {VT_I4} };

//////////////////////////////////////////////////////////////////////

CPDF2WordWrapper::CPDF2WordWrapper()
: MyPDF2WordEvents()
{
}

//////////////////////////////////////////////////////////////////////

CPDF2WordWrapper::~CPDF2WordWrapper()
{
	_Release();
}

//////////////////////////////////////////////////////////////////////

void CPDF2WordWrapper::SetPDF2Word(EasyConverterLib::IPDF2WordPtr pPDF2Word)
{
	HRESULT hr = NULL;

	_Release();

	if(pPDF2Word != NULL)
	{
		hr = MyPDF2WordEvents::DispEventAdvise(pPDF2Word, &__uuidof(EasyConverterLib::_IPDF2WordEvents));
		if(FAILED(hr))
		{
			_com_raise_error(hr);
		}

		m_pPDF2Word = pPDF2Word;
	}
}

//////////////////////////////////////////////////////////////////////

void CPDF2WordWrapper::_Release()
{
	if(m_pPDF2Word != NULL)
	{
		MyPDF2WordEvents::DispEventUnadvise(m_pPDF2Word);
		m_pPDF2Word = NULL;
	}
}

//////////////////////////////////////////////////////////////////////

EasyConverterLib::cnvResponse __stdcall CPDF2WordWrapper::__OnPageStart(long nPageNumber, long nPageCount, BSTR bstrFileName)
{	 
	return OnPageStart(nPageNumber, nPageCount, bstrFileName);
}
//////////////////////////////////////////////////////////////////////

void __stdcall CPDF2WordWrapper::__OnConversionFinished(long nResult)
{
	OnConversionFinished(nResult);
}

//////////////////////////////////////////////////////////////////////
