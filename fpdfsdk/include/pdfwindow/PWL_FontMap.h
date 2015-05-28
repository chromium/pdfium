// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_FONTMAP_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_FONTMAP_H_

#include "../../../public/fpdf_sysfontinfo.h"
#include "../fxedit/fx_edit.h"

class CPDF_Document;
class IFX_SystemHandler;

struct CPWL_FontMap_Data
{
	CPDF_Font*			pFont;
	FX_INT32			nCharset;
	CFX_ByteString		sFontName;
};

struct CPWL_FontMap_Native
{
	FX_INT32			nCharset;
	CFX_ByteString		sFontName;
};

#ifndef ANSI_CHARSET

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204
#define BALTIC_CHARSET          186

#endif

#ifndef PWL_CLASS

	#ifdef FX_READER_DLL
		#define PWL_CLASS		__declspec(dllexport)
	#else
		#define PWL_CLASS
	#endif
#endif

class PWL_CLASS CPWL_FontMap : public IFX_Edit_FontMap
{
public:
	CPWL_FontMap(IFX_SystemHandler* pSystemHandler);
	virtual ~CPWL_FontMap();

	virtual CPDF_Font*							GetPDFFont(FX_INT32 nFontIndex);
	virtual CFX_ByteString						GetPDFFontAlias(FX_INT32 nFontIndex);
	virtual FX_INT32							GetWordFontIndex(FX_WORD word, FX_INT32 nCharset, FX_INT32 nFontIndex);
	virtual FX_INT32							CharCodeFromUnicode(FX_INT32 nFontIndex, FX_WORD word);
	virtual FX_INT32							CharSetFromUnicode(FX_WORD word, FX_INT32 nOldCharset);

public:
	virtual void								Initial(FX_LPCSTR fontname = NULL);
	void										SetSystemHandler(IFX_SystemHandler* pSystemHandler);

	FX_INT32									GetFontMapCount() const;
	const CPWL_FontMap_Data*					GetFontMapData(FX_INT32 nIndex) const;

public:
	static FX_INT32								GetNativeCharset();
	CFX_ByteString								GetNativeFontName(FX_INT32 nCharset);

	static CFX_ByteString						GetDefaultFontByCharset(FX_INT32 nCharset);

	CPDF_Font*									AddFontToDocument(CPDF_Document* pDoc, CFX_ByteString& sFontName, FX_BYTE nCharset);
	static FX_BOOL								IsStandardFont(const CFX_ByteString& sFontName);							
	CPDF_Font*									AddStandardFont(CPDF_Document* pDoc, CFX_ByteString& sFontName);
	CPDF_Font*									AddSystemFont(CPDF_Document* pDoc, CFX_ByteString& sFontName, 
													FX_BYTE nCharset);

protected:
	virtual CPDF_Font*							FindFontSameCharset(CFX_ByteString& sFontAlias, FX_INT32 nCharset);
	virtual void								AddedFont(CPDF_Font* pFont, const CFX_ByteString& sFontAlias);
	FX_BOOL										KnowWord(FX_INT32 nFontIndex, FX_WORD word);

	virtual CPDF_Document*						GetDocument();

	void										Empty();
	FX_INT32									GetFontIndex(const CFX_ByteString& sFontName, FX_INT32 nCharset, FX_BOOL bFind);
	FX_INT32									GetPWLFontIndex(FX_WORD word, FX_INT32 nCharset);
	FX_INT32									AddFontData(CPDF_Font* pFont, const CFX_ByteString& sFontAlias, FX_INT32 nCharset = DEFAULT_CHARSET);

	CFX_ByteString								EncodeFontAlias(const CFX_ByteString& sFontName, FX_INT32 nCharset);
	CFX_ByteString								EncodeFontAlias(const CFX_ByteString& sFontName);

private:
	CFX_ByteString								GetFontName(FX_INT32 nFontIndex);
	FX_INT32									FindFont(const CFX_ByteString& sFontName, FX_INT32 nCharset = DEFAULT_CHARSET);

	CFX_ByteString								GetNativeFont(FX_INT32 nCharset);

public:
    using CharsetFontMap = FPDF_CharsetFontMap;
	static const CharsetFontMap					defaultTTFMap[];

protected:
	CFX_ArrayTemplate<CPWL_FontMap_Data*>		m_aData;
	CFX_ArrayTemplate<CPWL_FontMap_Native*>		m_aNativeFont;

private:
	CPDF_Document*								m_pPDFDoc;
	IFX_SystemHandler*							m_pSystemHandler;
};

class PWL_CLASS CPWL_DocFontMap : public CPWL_FontMap
{
public:
	CPWL_DocFontMap(IFX_SystemHandler* pSystemHandler, CPDF_Document* pAttachedDoc);
	virtual ~CPWL_DocFontMap();

	virtual CPDF_Document*						GetDocument();

private:
	CPDF_Document*								m_pAttachedDoc;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_FONTMAP_H_
