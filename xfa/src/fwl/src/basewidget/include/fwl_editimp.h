// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_EDIT_IMP_H
#define _FWL_EDIT_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class CFWL_ScrollBarImp;
class IFWL_Caret;
class IFWL_AdapterTextField;
class CFWL_EditImp;
class CFWL_EditImpDelegate;
class CFWL_EditImp : public CFWL_WidgetImp, public IFDE_TxtEdtEventSink
{
public:
    CFWL_EditImp(IFWL_Widget *pOuter = NULL);
    CFWL_EditImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter = NULL);
    ~CFWL_EditImp();
    virtual FWL_ERR		GetClassName(CFX_WideString &wsClass) const;
    virtual FX_DWORD	GetClassID() const;
    virtual FWL_ERR		Initialize();
    virtual FWL_ERR		Finalize();
    virtual FWL_ERR		GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize = FALSE);
    virtual FWL_ERR		SetWidgetRect(const CFX_RectF &rect);
    virtual	FWL_ERR		Update();
    virtual FX_DWORD	HitTest(FX_FLOAT fx, FX_FLOAT fy);
    virtual FWL_ERR		SetStates(FX_DWORD dwStates, FX_BOOL bSet = TRUE);
    virtual FWL_ERR		DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
    virtual FWL_ERR		SetThemeProvider(IFWL_ThemeProvider *pThemeProvider);
    virtual FWL_ERR		SetText(const CFX_WideString &wsText);
    virtual FX_INT32	GetTextLength() const;
    virtual FWL_ERR		GetText(CFX_WideString &wsText, FX_INT32 nStart = 0, FX_INT32 nCount = -1) const;
    virtual FWL_ERR		ClearText();
    virtual FX_INT32	GetCaretPos() const;
    virtual FX_INT32	SetCaretPos(FX_INT32 nIndex, FX_BOOL bBefore = TRUE);
    virtual FWL_ERR		AddSelRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    virtual FX_INT32	CountSelRanges();
    virtual FX_INT32	GetSelRange(FX_INT32 nIndex, FX_INT32 &nStart);
    virtual FWL_ERR		ClearSelections();
    virtual FX_INT32	GetLimit();
    virtual FWL_ERR		SetLimit(FX_INT32 nLimit);
    virtual FWL_ERR		SetAliasChar(FX_WCHAR wAlias);
    virtual FWL_ERR		SetFormatString(const CFX_WideString &wsFormat);
    virtual FWL_ERR		Insert(FX_INT32 nStart, FX_LPCWSTR lpText, FX_INT32 nLen);
    virtual FWL_ERR		DeleteSelections();
    virtual FWL_ERR		DeleteRange(FX_INT32 nStart, FX_INT32 nCount = -1);
    virtual FWL_ERR		ReplaceSelections(const CFX_WideStringC &wsReplace);
    virtual FWL_ERR		Replace(FX_INT32 nStart, FX_INT32 nLen, const CFX_WideStringC &wsReplace);
    virtual FWL_ERR		DoClipboard(FX_INT32 iCmd);
    virtual FX_BOOL		Copy(CFX_WideString &wsCopy);
    virtual FX_BOOL		Cut(CFX_WideString &wsCut);
    virtual FX_BOOL		Paste(const CFX_WideString &wsPaste);
    virtual FX_BOOL		Delete();
    virtual FX_BOOL		Redo(FX_BSTR bsRecord);
    virtual FX_BOOL		Undo(FX_BSTR bsRecord);
    virtual FX_BOOL		Undo();
    virtual FX_BOOL		Redo();
    virtual FX_BOOL		CanUndo();
    virtual FX_BOOL		CanRedo();
    virtual FWL_ERR		SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant);
    virtual FWL_ERR		SetOuter(IFWL_Widget *pOuter);
    virtual FWL_ERR		SetNumberRange(FX_INT32 iMin, FX_INT32 iMax);
    virtual void		On_CaretChanged(IFDE_TxtEdtEngine *pEdit, FX_INT32 nPage, FX_BOOL bVisible = TRUE);
    virtual void		On_TextChanged(IFDE_TxtEdtEngine * pEdit, FDE_TXTEDT_TEXTCHANGE_INFO &ChangeInfo);
    virtual void		On_PageCountChanged(IFDE_TxtEdtEngine *pEdit) {}
    virtual void		On_SelChanged(IFDE_TxtEdtEngine *pEdit);
    virtual FX_BOOL		On_PageLoad(IFDE_TxtEdtEngine *pEdit, FX_INT32 nPageIndex, FX_INT32 nPurpose);
    virtual FX_BOOL		On_PageUnload(IFDE_TxtEdtEngine *pEdit, FX_INT32 nPageIndex, FX_INT32 nPurpose);
    virtual FX_BOOL		On_PageChange(IFDE_TxtEdtEngine *pEdit, FX_BOOL bPageUp = TRUE)
    {
        return TRUE;
    }
    virtual void		On_AddDoRecord(IFDE_TxtEdtEngine *pEdit, FX_BSTR bsDoRecord);
    virtual FX_BOOL		On_ValidateField(IFDE_TxtEdtEngine *pEdit, FX_INT32 nBlockIndex, FX_INT32 nFieldIndex, \
                                         const CFX_WideString &wsFieldText, FX_INT32 nCharIndex);
    virtual FX_BOOL		On_ValidateBlock(IFDE_TxtEdtEngine *pEdit, FX_INT32 nBlockIndex);
    virtual FX_BOOL		On_GetBlockFormatText(IFDE_TxtEdtEngine *pEdit, FX_INT32 nBlockIndex, CFX_WideString &wsBlockText);
    virtual FX_BOOL		On_Validate(IFDE_TxtEdtEngine * pEdit, CFX_WideString &wsText);
    virtual FWL_ERR     SetBackgroundColor(FX_DWORD color);
    virtual FWL_ERR     SetFont(const CFX_WideString &wsFont, FX_FLOAT fSize);
    void				SetScrollOffset(FX_FLOAT fScrollOffset);
    FX_BOOL				GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray &sSuggest);
    FX_BOOL				ReplaceSpellCheckWord(CFX_PointF pointf, FX_BSTR bsReplace);
protected:
    void				DrawTextBk(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix = NULL);
    void				DrawContent(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix = NULL);
    void				UpdateEditEngine();
    void				UpdateEditParams();
    void				UpdateEditLayout();
    FX_BOOL				UpdateOffset();
    FX_BOOL				UpdateOffset(IFWL_ScrollBar *pScrollBar, FX_FLOAT fPosChanged);
    void				UpdateVAlignment();
    void				UpdateCaret();
    IFWL_ScrollBar*		UpdateScroll();
    void				Layout();
    void				LayoutScrollBar();
    void				DeviceToEngine(CFX_PointF &pt);
    void				InitScrollBar(FX_BOOL bVert = TRUE);
    void				InitEngine();
    virtual	void		ShowCaret(FX_BOOL bVisible, CFX_RectF *pRect = NULL);
    FX_BOOL				ValidateNumberChar(FX_WCHAR cNum);
    void				InitCaret();
    void				ClearRecord();
    FX_BOOL				IsShowScrollBar(FX_BOOL bVert);
    FX_BOOL				IsContentHeightOverflow();
    FX_INT32			AddDoRecord(FX_BSTR bsDoRecord);
    void				ProcessInsertError(FX_INT32 iError);

    void				DrawSpellCheck(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
    void				AddSpellCheckObj(CFX_Path& PathData, FX_INT32 nStart, FX_INT32 nCount, FX_FLOAT fOffSetX, FX_FLOAT fOffSetY);
    FX_INT32			GetWordAtPoint(CFX_PointF pointf, FX_INT32& nCount);
    CFX_RectF			m_rtClient;
    CFX_RectF			m_rtEngine;
    CFX_RectF			m_rtStatic;
    FX_FLOAT			m_fVAlignOffset;
    FX_FLOAT			m_fScrollOffsetX;
    FX_FLOAT			m_fScrollOffsetY;
    IFDE_TxtEdtEngine*	m_pEdtEngine;
    FX_BOOL				m_bLButtonDown;
    FX_INT32			m_nSelStart;
    FX_INT32			m_nLimit;
    FX_FLOAT			m_fSpaceAbove;
    FX_FLOAT			m_fSpaceBelow;
    FX_FLOAT			m_fFontSize;
    FX_ARGB				m_argbSel;
    FX_BOOL             m_bSetRange;
    FX_INT32			m_iMin;
    FX_INT32			m_iMax;
    IFWL_ScrollBar*		m_pVertScrollBar;
    IFWL_ScrollBar*		m_pHorzScrollBar;
    IFWL_Caret*			m_pCaret;
    IFWL_AdapterTextField*	m_pTextField;
    CFX_WideString			m_wsCache;
    friend class CFWL_TxtEdtEventSink;
    friend class CFWL_EditImpDelegate;
    FX_DWORD            m_backColor;
    FX_BOOL             m_updateBackColor;
    CFX_WideString      m_wsFont;
    CFX_ByteStringArray m_RecordArr;
    FX_INT32			m_iCurRecord;
    FX_INT32			m_iMaxRecord;
};
class CFWL_EditImpDelegate : public CFWL_WidgetImpDelegate
{
public:
    CFWL_EditImpDelegate(CFWL_EditImp *pOwner);
    virtual FX_INT32	OnProcessMessage(CFWL_Message *pMessage);
    virtual FWL_ERR		OnProcessEvent(CFWL_Event *pEvent);
    virtual FWL_ERR		OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
protected:
    void    DoActivate(CFWL_MsgActivate *pMsg);
    void    DoDeactivate(CFWL_MsgDeactivate *pMsg);
    void	DoButtonDown(CFWL_MsgMouse *pMsg);
    void	OnFocusChanged(CFWL_Message *pMsg, FX_BOOL bSet = TRUE);
    void	OnLButtonDown(CFWL_MsgMouse *pMsg);
    void	OnLButtonUp(CFWL_MsgMouse *pMsg);
    void	OnButtonDblClk(CFWL_MsgMouse *pMsg);
    void	OnMouseMove(CFWL_MsgMouse *pMsg);
    void	OnKeyDown(CFWL_MsgKey *pMsg);
    void	OnChar(CFWL_MsgKey *pMsg);
    FX_BOOL	OnScroll(IFWL_ScrollBar *pScrollBar, FX_DWORD dwCode, FX_FLOAT fPos);
    void    DoCursor(CFWL_MsgMouse *pMsg);
    CFWL_EditImp *m_pOwner;
};
#endif
