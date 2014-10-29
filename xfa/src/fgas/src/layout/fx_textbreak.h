// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_TEXTBREAK_IMP
#define _FX_TEXTBREAK_IMP
class CFX_TxtLine;
class CFX_Txtbreak;
class CFX_TxtLine : public CFX_Object
{
public:
    CFX_TxtLine(FX_INT32 iBlockSize) : m_iStart(0)
        , m_iWidth(0)
        , m_iArabicChars(0)
    {
        m_pLineChars = FX_NEW CFX_TxtCharArray;
        m_pLinePieces = FXTARGET_New CFX_TxtPieceArray(16);
    }
    ~CFX_TxtLine()
    {
        RemoveAll();
        delete m_pLineChars;
        FXTARGET_Delete m_pLinePieces;
    }
    FX_INT32 CountChars() const
    {
        return m_pLineChars->GetSize();
    }
    CFX_TxtChar* GetCharPtr(FX_INT32 index) const
    {
        FXSYS_assert(index > -1 && index < m_pLineChars->GetSize());
        return m_pLineChars->GetDataPtr(index);
    }
    FX_INT32 CountPieces() const
    {
        return m_pLinePieces->GetSize();
    }
    CFX_TxtPiece* GetPiecePtr(FX_INT32 index) const
    {
        FXSYS_assert(index > -1 && index < m_pLinePieces->GetSize());
        return m_pLinePieces->GetPtrAt(index);
    }
    void GetString(CFX_WideString &wsStr) const
    {
        FX_INT32 iCount = m_pLineChars->GetSize();
        FX_LPWSTR pBuf = wsStr.GetBuffer(iCount);
        CFX_Char *pChar;
        for (FX_INT32 i = 0; i < iCount; i ++) {
            pChar = m_pLineChars->GetDataPtr(i);
            *pBuf ++ = (FX_WCHAR)pChar->m_wCharCode;
        }
        wsStr.ReleaseBuffer(iCount);
    }
    void RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        m_pLineChars->RemoveAll();
        m_pLinePieces->RemoveAll(bLeaveMemory);
        m_iWidth = 0;
        m_iArabicChars = 0;
    }
    CFX_TxtCharArray	*m_pLineChars;
    CFX_TxtPieceArray	*m_pLinePieces;
    FX_INT32			m_iStart;
    FX_INT32			m_iWidth;
    FX_INT32			m_iArabicChars;
};
class CFX_TxtBreak : public IFX_TxtBreak, public CFX_Object
{
public:
    CFX_TxtBreak(FX_DWORD dwPolicies);
    ~CFX_TxtBreak();
    virtual void			Release()
    {
        delete this;
    }
    virtual void			SetLineWidth(FX_FLOAT fLineWidth);
    virtual void			SetLinePos(FX_FLOAT fLinePos);
    virtual FX_DWORD		GetLayoutStyles() const
    {
        return m_dwLayoutStyles;
    }
    virtual void			SetLayoutStyles(FX_DWORD dwLayoutStyles);
    virtual void			SetFont(IFX_Font *pFont);
    virtual void			SetFontSize(FX_FLOAT fFontSize);
    virtual void			SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant);
    virtual void			SetDefaultChar(FX_WCHAR wch);
    virtual void			SetParagraphBreakChar(FX_WCHAR wch);
    virtual void			SetLineBreakTolerance(FX_FLOAT fTolerance);
    virtual void			SetHorizontalScale(FX_INT32 iScale);
    virtual void			SetVerticalScale(FX_INT32 iScale);
    virtual void			SetCharRotation(FX_INT32 iCharRotation);
    virtual void			SetCharSpace(FX_FLOAT fCharSpace);
    virtual void			SetAlignment(FX_INT32 iAlignment);
    virtual FX_DWORD		GetContextCharStyles() const;
    virtual void			SetContextCharStyles(FX_DWORD dwCharStyles);
    virtual void			SetCombWidth(FX_FLOAT fCombWidth);
    virtual void			SetUserData(FX_LPVOID pUserData);
    virtual FX_DWORD				AppendChar(FX_WCHAR wch);
    virtual FX_DWORD				EndBreak(FX_DWORD dwStatus = FX_TXTBREAK_PieceBreak);
    virtual FX_INT32				CountBreakChars() const;
    virtual FX_INT32				CountBreakPieces() const;
    virtual const CFX_TxtPiece*		GetBreakPiece(FX_INT32 index) const;
    virtual void					ClearBreakPieces();
    virtual void			Reset();
    virtual FX_INT32		GetDisplayPos(FX_LPCTXTRUN pTxtRun, FXTEXT_CHARPOS *pCharPos, FX_BOOL bCharCode = FALSE, CFX_WideString *pWSForms = NULL, FX_AdjustCharDisplayPos pAdjustPos = NULL) const;
    virtual FX_INT32		GetCharRects(FX_LPCTXTRUN pTxtRun, CFX_RectFArray &rtArray, FX_BOOL bCharBBox = FALSE) const;
    void				AppendChar_PageLoad(CFX_Char *pCurChar, FX_DWORD dwProps);
    FX_DWORD			AppendChar_Combination(CFX_Char *pCurChar, FX_INT32 iRotation);
    FX_DWORD			AppendChar_Tab(CFX_Char *pCurChar, FX_INT32 iRotation);
    FX_DWORD			AppendChar_Control(CFX_Char *pCurChar, FX_INT32 iRotation);
    FX_DWORD			AppendChar_Arabic(CFX_Char *pCurChar, FX_INT32 iRotation);
    FX_DWORD			AppendChar_Others(CFX_Char *pCurChar, FX_INT32 iRotation);
protected:
    FX_DWORD			m_dwPolicies;
    FX_BOOL				m_bPagination;
    IFX_ArabicChar		*m_pArabicChar;
    FX_INT32			m_iLineWidth;
    FX_DWORD			m_dwLayoutStyles;
    FX_BOOL				m_bVertical;
    FX_BOOL				m_bArabicContext;
    FX_BOOL				m_bArabicShapes;
    FX_BOOL				m_bRTL;
    FX_BOOL				m_bSingleLine;
    FX_BOOL				m_bCombText;
    FX_INT32			m_iArabicContext;
    FX_INT32			m_iCurArabicContext;
    IFX_Font			*m_pFont;
    FX_INT32			m_iFontSize;
    FX_BOOL				m_bEquidistant;
    FX_INT32			m_iTabWidth;
    FX_WCHAR			m_wDefChar;
    FX_WCHAR			m_wParagBreakChar;
    FX_INT32			m_iDefChar;
    FX_INT32			m_iLineRotation;
    FX_INT32			m_iCharRotation;
    FX_INT32			m_iRotation;
    FX_INT32			m_iAlignment;
    FX_DWORD			m_dwContextCharStyles;
    FX_INT32			m_iCombWidth;
    FX_LPVOID			m_pUserData;
    FX_DWORD			m_dwCharType;
    FX_BOOL				m_bCurRTL;
    FX_INT32			m_iCurAlignment;
    FX_BOOL				m_bArabicNumber;
    FX_BOOL				m_bArabicComma;
    CFX_TxtLine			*m_pTxtLine1;
    CFX_TxtLine			*m_pTxtLine2;
    CFX_TxtLine			*m_pCurLine;
    FX_INT32			m_iReady;
    FX_INT32			m_iTolerance;
    FX_INT32			m_iHorScale;
    FX_INT32			m_iVerScale;
    FX_INT32			m_iCharSpace;
    void				SetBreakStatus();
    FX_INT32			GetLineRotation(FX_DWORD dwStyles) const;
    CFX_TxtChar*		GetLastChar(FX_INT32 index, FX_BOOL bOmitChar = TRUE) const;
    CFX_TxtLine*		GetTxtLine(FX_BOOL bReady) const;
    CFX_TxtPieceArray*	GetTxtPieces(FX_BOOL bReady) const;
    FX_DWORD			GetUnifiedCharType(FX_DWORD dwType) const;
    void				ResetArabicContext();
    void				ResetContextCharStyles();
    void				EndBreak_UpdateArabicShapes();
    FX_BOOL				EndBreak_SplitLine(CFX_TxtLine *pNextLine, FX_BOOL bAllChars, FX_DWORD dwStatus);
    void				EndBreak_BidiLine(CFX_TPOArray &tpos, FX_DWORD dwStatus);
    void				EndBreak_Alignment(CFX_TPOArray &tpos, FX_BOOL bAllChars, FX_DWORD dwStatus);
    FX_INT32			GetBreakPos(CFX_TxtCharArray &ca, FX_INT32 &iEndPos, FX_BOOL bAllChars = FALSE, FX_BOOL bOnlyBrk = FALSE);
    void				SplitTextLine(CFX_TxtLine *pCurLine, CFX_TxtLine *pNextLine, FX_BOOL bAllChars = FALSE);
};
#endif
