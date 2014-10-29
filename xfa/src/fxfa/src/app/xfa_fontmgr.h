// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_FONTMGR_IMP_H
#define _FXFA_FORMFILLER_FONTMGR_IMP_H
struct XFA_FONTINFO {
    FX_DWORD	dwFontNameHash;
    FX_LPCWSTR	pPsName;
    FX_LPCWSTR	pReplaceFont;
    FX_WORD		dwStyles;
    FX_WORD		wCodePage;
};
class CXFA_DefFontMgr : public IXFA_FontMgr, public CFX_Object
{
public:
    CXFA_DefFontMgr() {}
    virtual ~CXFA_DefFontMgr();
    virtual	void			Release()
    {
        delete this;
    }
    virtual IFX_Font*		GetFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage = 0xFFFF);
    virtual IFX_Font*		GetDefaultFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage = 0xFFFF);
protected:
    CFX_PtrArray			m_CacheFonts;
};
class CXFA_PDFFontMgr : public IFX_FontProvider, public CFX_Object
{
public:
    CXFA_PDFFontMgr(CXFA_FFDoc* pDoc);
    ~CXFA_PDFFontMgr();
    IFX_Font*				GetFont(FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, CPDF_Font** pPDFFont, FX_BOOL bStrictMatch = TRUE);
    FX_BOOL					GetCharWidth(IFX_Font* pFont, FX_WCHAR wUnicode, FX_INT32 &iWidth, FX_BOOL bCharCode);
    CFX_MapPtrToPtr			m_FDE2PDFFont;
protected:
    IFX_Font*				FindFont(CFX_ByteString strFamilyName, FX_BOOL bBold, FX_BOOL bItalic, CPDF_Font** pPDFFont, FX_BOOL bStrictMatch = TRUE);
    CFX_ByteString			PsNameToFontName(const CFX_ByteString& strPsName, FX_BOOL bBold, FX_BOOL bItalic);
    FX_BOOL					PsNameMatchDRFontName(FX_BSTR bsPsName, FX_BOOL bBold, FX_BOOL bItalic, const CFX_ByteString& bsDRFontName, FX_BOOL bStrictMatch = TRUE);
    CXFA_FFDoc*								m_pDoc;
    CFX_CMapByteStringToPtr					m_FontArray;
};
class CXFA_FontMgr : public CFX_Object
{
public:
    CXFA_FontMgr();
    ~CXFA_FontMgr();
    IFX_Font*		GetFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles,
                            FX_WORD wCodePage = 0xFFFF);
    void			LoadDocFonts(XFA_HDOC hDoc);
    void			ReleaseDocFonts(XFA_HDOC hDoc);

    void			SetDefFontMgr(IXFA_FontMgr* pFontMgr);
protected:
    void					DelAllMgrMap();
    CFX_MapPtrToPtr			m_PDFFontMgrArray;
    IXFA_FontMgr*			m_pDefFontMgr;
    CFX_CMapByteStringToPtr	m_FontArray;
};
#endif
