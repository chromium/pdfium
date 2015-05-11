// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../public/fpdf_edit.h"
#include "../../public/fpdf_save.h"
#include "../include/fsdk_define.h"

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

class CFX_IFileWrite FX_FINAL : public IFX_StreamWrite
{
	
public:
	CFX_IFileWrite();
	FX_BOOL				Init( FPDF_FILEWRITE * pFileWriteStruct );
	virtual	FX_BOOL		WriteBlock(const void* pData, size_t size) FX_OVERRIDE;
	virtual void		Release() FX_OVERRIDE {}
	
protected:
	FPDF_FILEWRITE*		m_pFileWriteStruct;
};

CFX_IFileWrite::CFX_IFileWrite()
{
	m_pFileWriteStruct = NULL;
}

FX_BOOL CFX_IFileWrite::Init( FPDF_FILEWRITE * pFileWriteStruct )
{
	if (!pFileWriteStruct)
		return FALSE;
	else
	{
		m_pFileWriteStruct = pFileWriteStruct;
	}
	return TRUE;
}

FX_BOOL CFX_IFileWrite::WriteBlock(const void* pData, size_t size)
{
	if (m_pFileWriteStruct)
	{
		m_pFileWriteStruct->WriteBlock( m_pFileWriteStruct, pData, size );
		return TRUE;
	}
	else 
		return FALSE;
}

FPDF_BOOL _FPDF_Doc_Save(FPDF_DOCUMENT document,FPDF_FILEWRITE * pFileWrite,FPDF_DWORD flags, FPDF_BOOL bSetVersion,
						 int fileVerion)
{
	CPDF_Document* pDoc = (CPDF_Document*)document;
	if (!pDoc) 
		return 0;
	
	if ( flags < FPDF_INCREMENTAL || flags > FPDF_REMOVE_SECURITY )
	{
		flags = 0;
	}
	
	CPDF_Creator FileMaker(pDoc);
	if(bSetVersion)
		FileMaker.SetFileVersion(fileVerion);
	if(flags == FPDF_REMOVE_SECURITY)
	{
		flags =  0;
		FileMaker.RemoveSecurity();
	}
	CFX_IFileWrite* pStreamWrite = NULL;
	FX_BOOL bRet;
	pStreamWrite = new CFX_IFileWrite;
	pStreamWrite->Init( pFileWrite );
	bRet = FileMaker.Create(pStreamWrite, flags);
	delete pStreamWrite;
	return bRet;
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_SaveAsCopy(	FPDF_DOCUMENT document,FPDF_FILEWRITE * pFileWrite,
												FPDF_DWORD flags )
{
	return _FPDF_Doc_Save(document, pFileWrite, flags, FALSE , 0);
}


DLLEXPORT FPDF_BOOL STDCALL FPDF_SaveWithVersion(	FPDF_DOCUMENT document,FPDF_FILEWRITE * pFileWrite,
	FPDF_DWORD flags, int fileVersion)
{
	return _FPDF_Doc_Save(document, pFileWrite, flags, TRUE , fileVersion);
}
