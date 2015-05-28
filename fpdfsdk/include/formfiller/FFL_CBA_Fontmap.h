// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_CBA_FONTMAP_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_CBA_FONTMAP_H_

#include "../pdfwindow/PWL_FontMap.h"

class CPDF_Dictionary;
class CPDFSDK_Annot;

class CBA_FontMap : public CPWL_FontMap
{
public:
	CBA_FontMap(CPDFSDK_Annot* pAnnot, IFX_SystemHandler* pSystemHandler);
	CBA_FontMap(CPDF_Document* pDocument, CPDF_Dictionary* pAnnotDict, IFX_SystemHandler* pSystemHandler);

	virtual ~CBA_FontMap();

	virtual void				Initial(FX_LPCSTR fontname = NULL);

public:
	void						SetDefaultFont(CPDF_Font * pFont, const CFX_ByteString & sFontName);

	void						Reset();
	void						SetAPType(const CFX_ByteString& sAPType);

protected:
	virtual CPDF_Font*			FindFontSameCharset(CFX_ByteString& sFontAlias, FX_INT32 nCharset);
	virtual void				AddedFont(CPDF_Font* pFont, const CFX_ByteString& sFontAlias);
	virtual CPDF_Document*		GetDocument();
private:
	CPDF_Font*					FindResFontSameCharset(CPDF_Dictionary* pResDict, CFX_ByteString& sFontAlias,
									FX_INT32 nCharset);
	CPDF_Font*					GetAnnotDefaultFont(CFX_ByteString &csNameTag);
	void						AddFontToAnnotDict(CPDF_Font* pFont, const CFX_ByteString& sAlias);

private:
	CPDF_Document*				m_pDocument;
	CPDF_Dictionary*			m_pAnnotDict;
	CPDF_Font*					m_pDefaultFont;
	CFX_ByteString				m_sDefaultFontName;
	
	CFX_ByteString				m_sAPType;
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_CBA_FONTMAP_H_
