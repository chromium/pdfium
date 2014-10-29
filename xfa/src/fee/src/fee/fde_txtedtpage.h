// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTPAGE_H
#define _FDE_TXTEDTPAGE_H
class CFDE_TxtEdtEngine;
class IFX_CharIter;
class CFDE_TxtEdtPage;
class CFDE_TxtEdtLineExt;
class CFDE_TxtEdtTextSet;
class CFDE_TxtEdtParag;
enum FDE_TXTEDT_CHARTYPE {
    FDE_TXTEDT_CHARTYPE_Unknown	= 0	,
    FDE_TXTEDT_CHARTYPE_Tab			,
    FDE_TXTEDT_CHARTYPE_Space		,
    FDE_TXTEDT_CHARTYPE_Punctuation	,
    FDE_TXTEDT_CHARTYPE_LineBreak	,
    FDE_TXTEDT_CHARTYPE_Number		,
    FDE_TXTEDT_CHARTYPE_Char		,
    FDE_TXTEDT_CHARTYPE_CJK			,
};
static FDE_TXTEDT_CHARTYPE FDE_GetEditSelCharType(FX_WCHAR wChar)
{
    if (wChar == 0x9) {
        return FDE_TXTEDT_CHARTYPE_Tab;
    } else if (wChar == 0x20 || wChar == 0xA0) {
        return FDE_TXTEDT_CHARTYPE_Space;
    } else if (wChar == 0x9 || wChar == 0x20 || wChar == 0xA0 || (wChar >= L'!' && wChar <= L'/') || (wChar >= L':' && wChar <= L'@') || (wChar >= L'[' && wChar <= L'^') ||
               (wChar >= L'{' && wChar <= L'~') || wChar == 0x60) {
        return FDE_TXTEDT_CHARTYPE_Punctuation;
    } else if (wChar == 0x0a || wChar == 0x0d) {
        return FDE_TXTEDT_CHARTYPE_LineBreak;
    } else if (wChar >= '0' && wChar <= '9') {
        return FDE_TXTEDT_CHARTYPE_Number;
    } else if ((wChar >= 0x2e80 && wChar <= 0x2eff) || (wChar >= 0x3000 && wChar <= 0x303f) || (wChar >= 0x31c0 && wChar <= 0x31ef)
               || (wChar >= 0x3200 && wChar <= 0x32ff) || (wChar >= 0x3300 && wChar <= 0x33ff) || (wChar >= 0x3400 && wChar <= 0x4dbf)
               || (wChar >= 0x4e00 && wChar <= 0x9fff) || (wChar >= 0xf900 && wChar <= 0xfaff) || (wChar >= 0xfe30 && wChar <= 0xfe4f)) {
        return FDE_TXTEDT_CHARTYPE_CJK;
    } else {
        return FDE_TXTEDT_CHARTYPE_Char;
    }
}
typedef struct _FDE_TXTEDTPIECE : public CFX_Object {
    FX_INT32	nStart;
    FX_INT32	nCount;
    FX_INT32	nBidiLevel;
    CFX_RectF	rtPiece;
    FX_DWORD	dwCharStyles;
} FDE_TEXTEDITPIECE, * FDE_LPTEXTEDITPIECE;
typedef CFX_MassArrayTemplate<FDE_TEXTEDITPIECE> CFDE_TXTEDTPieceMassArray;
class CFDE_TxtEdtTextSet : public IFDE_TextSet, public CFX_Object
{
public:
    CFDE_TxtEdtTextSet(CFDE_TxtEdtPage * pPage);
    ~CFDE_TxtEdtTextSet();

    virtual FDE_VISUALOBJTYPE	GetType();
    virtual FX_BOOL		GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF &bbox);
    virtual FX_BOOL		GetMatrix(FDE_HVISUALOBJ hVisualObj, CFX_Matrix &matrix);
    virtual FX_BOOL		GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF &rt);
    virtual FX_BOOL		GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF &rt);
    virtual FX_INT32	GetString(FDE_HVISUALOBJ hText, CFX_WideString &wsText);
    virtual IFX_Font*	GetFont(FDE_HVISUALOBJ hText);
    virtual FX_FLOAT	GetFontSize(FDE_HVISUALOBJ hText);
    virtual FX_ARGB		GetFontColor(FDE_HVISUALOBJ hText);
    virtual FX_INT32	GetDisplayPos(FDE_HVISUALOBJ hText, FXTEXT_CHARPOS *pCharPos, FX_BOOL bCharCode = FALSE, CFX_WideString *pWSForms = NULL);
    virtual FX_INT32	GetCharRects(FDE_HVISUALOBJ hText, CFX_RectFArray &rtArray);
    virtual FX_INT32	GetCharRects_Impl(FDE_HVISUALOBJ hText, CFX_RectFArray &rtArray, FX_BOOL bBBox = FALSE);
private:

    CFDE_TxtEdtPage * m_pPage;
};
class CFDE_TxtEdtPage : public CFX_Object, public IFDE_TxtEdtPage
{
public:
    CFDE_TxtEdtPage(IFDE_TxtEdtEngine * pEngine, FX_INT32 nLineIndex);

    virtual void				Release();
    virtual IFDE_TxtEdtEngine * GetEngine() const;

    virtual FDE_VISUALOBJTYPE	GetType();
    virtual FX_BOOL		GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF &bbox);
    virtual FX_BOOL		GetMatrix(FDE_HVISUALOBJ hVisualObj, CFX_Matrix &matrix);
    virtual FX_BOOL		GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF &rt);
    virtual FX_BOOL		GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF &rt);
    virtual FX_INT32	GetCharRect(FX_INT32 nIndex, CFX_RectF &rect, FX_BOOL bBBox = FALSE) const;
    virtual FX_INT32	GetCharIndex(const CFX_PointF &fPoint, FX_BOOL &bBefore);
    virtual FX_INT32	SelectWord(const CFX_PointF &fPoint, FX_INT32 &nCount);
    virtual FX_INT32	GetCharStart() const;
    virtual FX_INT32	GetCharCount() const;
    virtual void		CalcRangeRectArray(FX_INT32 nStart, FX_INT32 nCount, CFX_RectFArray &RectFArr) const;
    virtual FX_INT32	GetDisplayPos(const CFX_RectF &rtClip, FXTEXT_CHARPOS *&pCharPos, FX_LPRECTF pBBox) const;

    virtual FX_BOOL				IsLoaded(FX_LPCRECTF pClipBox = NULL);
    virtual FX_INT32			LoadPage(FX_LPCRECTF pClipBox = NULL, IFX_Pause *pPause = NULL);
    virtual void				UnloadPage(FX_LPCRECTF pClipBox = NULL);

    virtual const CFX_RectF&	GetContentsBox();
    virtual FX_POSITION			GetFirstPosition(FDE_HVISUALOBJ hCanvas);
    virtual FDE_HVISUALOBJ		GetNext(FDE_HVISUALOBJ hCanvas, FX_POSITION &pos, IFDE_VisualSet *&pVisualSet);
    virtual FDE_HVISUALOBJ		GetParentCanvas(FDE_HVISUALOBJ hCanvas, IFDE_VisualSet *&pVisualSet);
    virtual FX_WCHAR	GetChar(FX_LPVOID pIdentity, FX_INT32 index) const;
    virtual FX_INT32	GetWidth(FX_LPVOID pIdentity, FX_INT32 index) const;
protected:
    virtual ~CFDE_TxtEdtPage();
private:
    void NormalizePt2Rect(CFX_PointF &ptF, const CFX_RectF &rtF, FX_FLOAT fTolerance) const;
    IFX_CharIter				*m_pIter;
    CFDE_TxtEdtTextSet			*m_pTextSet;
    CFDE_TxtEdtEngine			*m_pEditEngine;
    CFDE_TXTEDTPieceMassArray	m_PieceMassArr;
    CFDE_TxtEdtParag *			m_pBgnParag;
    CFDE_TxtEdtParag *			m_pEndParag;

    FX_INT32	m_nRefCount;
    FX_INT32	m_nPageStart;
    FX_INT32	m_nCharCount;
    FX_INT32	m_nPageIndex;
    FX_BOOL		m_bLoaded;
    FX_BOOL		m_bLastPage;
    CFX_RectF	m_rtPage;
    CFX_RectF	m_rtPageMargin;
    CFX_RectF	m_rtPageContents;
    CFX_RectF	m_rtPageCanvas;
    FX_INT32*	m_pCharWidth;
};
#endif
