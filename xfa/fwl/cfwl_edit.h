// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EDIT_H_
#define XFA_FWL_CFWL_EDIT_H_

#include <deque>
#include <memory>
#include <vector>

#include "xfa/fde/cfde_txtedtengine.h"
#include "xfa/fde/ifde_txtedtdorecord.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_scrollbar.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxgraphics/cfx_path.h"

#define FWL_STYLEEXT_EDT_ReadOnly (1L << 0)
#define FWL_STYLEEXT_EDT_MultiLine (1L << 1)
#define FWL_STYLEEXT_EDT_WantReturn (1L << 2)
#define FWL_STYLEEXT_EDT_AutoHScroll (1L << 4)
#define FWL_STYLEEXT_EDT_AutoVScroll (1L << 5)
#define FWL_STYLEEXT_EDT_Validate (1L << 7)
#define FWL_STYLEEXT_EDT_Password (1L << 8)
#define FWL_STYLEEXT_EDT_Number (1L << 9)
#define FWL_STYLEEXT_EDT_CombText (1L << 17)
#define FWL_STYLEEXT_EDT_HNear 0
#define FWL_STYLEEXT_EDT_HCenter (1L << 18)
#define FWL_STYLEEXT_EDT_HFar (2L << 18)
#define FWL_STYLEEXT_EDT_VNear 0
#define FWL_STYLEEXT_EDT_VCenter (1L << 20)
#define FWL_STYLEEXT_EDT_VFar (2L << 20)
#define FWL_STYLEEXT_EDT_Justified (1L << 22)
#define FWL_STYLEEXT_EDT_HAlignMask (3L << 18)
#define FWL_STYLEEXT_EDT_VAlignMask (3L << 20)
#define FWL_STYLEEXT_EDT_HAlignModeMask (3L << 22)
#define FWL_STYLEEXT_EDT_ShowScrollbarFocus (1L << 25)
#define FWL_STYLEEXT_EDT_OuterScrollbar (1L << 26)
#define FWL_STYLEEXT_EDT_LastLineHeight (1L << 27)

class IFDE_TxtEdtDoRecord;
class CFWL_Edit;
class CFWL_MessageMouse;
class CFWL_WidgetProperties;
class CFWL_Caret;

class CFWL_Edit : public CFWL_Widget {
 public:
  CFWL_Edit(const CFWL_App* app,
            std::unique_ptr<CFWL_WidgetProperties> properties,
            CFWL_Widget* pOuter);
  ~CFWL_Edit() override;

  // CFWL_Widget:
  FWL_Type GetClassID() const override;
  CFX_RectF GetAutosizedWidgetRect() override;
  CFX_RectF GetWidgetRect() override;
  void Update() override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  void SetStates(uint32_t dwStates) override;
  void DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix) override;
  void SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

  virtual void SetText(const CFX_WideString& wsText);

  int32_t GetTextLength() const;
  CFX_WideString GetText() const;
  void ClearText();

  void AddSelRange(int32_t nStart);
  int32_t CountSelRanges() const;
  int32_t GetSelRange(int32_t nIndex, int32_t* nStart) const;
  void ClearSelections();
  int32_t GetLimit() const;
  void SetLimit(int32_t nLimit);
  void SetAliasChar(FX_WCHAR wAlias);
  bool Copy(CFX_WideString& wsCopy);
  bool Cut(CFX_WideString& wsCut);
  bool Paste(const CFX_WideString& wsPaste);
  bool Redo(const IFDE_TxtEdtDoRecord* pRecord);
  bool Undo(const IFDE_TxtEdtDoRecord* pRecord);
  bool Undo();
  bool Redo();
  bool CanUndo();
  bool CanRedo();

  void SetOuter(CFWL_Widget* pOuter);

  void OnCaretChanged();
  void OnTextChanged(const FDE_TXTEDT_TEXTCHANGE_INFO& ChangeInfo);
  void OnSelChanged();
  bool OnPageLoad(int32_t nPageIndex);
  bool OnPageUnload(int32_t nPageIndex);
  void OnAddDoRecord(std::unique_ptr<IFDE_TxtEdtDoRecord> pRecord);
  bool OnValidate(const CFX_WideString& wsText);
  void SetScrollOffset(FX_FLOAT fScrollOffset);

 protected:
  void ShowCaret(CFX_RectF* pRect);
  void HideCaret(CFX_RectF* pRect);
  const CFX_RectF& GetRTClient() const { return m_rtClient; }
  CFDE_TxtEdtEngine* GetTxtEdtEngine() { return &m_EdtEngine; }

 private:
  void DrawTextBk(CFX_Graphics* pGraphics,
                  IFWL_ThemeProvider* pTheme,
                  const CFX_Matrix* pMatrix);
  void DrawContent(CFX_Graphics* pGraphics,
                   IFWL_ThemeProvider* pTheme,
                   const CFX_Matrix* pMatrix);
  void DrawSpellCheck(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix);

  void UpdateEditEngine();
  void UpdateEditParams();
  void UpdateEditLayout();
  bool UpdateOffset();
  bool UpdateOffset(CFWL_ScrollBar* pScrollBar, FX_FLOAT fPosChanged);
  void UpdateVAlignment();
  void UpdateCaret();
  CFWL_ScrollBar* UpdateScroll();
  void Layout();
  void LayoutScrollBar();
  CFX_PointF DeviceToEngine(const CFX_PointF& pt);
  void InitVerticalScrollBar();
  void InitHorizontalScrollBar();
  void InitEngine();
  void InitCaret();
  bool ValidateNumberChar(FX_WCHAR cNum);
  void ClearRecord();
  bool IsShowScrollBar(bool bVert);
  bool IsContentHeightOverflow();
  int32_t AddDoRecord(std::unique_ptr<IFDE_TxtEdtDoRecord> pRecord);
  void ProcessInsertError(int32_t iError);
  void AddSpellCheckObj(CFX_Path& PathData,
                        int32_t nStart,
                        int32_t nCount,
                        FX_FLOAT fOffSetX,
                        FX_FLOAT fOffSetY);

  void DoButtonDown(CFWL_MessageMouse* pMsg);
  void OnFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnButtonDblClk(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnKeyDown(CFWL_MessageKey* pMsg);
  void OnChar(CFWL_MessageKey* pMsg);
  bool OnScroll(CFWL_ScrollBar* pScrollBar,
                CFWL_EventScroll::Code dwCode,
                FX_FLOAT fPos);

  CFX_RectF m_rtClient;
  CFX_RectF m_rtEngine;
  CFX_RectF m_rtStatic;
  FX_FLOAT m_fVAlignOffset;
  FX_FLOAT m_fScrollOffsetX;
  FX_FLOAT m_fScrollOffsetY;
  CFDE_TxtEdtEngine m_EdtEngine;
  bool m_bLButtonDown;
  int32_t m_nSelStart;
  int32_t m_nLimit;
  FX_FLOAT m_fFontSize;
  bool m_bSetRange;
  int32_t m_iMax;
  std::unique_ptr<CFWL_ScrollBar> m_pVertScrollBar;
  std::unique_ptr<CFWL_ScrollBar> m_pHorzScrollBar;
  std::unique_ptr<CFWL_Caret> m_pCaret;
  CFX_WideString m_wsCache;
  CFX_WideString m_wsFont;
  std::deque<std::unique_ptr<IFDE_TxtEdtDoRecord>> m_DoRecords;
  int32_t m_iCurRecord;
  int32_t m_iMaxRecord;
};

#endif  // XFA_FWL_CFWL_EDIT_H_
