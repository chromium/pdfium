// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_RTFBREAK_IMP
#define _FX_RTFBREAK_IMP
class CFX_RTFLine;
class CFX_RTFBreak;
class CFX_RTFLine : public CFX_Object
{
public:
    CFX_RTFLine() : m_LinePieces(16)
        , m_iStart(0)
        , m_iWidth(0)
        , m_iArabicChars(0)
        , m_iMBCSChars(0)
    {
    }
    ~CFX_RTFLine()
    {
        RemoveAll();
    }
    FX_INT32 CountChars() const
    {
        return m_LineChars.GetSize();
    }
    CFX_RTFChar& GetChar(FX_INT32 index)
    {
        FXSYS_assert(index > -1 && index < m_LineChars.GetSize());
        return *m_LineChars.GetDataPtr(index);
    }
    CFX_RTFChar* GetCharPtr(FX_INT32 index)
    {
        FXSYS_assert(index > -1 && index < m_LineChars.GetSize());
        return m_LineChars.GetDataPtr(index);
    }
    FX_INT32 CountPieces() const
    {
        return m_LinePieces.GetSize();
    }
    CFX_RTFPiece& GetPiece(FX_INT32 index) const
    {
        FXSYS_assert(index > -1 && index < m_LinePieces.GetSize());
        return m_LinePieces.GetAt(index);
    }
    CFX_RTFPiece* GetPiecePtr(FX_INT32 index) const
    {
        FXSYS_assert(index > -1 && index < m_LinePieces.GetSize());
        return m_LinePieces.GetPtrAt(index);
    }
    FX_INT32 GetLineEnd() const
    {
        return m_iStart + m_iWidth;
    }
    void RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        CFX_RTFChar *pChar;
        IFX_Unknown *pUnknown;
        FX_INT32 iCount = m_LineChars.GetSize();
        for (FX_INT32 i = 0; i < iCount; i ++) {
            pChar = m_LineChars.GetDataPtr(i);
            if ((pUnknown = pChar->m_pUserData) != NULL) {
                pUnknown->Release();
            }
        }
        m_LineChars.RemoveAll();
        m_LinePieces.RemoveAll(bLeaveMemory);
        m_iWidth = 0;
        m_iArabicChars = 0;
        m_iMBCSChars = 0;
    }
    CFX_RTFCharArray	m_LineChars;
    CFX_RTFPieceArray	m_LinePieces;
    FX_INT32			m_iStart;
    FX_INT32			m_iWidth;
    FX_INT32			m_iArabicChars;
    FX_INT32			m_iMBCSChars;
};
class CFX_RTFBreak : public IFX_RTFBreak, public CFX_Object
{
public:
    CFX_RTFBreak(FX_DWORD dwPolicies);
    ~CFX_RTFBreak();
    virtual void			Release()
    {
        delete this;
    }
    virtual void			SetLineWidth(FX_FLOAT fLineStart, FX_FLOAT fLineEnd);
    virtual void			SetLinePos(FX_FLOAT fLinePos);
    virtual FX_DWORD		GetLayoutStyles() const
    {
        return m_dwLayoutStyles;
    }
    virtual void			SetLayoutStyles(FX_DWORD dwLayoutStyles);
    virtual void			SetFont(IFX_Font *pFont);
    virtual void			SetFontSize(FX_FLOAT fFontSize);
    virtual void			SetTabWidth(FX_FLOAT fTabWidth);
    virtual void			AddPositionedTab(FX_FLOAT fTabPos);
    virtual void			SetPositionedTabs(const CFX_FloatArray &tabs);
    virtual void			ClearPositionedTabs();
    virtual void			SetDefaultChar(FX_WCHAR wch);
    virtual void			SetLineBreakChar(FX_WCHAR wch);
    virtual void			SetLineBreakTolerance(FX_FLOAT fTolerance);
    virtual void			SetHorizontalScale(FX_INT32 iScale);
    virtual void			SetVerticalScale(FX_INT32 iScale);
    virtual void			SetCharRotation(FX_INT32 iCharRotation);
    virtual void			SetCharSpace(FX_FLOAT fCharSpace);
    virtual void			SetWordSpace(FX_BOOL bDefault, FX_FLOAT fWordSpace);
    virtual void			SetReadingOrder(FX_BOOL bRTL = FALSE);
    virtual void			SetAlignment(FX_INT32 iAlignment = FX_RTFLINEALIGNMENT_Left);
    virtual void			SetUserData(IFX_Unknown *pUserData);
    virtual FX_DWORD					AppendChar(FX_WCHAR wch);
    virtual FX_DWORD					EndBreak(FX_DWORD dwStatus = FX_RTFBREAK_PieceBreak);
    virtual FX_INT32					CountBreakPieces() const;
    virtual const CFX_RTFPiece*			GetBreakPiece(FX_INT32 index) const;
    virtual void						GetLineRect(CFX_RectF &rect) const;
    virtual void						ClearBreakPieces();
    virtual void			Reset();
    virtual FX_INT32		GetDisplayPos(FX_LPCRTFTEXTOBJ pText, FXTEXT_CHARPOS *pCharPos, FX_BOOL bCharCode = FALSE, CFX_WideString *pWSForms = NULL, FX_AdjustCharDisplayPos pAdjustPos = NULL) const;
    virtual FX_INT32		GetCharRects(FX_LPCRTFTEXTOBJ pText, CFX_RectFArray &rtArray, FX_BOOL bCharBBox = FALSE) const;
    FX_DWORD				AppendChar_CharCode(FX_WCHAR wch);
    FX_DWORD				AppendChar_Combination(CFX_RTFChar *pCurChar, FX_INT32 iRotation);
    FX_DWORD				AppendChar_Tab(CFX_RTFChar *pCurChar, FX_INT32 iRotation);
    FX_DWORD				AppendChar_Control(CFX_RTFChar *pCurChar, FX_INT32 iRotation);
    FX_DWORD				AppendChar_Arabic(CFX_RTFChar *pCurChar, FX_INT32 iRotation);
    FX_DWORD				AppendChar_Others(CFX_RTFChar *pCurChar, FX_INT32 iRotation);
protected:
    FX_DWORD			m_dwPolicies;
    IFX_ArabicChar		*m_pArabicChar;
    FX_INT32			m_iLineStart;
    FX_INT32			m_iLineEnd;
    FX_DWORD			m_dwLayoutStyles;
    FX_BOOL				m_bPagination;
    FX_BOOL				m_bVertical;
    FX_BOOL				m_bSingleLine;
    FX_BOOL				m_bCharCode;
    IFX_Font			*m_pFont;
    FX_INT32			m_iFontHeight;
    FX_INT32			m_iFontSize;
    FX_INT32			m_iTabWidth;
    CFX_Int32Array		m_PositionedTabs;
    FX_BOOL				m_bOrphanLine;
    FX_WCHAR			m_wDefChar;
    FX_INT32			m_iDefChar;
    FX_WCHAR			m_wLineBreakChar;
    FX_INT32			m_iHorizontalScale;
    FX_INT32			m_iVerticalScale;
    FX_INT32			m_iLineRotation;
    FX_INT32			m_iCharRotation;
    FX_INT32			m_iRotation;
    FX_INT32			m_iCharSpace;
    FX_BOOL				m_bWordSpace;
    FX_INT32			m_iWordSpace;
    FX_BOOL				m_bRTL;
    FX_INT32			m_iAlignment;
    IFX_Unknown			*m_pUserData;
    FX_DWORD			m_dwCharType;
    FX_DWORD			m_dwIdentity;
    CFX_RTFLine			m_RTFLine1;
    CFX_RTFLine			m_RTFLine2;
    CFX_RTFLine			*m_pCurLine;
    FX_INT32			m_iReady;
    FX_INT32			m_iTolerance;
    FX_INT32			GetLineRotation(FX_DWORD dwStyles) const;
    void				SetBreakStatus();
    CFX_RTFChar*		GetLastChar(FX_INT32 index) const;
    CFX_RTFLine*		GetRTFLine(FX_BOOL bReady) const;
    CFX_RTFPieceArray*	GetRTFPieces(FX_BOOL bReady) const;
    FX_DWORD			GetUnifiedCharType(FX_DWORD dwType) const;
    FX_INT32			GetLastPositionedTab() const;
    FX_BOOL				GetPositionedTab(FX_INT32 &iTabPos) const;
    FX_INT32			GetBreakPos(CFX_RTFCharArray &tca, FX_INT32 &iEndPos, FX_BOOL bAllChars = FALSE, FX_BOOL bOnlyBrk = FALSE);
    void				SplitTextLine(CFX_RTFLine *pCurLine, CFX_RTFLine *pNextLine, FX_BOOL bAllChars = FALSE);
    FX_BOOL				EndBreak_SplitLine(CFX_RTFLine *pNextLine, FX_BOOL bAllChars, FX_DWORD dwStatus);
    void				EndBreak_BidiLine(CFX_TPOArray &tpos, FX_DWORD dwStatus);
    void				EndBreak_Alignment(CFX_TPOArray &tpos, FX_BOOL bAllChars, FX_DWORD dwStatus);
};
#endif
