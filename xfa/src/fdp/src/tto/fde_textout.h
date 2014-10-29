// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TEXTOUT_IMP
#define _FDE_TEXTOUT_IMP
struct FDE_TTOPIECE {
public:
    FX_INT32	iStartChar;
    FX_INT32	iChars;
    FX_DWORD	dwCharStyles;
    CFX_RectF	rtPiece;
};
typedef FDE_TTOPIECE * FDE_LPTTOPIECE;
typedef CFX_MassArrayTemplate<FDE_TTOPIECE> CFDE_TTOPieceArray;
class CFDE_TTOLine : public CFX_Target
{
public:
    CFDE_TTOLine();
    CFDE_TTOLine(const CFDE_TTOLine &ttoLine);
    ~CFDE_TTOLine();
    FX_INT32		AddPiece(FX_INT32 index, const FDE_TTOPIECE &ttoPiece);
    FX_INT32		GetSize() const;
    FDE_LPTTOPIECE	GetPtrAt(FX_INT32 index);
    void			RemoveLast(FX_INT32 iCount);
    void			RemoveAll(FX_BOOL bLeaveMemory);
    FX_BOOL			m_bNewReload;
    CFDE_TTOPieceArray m_pieces;
protected:
    FX_INT32		m_iPieceCount;
};
typedef CFX_ObjectMassArrayTemplate<CFDE_TTOLine> CFDE_TTOLineArray;
class CFDE_TextOut : public IFDE_TextOut, public CFX_Target
{
public:
    CFDE_TextOut();
    ~CFDE_TextOut();
    virtual	void		Release()
    {
        FDE_Delete this;
    }
    virtual void		SetFont(IFX_Font *pFont);
    virtual void		SetFontSize(FX_FLOAT fFontSize);
    virtual void		SetTextColor(FX_ARGB color);
    virtual void		SetStyles(FX_DWORD dwStyles);
    virtual void		SetTabWidth(FX_FLOAT fTabWidth);
    virtual void		SetEllipsisString(const CFX_WideString &wsEllipsis);
    virtual void		SetParagraphBreakChar(FX_WCHAR wch);
    virtual void		SetAlignment(FX_INT32 iAlignment);
    virtual void		SetLineSpace(FX_FLOAT fLineSpace);
    virtual void		SetDIBitmap(CFX_DIBitmap *pDIB);
    virtual void		SetRenderDevice(CFX_RenderDevice *pDevice);
    virtual void		SetClipRect(const CFX_Rect &rtClip);
    virtual void		SetClipRect(const CFX_RectF &rtClip);
    virtual void		SetMatrix(const CFX_Matrix &matrix);
    virtual void		SetLineBreakTolerance(FX_FLOAT fTolerance);
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_Size &size);
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_SizeF &size);
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_Rect &rect);
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_RectF &rect);

    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, FX_INT32 x, FX_INT32 y);
    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, FX_FLOAT x, FX_FLOAT y);
    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_Rect &rect);
    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_RectF &rect);

    virtual void		SetLogicClipRect(const CFX_RectF &rtClip);
    virtual void		CalcLogicSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_SizeF &size);
    virtual void		CalcLogicSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_RectF &rect);
    virtual void		DrawLogicText(FX_LPCWSTR pwsStr, FX_INT32 iLength, FX_FLOAT x, FX_FLOAT y);
    virtual void		DrawLogicText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_RectF &rect);
    virtual FX_INT32	GetTotalLines();
protected:
    void				CalcTextSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_RectF &rect);
    FX_BOOL				RetrieveLineWidth(FX_DWORD dwBreakStatus, FX_FLOAT &fStartPos, FX_FLOAT &fWidth, FX_FLOAT &fHeight);
    void				SetLineWidth(CFX_RectF &rect);
    void				DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_RectF &rect, const CFX_RectF &rtClip);
    void				LoadText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_RectF &rect);
    void				LoadEllipsis();
    void				ExpandBuffer(FX_INT32 iSize, FX_INT32 iType);
    void				RetrieveEllPieces(FX_INT32 *&pCharWidths);

    void				Reload(const CFX_RectF &rect);
    void				ReloadLinePiece(CFDE_TTOLine *pLine, const CFX_RectF &rect);
    FX_BOOL				RetriecePieces(FX_DWORD dwBreakStatus, FX_INT32 &iStartChar, FX_INT32 &iPieceWidths, FX_BOOL bReload, const CFX_RectF &rect);
    void				AppendPiece(const FDE_TTOPIECE &ttoPiece, FX_BOOL bNeedReload, FX_BOOL bEnd);
    void				ReplaceWidthEllipsis();
    void				DoAlignment(const CFX_RectF &rect);
    void				OnDraw(const CFX_RectF &rtClip);
    FX_INT32			GetDisplayPos(FDE_LPTTOPIECE pPiece);
    FX_INT32			GetCharRects(FDE_LPTTOPIECE pPiece);

    void				ToTextRun(const FDE_LPTTOPIECE pPiece, FX_TXTRUN &tr);
    void				DrawLine(const FDE_LPTTOPIECE pPiece, IFDE_Pen *&pPen);

    IFX_TxtBreak		*m_pTxtBreak;
    IFX_Font			*m_pFont;
    FX_FLOAT			m_fFontSize;
    FX_FLOAT			m_fLineSpace;
    FX_FLOAT			m_fLinePos;
    FX_FLOAT			m_fTolerance;

    FX_INT32			m_iAlignment;
    FX_INT32			m_iTxtBkAlignment;
    FX_INT32			*m_pCharWidths;
    FX_INT32			m_iChars;
    FX_INT32			*m_pEllCharWidths;
    FX_INT32			m_iEllChars;
    FX_WCHAR			m_wParagraphBkChar;
    FX_ARGB				m_TxtColor;
    FX_DWORD			m_dwStyles;
    FX_DWORD			m_dwTxtBkStyles;
    CFX_WideString		m_wsEllipsis;
    FX_BOOL				m_bElliChanged;
    FX_INT32			m_iEllipsisWidth;
    CFX_WideString		m_wsText;
    CFX_RectF			m_rtClip;
    CFX_RectF			m_rtLogicClip;
    CFX_Matrix			m_Matrix;
    CFDE_TTOLineArray	m_ttoLines;
    FX_INT32			m_iCurLine;
    FX_INT32			m_iCurPiece;
    FX_INT32			m_iTotalLines;
    FXTEXT_CHARPOS		*m_pCharPos;
    FX_INT32			m_iCharPosSize;
    IFDE_RenderDevice	*m_pRenderDevice;
    CFX_Int32Array		m_hotKeys;
    CFX_RectFArray		m_rectArray;
};
#endif
