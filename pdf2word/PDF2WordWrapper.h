// PDF2WordWrapper.h: interface for the CPDF2WordWrapper class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


// #pragma warning(disable : 4290)
#import "progid:EasyConverter.PDF2Word.5" named_guids
// #pragma warning (default : 4192 4146)

class CPDF2WordWrapper;
const int ID_PDF2WORD = 1;
const int VERSION_MAJOR = 5;
const int VERSION_MINOR = 0;

extern _ATL_FUNC_INFO info1;
extern _ATL_FUNC_INFO info2;

typedef IDispEventImpl<ID_PDF2WORD, 
					CPDF2WordWrapper, 
					&__uuidof(EasyConverterLib::_IPDF2WordEvents), 
					&EasyConverterLib::LIBID_EasyConverterLib, 
					VERSION_MAJOR, 
					VERSION_MINOR> MyPDF2WordEvents;


class CPDF2WordWrapper : public MyPDF2WordEvents
{
public:
	// constructor/destructor
	CPDF2WordWrapper();
	virtual ~CPDF2WordWrapper();

	// set IPrintJobMonitor
	void SetPDF2Word(EasyConverterLib::IPDF2WordPtr pPDF2Word);

public:
	// pure virtual functions
	virtual EasyConverterLib::cnvResponse OnPageStart(long nPageNumber, long nPageCount, BSTR bstrFileName) = 0;
	virtual void OnConversionFinished(long nResult) = 0;

	//////////////////////////////////////////////
	// Sink interface declaration

	BEGIN_SINK_MAP(CPDF2WordWrapper)
		SINK_ENTRY_INFO(ID_PDF2WORD, __uuidof(EasyConverterLib::_IPDF2WordEvents), 1, __OnPageStart, &info1)
		SINK_ENTRY_INFO(ID_PDF2WORD, __uuidof(EasyConverterLib::_IPDF2WordEvents), 2, __OnConversionFinished, &info2)
	END_SINK_MAP()

	//////////////////////////////////////////////
	// COM event hookup
public:
	STDMETHOD_(EasyConverterLib::cnvResponse, __OnPageStart)(long nPageNumber, long nPageCount, BSTR bstrFileName);
	STDMETHOD_(void, __OnConversionFinished)(long nResult);

private:
	void _Release();

private:
	EasyConverterLib::IPDF2WordPtr m_pPDF2Word;
};

