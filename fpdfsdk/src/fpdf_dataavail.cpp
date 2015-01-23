// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fpdf_dataavail.h"

extern void ProcessParseError(FX_DWORD err_code);
class CFPDF_FileAvailWrap : public IFX_FileAvail
{
public:
	CFPDF_FileAvailWrap()
	{
		m_pfileAvail = NULL;
	}

	void Set(FX_FILEAVAIL* pfileAvail)
	{
		m_pfileAvail = pfileAvail;
	}

	virtual FX_BOOL			IsDataAvail( FX_FILESIZE offset, FX_DWORD size)
	{
		return m_pfileAvail->IsDataAvail(m_pfileAvail, offset, size);
	}

private:
	FX_FILEAVAIL* m_pfileAvail;
};  

class CFPDF_FileAccessWrap : public IFX_FileRead
{
public:
	CFPDF_FileAccessWrap()
	{
		m_pFileAccess = NULL;
	}

	void Set(FPDF_FILEACCESS* pFile)
	{
		m_pFileAccess = pFile;
	}

	virtual FX_FILESIZE		GetSize()
	{
		return m_pFileAccess->m_FileLen; 
	}

	virtual FX_BOOL			ReadBlock(void* buffer, FX_FILESIZE offset, size_t size)
	{
		return m_pFileAccess->m_GetBlock(m_pFileAccess->m_Param, offset, (FX_LPBYTE)buffer, size);
	}

	virtual void			Release()
	{
	}

private:
	FPDF_FILEACCESS*		m_pFileAccess;
};

class CFPDF_DownloadHintsWrap : public IFX_DownloadHints
{
public:
	CFPDF_DownloadHintsWrap(FX_DOWNLOADHINTS* pDownloadHints)
	{
		m_pDownloadHints = pDownloadHints;
	}
public:
	virtual void			AddSegment(FX_FILESIZE offset, FX_DWORD size) 
	{
		m_pDownloadHints->AddSegment(m_pDownloadHints, offset, size);
	}	
private:
	FX_DOWNLOADHINTS* m_pDownloadHints;
};

class CFPDF_DataAvail : public CFX_Object
{
public:
	CFPDF_DataAvail()
	{
		m_pDataAvail = NULL;
	}

	~CFPDF_DataAvail()
	{
		if (m_pDataAvail) delete m_pDataAvail;
	}

	IPDF_DataAvail*			m_pDataAvail;
	CFPDF_FileAvailWrap		m_FileAvail;
	CFPDF_FileAccessWrap	m_FileRead;
};

DLLEXPORT FPDF_AVAIL STDCALL FPDFAvail_Create(FX_FILEAVAIL* file_avail, FPDF_FILEACCESS* file)
{
	CFPDF_DataAvail* pAvail = FX_NEW CFPDF_DataAvail;
	pAvail->m_FileAvail.Set(file_avail);
	pAvail->m_FileRead.Set(file);
	pAvail->m_pDataAvail = IPDF_DataAvail::Create(&pAvail->m_FileAvail, &pAvail->m_FileRead);
	return pAvail;
}

DLLEXPORT void STDCALL FPDFAvail_Destroy(FPDF_AVAIL avail)
{
	if (avail == NULL) return;
	delete (CFPDF_DataAvail*)avail;
}

DLLEXPORT int STDCALL FPDFAvail_IsDocAvail(FPDF_AVAIL avail, FX_DOWNLOADHINTS* hints)
{
	if (avail == NULL || hints == NULL) return 0;
	CFPDF_DownloadHintsWrap hints_wrap(hints);
	return ((CFPDF_DataAvail*)avail)->m_pDataAvail->IsDocAvail(&hints_wrap);
}

extern void CheckUnSupportError(CPDF_Document * pDoc, FX_DWORD err_code);

DLLEXPORT FPDF_DOCUMENT STDCALL FPDFAvail_GetDocument(FPDF_AVAIL avail,	FPDF_BYTESTRING password)
{
	if (avail == NULL) return NULL;
	CPDF_Parser* pParser = FX_NEW CPDF_Parser;
	pParser->SetPassword(password);

	FX_DWORD err_code = pParser->StartAsynParse(((CFPDF_DataAvail*)avail)->m_pDataAvail->GetFileRead());
	if (err_code) {
		delete pParser;
		ProcessParseError(err_code);
		return NULL;
	}
	((CFPDF_DataAvail*)avail)->m_pDataAvail->SetDocument(pParser->GetDocument());
	CheckUnSupportError(pParser->GetDocument(), FPDF_ERR_SUCCESS);
	return pParser->GetDocument();
}

DLLEXPORT int STDCALL FPDFAvail_GetFirstPageNum(FPDF_DOCUMENT doc)
{
	if (doc == NULL) return 0;
	CPDF_Document* pDoc = (CPDF_Document*)doc;
	return ((CPDF_Parser*)pDoc->GetParser())->GetFirstPageNo();
}

DLLEXPORT int STDCALL FPDFAvail_IsPageAvail(FPDF_AVAIL avail, int page_index, FX_DOWNLOADHINTS* hints)
{
	if (avail == NULL || hints == NULL) return 0;
	CFPDF_DownloadHintsWrap hints_wrap(hints);
	return ((CFPDF_DataAvail*)avail)->m_pDataAvail->IsPageAvail(page_index, &hints_wrap);
}

DLLEXPORT int STDCALL FPDFAvail_IsFormAvail(FPDF_AVAIL avail, FX_DOWNLOADHINTS* hints)
{
	if (avail == NULL || hints == NULL) return -1;
	CFPDF_DownloadHintsWrap hints_wrap(hints);
	return ((CFPDF_DataAvail*)avail)->m_pDataAvail->IsFormAvail(&hints_wrap);
}

DLLEXPORT FPDF_BOOL STDCALL FPDFAvail_IsLinearized(FPDF_AVAIL avail)
{
		if (avail == NULL) return -1;
	return ((CFPDF_DataAvail*)avail)->m_pDataAvail->IsLinearizedPDF();

}
