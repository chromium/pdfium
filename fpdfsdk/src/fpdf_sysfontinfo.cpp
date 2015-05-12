// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../public/fpdf_sysfontinfo.h"
#include "../include/fsdk_define.h"
#include "../include/pdfwindow/PWL_FontMap.h"

class CSysFontInfo_Ext FX_FINAL : public IFX_SystemFontInfo
{
public:
	FPDF_SYSFONTINFO*	m_pInfo;

	virtual void		Release() FX_OVERRIDE
	{
		if (m_pInfo->Release)
			m_pInfo->Release(m_pInfo);
		delete this;
	}

	virtual	FX_BOOL		EnumFontList(CFX_FontMapper* pMapper) FX_OVERRIDE
	{
		if (m_pInfo->EnumFonts) {
			m_pInfo->EnumFonts(m_pInfo, pMapper);
			return TRUE;
		}
		return FALSE;
	}

	virtual void*		MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR family, FX_BOOL& bExact)  FX_OVERRIDE
	{
		if (m_pInfo->MapFont)
			return m_pInfo->MapFont(m_pInfo, weight, bItalic, charset, pitch_family, family, &bExact);
		return NULL;
	}

	virtual void*		GetFont(FX_LPCSTR family)  FX_OVERRIDE
	{
		if (m_pInfo->GetFont)
			return m_pInfo->GetFont(m_pInfo, family);
		return NULL;
	}

	virtual FX_DWORD	GetFontData(void* hFont, FX_DWORD table, FX_LPBYTE buffer, FX_DWORD size)  FX_OVERRIDE
	{
		if (m_pInfo->GetFontData)
			return m_pInfo->GetFontData(m_pInfo, hFont, table, buffer, size);
		return 0;
	}

	virtual FX_BOOL		GetFaceName(void* hFont, CFX_ByteString& name)  FX_OVERRIDE
	{
		if (m_pInfo->GetFaceName == NULL) return FALSE;
		FX_DWORD size = m_pInfo->GetFaceName(m_pInfo, hFont, NULL, 0);
		if (size == 0) return FALSE;
		char* buffer = FX_Alloc(char, size);
		size = m_pInfo->GetFaceName(m_pInfo, hFont, buffer, size);
		name = CFX_ByteString(buffer, size);
		FX_Free(buffer);
		return TRUE;
	}

	virtual FX_BOOL		GetFontCharset(void* hFont, int& charset)  FX_OVERRIDE
	{
		if (m_pInfo->GetFontCharset) {
			charset = m_pInfo->GetFontCharset(m_pInfo, hFont);
			return TRUE;
		}
		return FALSE;
	}

	virtual void		DeleteFont(void* hFont)  FX_OVERRIDE
	{
		if (m_pInfo->DeleteFont)
			m_pInfo->DeleteFont(m_pInfo, hFont);
	}

private:
        ~CSysFontInfo_Ext() { }
};

DLLEXPORT void STDCALL FPDF_AddInstalledFont(void* mapper, const char* name, int charset)
{
	((CFX_FontMapper*)mapper)->AddInstalledFont(name, charset);
}

DLLEXPORT void STDCALL FPDF_SetSystemFontInfo(FPDF_SYSFONTINFO* pFontInfoExt)
{
	if (pFontInfoExt->version != 1) return;

	CSysFontInfo_Ext* pFontInfo = new CSysFontInfo_Ext;
	pFontInfo->m_pInfo = pFontInfoExt;
	CFX_GEModule::Get()->GetFontMgr()->SetSystemFontInfo(pFontInfo);
}

DLLEXPORT const FPDF_CharsetFontMap* STDCALL FPDF_GetDefaultTTFMap()
{
    return CPWL_FontMap::defaultTTFMap;
}

struct FPDF_SYSFONTINFO_DEFAULT : public FPDF_SYSFONTINFO
{
	IFX_SystemFontInfo*	m_pFontInfo;
};

static void DefaultRelease(struct _FPDF_SYSFONTINFO* pThis)
{
	((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->Release();
}

static void DefaultEnumFonts(struct _FPDF_SYSFONTINFO* pThis, void* pMapper)
{
	((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->EnumFontList((CFX_FontMapper*)pMapper);
}

static void* DefaultMapFont(struct _FPDF_SYSFONTINFO* pThis, int weight, int bItalic, int charset, int pitch_family, const char* family, int* bExact)
{
	return ((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->MapFont(weight, bItalic, charset, pitch_family, family, *bExact);
}

void* DefaultGetFont(struct _FPDF_SYSFONTINFO* pThis, const char* family)
{
	return ((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->GetFont(family);
}

static unsigned long DefaultGetFontData(struct _FPDF_SYSFONTINFO* pThis, void* hFont,
			unsigned int table, unsigned char* buffer, unsigned long buf_size)
{
	return ((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->GetFontData(hFont, table, buffer, buf_size);
}

static unsigned long DefaultGetFaceName(struct _FPDF_SYSFONTINFO* pThis, void* hFont, char* buffer, unsigned long buf_size)
{
	CFX_ByteString name;
	if (!((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->GetFaceName(hFont, name)) return 0;
	if (name.GetLength() >= (long)buf_size) return name.GetLength() + 1;
	FXSYS_strcpy(buffer, name);
	return name.GetLength() + 1;
}

static int DefaultGetFontCharset(struct _FPDF_SYSFONTINFO* pThis, void* hFont)
{
	int charset;
	if (!((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->GetFontCharset(hFont, charset)) return 0;
	return charset;
}

static void DefaultDeleteFont(struct _FPDF_SYSFONTINFO* pThis, void* hFont)
{
	((FPDF_SYSFONTINFO_DEFAULT*)pThis)->m_pFontInfo->DeleteFont(hFont);
}

DLLEXPORT FPDF_SYSFONTINFO* STDCALL FPDF_GetDefaultSystemFontInfo()
{
	IFX_SystemFontInfo* pFontInfo = IFX_SystemFontInfo::CreateDefault();
	if (pFontInfo == NULL) return NULL;

	FPDF_SYSFONTINFO_DEFAULT* pFontInfoExt = FX_Alloc(FPDF_SYSFONTINFO_DEFAULT, 1);
	pFontInfoExt->DeleteFont = DefaultDeleteFont;
	pFontInfoExt->EnumFonts = DefaultEnumFonts;
	pFontInfoExt->GetFaceName = DefaultGetFaceName;
	pFontInfoExt->GetFont = DefaultGetFont;
	pFontInfoExt->GetFontCharset = DefaultGetFontCharset;
	pFontInfoExt->GetFontData = DefaultGetFontData;
	pFontInfoExt->MapFont = DefaultMapFont;
	pFontInfoExt->Release = DefaultRelease;
	pFontInfoExt->version = 1;
	pFontInfoExt->m_pFontInfo = pFontInfo;
	return pFontInfoExt;
}
