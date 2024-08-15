// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_DATETIMEPICKER_H_
#define XFA_FWL_CFWL_DATETIMEPICKER_H_

#include <utility>

#include "v8/include/cppgc/member.h"
#include "xfa/fwl/cfwl_datetimeedit.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_monthcalendar.h"
#include "xfa/fwl/cfwl_widget.h"

namespace pdfium {

#define FWL_STYLEEXT_DTP_ShortDateFormat (1L << 1)
#define FWL_STYLEEXT_DTP_EditHAlignMask (3L << 4)
#define FWL_STYLEEXT_DTP_EditHNear (0L << 4)
#define FWL_STYLEEXT_DTP_EditHCenter (1L << 4)
#define FWL_STYLEEXT_DTP_EditHFar (2L << 4)
#define FWL_STYLEEXT_DTP_EditVAlignMask (3L << 6)
#define FWL_STYLEEXT_DTP_EditVNear (0L << 6)
#define FWL_STYLEEXT_DTP_EditVCenter (1L << 6)
#define FWL_STYLEEXT_DTP_EditVFar (2L << 6)
#define FWL_STYLEEXT_DTP_EditJustified (1L << 8)

class CFWL_DateTimeEdit;

class CFWL_DateTimePicker final : public CFWL_Widget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_DateTimePicker() override;

  // CFWL_Widget:
  void PreFinalize() override;
  void Trace(cppgc::Visitor* visitor) const override;
  FWL_Type GetClassID() const override;
  void Update() override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  void GetCurSel(int32_t& iYear, int32_t& iMonth, int32_t& iDay);
  void SetCurSel(int32_t iYear, int32_t iMonth, int32_t iDay);

  void SetEditText(const WideString& wsText);
  size_t GetEditTextLength() const;
  WideString GetEditText() const;
  void ClearText();

  void SelectAll();
  void ClearSelection();
  bool HasSelection() const { return m_pEdit->HasSelection(); }
  // Returns <start, count> of the selection.
  std::pair<size_t, size_t> GetSelection() const {
    return m_pEdit->GetSelection();
  }
  std::optional<WideString> Copy();
  std::optional<WideString> Cut();
  bool Paste(const WideString& wsPaste);
  bool Undo();
  bool Redo();
  bool CanUndo();
  bool CanRedo();

  CFX_RectF GetBBox() const;
  void SetEditLimit(int32_t nLimit) { m_pEdit->SetLimit(nLimit); }
  void ModifyEditStyleExts(uint32_t dwStyleExtsAdded,
                           uint32_t dwStyleExtsRemoved);

  bool IsMonthCalendarVisible() const;
  void ShowMonthCalendar();
  void HideMonthCalendar();
  void ProcessSelChanged(int32_t iYear, int32_t iMonth, int32_t iDay);

 private:
  explicit CFWL_DateTimePicker(CFWL_App* pApp);

  void DrawDropDownButton(CFGAS_GEGraphics* pGraphics,
                          const CFX_Matrix& mtMatrix);
  WideString FormatDateString(int32_t iYear, int32_t iMonth, int32_t iDay);
  void ResetEditAlignment();
  void GetPopupPos(float fMinHeight,
                   float fMaxHeight,
                   const CFX_RectF& rtAnchor,
                   CFX_RectF* pPopupRect);
  void OnFocusGained(CFWL_Message* pMsg);
  void OnFocusLost(CFWL_Message* pMsg);
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnMouseLeave(CFWL_MessageMouse* pMsg);
  bool NeedsToShowButton() const;
  void RepaintInflatedMonthCalRect();

  bool m_bLBtnDown = false;
  Mask<CFWL_PartState> m_iBtnState = CFWL_PartState::kChecked;
  int32_t m_iYear = -1;
  int32_t m_iMonth = -1;
  int32_t m_iDay = -1;
  float m_fBtn = 0.0f;
  CFX_RectF m_BtnRect;
  CFX_RectF m_ClientRect;
  cppgc::Member<CFWL_DateTimeEdit> const m_pEdit;
  cppgc::Member<CFWL_MonthCalendar> const m_pMonthCal;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_DateTimePicker;

#endif  // XFA_FWL_CFWL_DATETIMEPICKER_H_
