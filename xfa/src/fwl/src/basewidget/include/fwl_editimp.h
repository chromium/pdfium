// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_EDIT_IMP_H
#define _FWL_EDIT_IMP_H

#include <memory>

class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class CFWL_ScrollBarImp;
class IFWL_Caret;
class IFWL_AdapterTextField;
class CFWL_EditImp;
class CFWL_EditImpDelegate;
class CFWL_EditImp : public CFWL_WidgetImp, public IFDE_TxtEdtEventSink {
 public:
  CFWL_EditImp(const CFWL_WidgetImpProperties& properties, IFWL_Widget* pOuter);
  ~CFWL_EditImp() override;

  // CFWL_WidgetImp:
  FWL_ERR GetClassName(CFX_WideString& wsClass) const override;
  FX_DWORD GetClassID() const override;
  FWL_ERR Initialize() override;
  FWL_ERR Finalize() override;
  FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_ERR SetWidgetRect(const CFX_RectF& rect) override;
  FWL_ERR Update() override;
  FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  FWL_ERR SetStates(FX_DWORD dwStates, FX_BOOL bSet = TRUE) override;
  FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                     const CFX_Matrix* pMatrix = NULL) override;
  FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) override;

  virtual FWL_ERR SetText(const CFX_WideString& wsText);
  virtual int32_t GetTextLength() const;
  virtual FWL_ERR GetText(CFX_WideString& wsText,
                          int32_t nStart = 0,
                          int32_t nCount = -1) const;
  virtual FWL_ERR ClearText();
  virtual int32_t GetCaretPos() const;
  virtual int32_t SetCaretPos(int32_t nIndex, FX_BOOL bBefore = TRUE);
  virtual FWL_ERR AddSelRange(int32_t nStart, int32_t nCount = -1);
  virtual int32_t CountSelRanges();
  virtual int32_t GetSelRange(int32_t nIndex, int32_t& nStart);
  virtual FWL_ERR ClearSelections();
  virtual int32_t GetLimit();
  virtual FWL_ERR SetLimit(int32_t nLimit);
  virtual FWL_ERR SetAliasChar(FX_WCHAR wAlias);
  virtual FWL_ERR SetFormatString(const CFX_WideString& wsFormat);
  virtual FWL_ERR Insert(int32_t nStart, const FX_WCHAR* lpText, int32_t nLen);
  virtual FWL_ERR DeleteSelections();
  virtual FWL_ERR DeleteRange(int32_t nStart, int32_t nCount = -1);
  virtual FWL_ERR ReplaceSelections(const CFX_WideStringC& wsReplace);
  virtual FWL_ERR Replace(int32_t nStart,
                          int32_t nLen,
                          const CFX_WideStringC& wsReplace);
  virtual FWL_ERR DoClipboard(int32_t iCmd);
  virtual FX_BOOL Copy(CFX_WideString& wsCopy);
  virtual FX_BOOL Cut(CFX_WideString& wsCut);
  virtual FX_BOOL Paste(const CFX_WideString& wsPaste);
  virtual FX_BOOL Delete();
  virtual FX_BOOL Redo(const CFX_ByteStringC& bsRecord);
  virtual FX_BOOL Undo(const CFX_ByteStringC& bsRecord);
  virtual FX_BOOL Undo();
  virtual FX_BOOL Redo();
  virtual FX_BOOL CanUndo();
  virtual FX_BOOL CanRedo();
  virtual FWL_ERR SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant);
  virtual FWL_ERR SetOuter(IFWL_Widget* pOuter);
  virtual FWL_ERR SetNumberRange(int32_t iMin, int32_t iMax);
  void On_CaretChanged(IFDE_TxtEdtEngine* pEdit,
                       int32_t nPage,
                       FX_BOOL bVisible = true) override;
  void On_TextChanged(IFDE_TxtEdtEngine* pEdit,
                      FDE_TXTEDT_TEXTCHANGE_INFO& ChangeInfo) override;
  void On_PageCountChanged(IFDE_TxtEdtEngine* pEdit) override {}
  void On_SelChanged(IFDE_TxtEdtEngine* pEdit) override;
  FX_BOOL On_PageLoad(IFDE_TxtEdtEngine* pEdit,
                      int32_t nPageIndex,
                      int32_t nPurpose) override;
  FX_BOOL On_PageUnload(IFDE_TxtEdtEngine* pEdit,
                        int32_t nPageIndex,
                        int32_t nPurpose) override;
  FX_BOOL On_PageChange(IFDE_TxtEdtEngine* pEdit, int32_t nPageIndex) override {
    return TRUE;
  }
  void On_AddDoRecord(IFDE_TxtEdtEngine* pEdit,
                      const CFX_ByteStringC& bsDoRecord) override;
  FX_BOOL On_ValidateField(IFDE_TxtEdtEngine* pEdit,
                           int32_t nBlockIndex,
                           int32_t nFieldIndex,
                           const CFX_WideString& wsFieldText,
                           int32_t nCharIndex) override;
  FX_BOOL On_ValidateBlock(IFDE_TxtEdtEngine* pEdit,
                           int32_t nBlockIndex) override;
  FX_BOOL On_GetBlockFormatText(IFDE_TxtEdtEngine* pEdit,
                                int32_t nBlockIndex,
                                CFX_WideString& wsBlockText) override;
  FX_BOOL On_Validate(IFDE_TxtEdtEngine* pEdit,
                      CFX_WideString& wsText) override;
  virtual FWL_ERR SetBackgroundColor(FX_DWORD color);
  virtual FWL_ERR SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize);
  void SetScrollOffset(FX_FLOAT fScrollOffset);
  FX_BOOL GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray& sSuggest);
  FX_BOOL ReplaceSpellCheckWord(CFX_PointF pointf,
                                const CFX_ByteStringC& bsReplace);

 protected:
  void DrawTextBk(CFX_Graphics* pGraphics,
                  IFWL_ThemeProvider* pTheme,
                  const CFX_Matrix* pMatrix = NULL);
  void DrawContent(CFX_Graphics* pGraphics,
                   IFWL_ThemeProvider* pTheme,
                   const CFX_Matrix* pMatrix = NULL);
  void UpdateEditEngine();
  void UpdateEditParams();
  void UpdateEditLayout();
  FX_BOOL UpdateOffset();
  FX_BOOL UpdateOffset(IFWL_ScrollBar* pScrollBar, FX_FLOAT fPosChanged);
  void UpdateVAlignment();
  void UpdateCaret();
  IFWL_ScrollBar* UpdateScroll();
  void Layout();
  void LayoutScrollBar();
  void DeviceToEngine(CFX_PointF& pt);
  void InitScrollBar(FX_BOOL bVert = TRUE);
  void InitEngine();
  virtual void ShowCaret(FX_BOOL bVisible, CFX_RectF* pRect = NULL);
  FX_BOOL ValidateNumberChar(FX_WCHAR cNum);
  void InitCaret();
  void ClearRecord();
  FX_BOOL IsShowScrollBar(FX_BOOL bVert);
  FX_BOOL IsContentHeightOverflow();
  int32_t AddDoRecord(const CFX_ByteStringC& bsDoRecord);
  void ProcessInsertError(int32_t iError);

  void DrawSpellCheck(CFX_Graphics* pGraphics,
                      const CFX_Matrix* pMatrix = NULL);
  void AddSpellCheckObj(CFX_Path& PathData,
                        int32_t nStart,
                        int32_t nCount,
                        FX_FLOAT fOffSetX,
                        FX_FLOAT fOffSetY);
  int32_t GetWordAtPoint(CFX_PointF pointf, int32_t& nCount);
  CFX_RectF m_rtClient;
  CFX_RectF m_rtEngine;
  CFX_RectF m_rtStatic;
  FX_FLOAT m_fVAlignOffset;
  FX_FLOAT m_fScrollOffsetX;
  FX_FLOAT m_fScrollOffsetY;
  IFDE_TxtEdtEngine* m_pEdtEngine;
  FX_BOOL m_bLButtonDown;
  int32_t m_nSelStart;
  int32_t m_nLimit;
  FX_FLOAT m_fSpaceAbove;
  FX_FLOAT m_fSpaceBelow;
  FX_FLOAT m_fFontSize;
  FX_ARGB m_argbSel;
  FX_BOOL m_bSetRange;
  int32_t m_iMin;
  int32_t m_iMax;
  std::unique_ptr<IFWL_ScrollBar> m_pVertScrollBar;
  std::unique_ptr<IFWL_ScrollBar> m_pHorzScrollBar;
  std::unique_ptr<IFWL_Caret> m_pCaret;
  CFX_WideString m_wsCache;
  friend class CFWL_TxtEdtEventSink;
  friend class CFWL_EditImpDelegate;
  FX_DWORD m_backColor;
  FX_BOOL m_updateBackColor;
  CFX_WideString m_wsFont;
  CFX_ByteStringArray m_RecordArr;
  int32_t m_iCurRecord;
  int32_t m_iMaxRecord;
};
class CFWL_EditImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_EditImpDelegate(CFWL_EditImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  void DoActivate(CFWL_MsgActivate* pMsg);
  void DoDeactivate(CFWL_MsgDeactivate* pMsg);
  void DoButtonDown(CFWL_MsgMouse* pMsg);
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnButtonDblClk(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnKeyDown(CFWL_MsgKey* pMsg);
  void OnChar(CFWL_MsgKey* pMsg);
  FX_BOOL OnScroll(IFWL_ScrollBar* pScrollBar, FX_DWORD dwCode, FX_FLOAT fPos);
  void DoCursor(CFWL_MsgMouse* pMsg);
  CFWL_EditImp* m_pOwner;
};
#endif
