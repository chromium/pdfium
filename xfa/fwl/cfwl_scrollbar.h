// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_SCROLLBAR_H_
#define XFA_FWL_CFWL_SCROLLBAR_H_

#include <memory>

#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/cfwl_eventscroll.h"
#include "xfa/fwl/cfwl_timer.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetproperties.h"

class CFWL_Widget;

#define FWL_STYLEEXT_SCB_Horz (0L << 0)
#define FWL_STYLEEXT_SCB_Vert (1L << 0)

class CFWL_ScrollBar : public CFWL_Widget {
 public:
  CFWL_ScrollBar(const CFWL_App* app,
                 std::unique_ptr<CFWL_WidgetProperties> properties,
                 CFWL_Widget* pOuter);
  ~CFWL_ScrollBar() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void Update() override;
  void DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

  void GetRange(FX_FLOAT* fMin, FX_FLOAT* fMax) const {
    ASSERT(fMin);
    ASSERT(fMax);
    *fMin = m_fRangeMin;
    *fMax = m_fRangeMax;
  }
  void SetRange(FX_FLOAT fMin, FX_FLOAT fMax) {
    m_fRangeMin = fMin;
    m_fRangeMax = fMax;
  }
  FX_FLOAT GetPageSize() const { return m_fPageSize; }
  void SetPageSize(FX_FLOAT fPageSize) { m_fPageSize = fPageSize; }
  FX_FLOAT GetStepSize() const { return m_fStepSize; }
  void SetStepSize(FX_FLOAT fStepSize) { m_fStepSize = fStepSize; }
  FX_FLOAT GetPos() const { return m_fPos; }
  void SetPos(FX_FLOAT fPos) { m_fPos = fPos; }
  void SetTrackPos(FX_FLOAT fTrackPos);

 private:
  class Timer : public CFWL_Timer {
   public:
    explicit Timer(CFWL_ScrollBar* pToolTip);
    ~Timer() override {}

    void Run(CFWL_TimerInfo* pTimerInfo) override;
  };
  friend class CFWL_ScrollBar::Timer;

  bool IsVertical() const {
    return !!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_SCB_Vert);
  }
  void DrawTrack(CFX_Graphics* pGraphics,
                 IFWL_ThemeProvider* pTheme,
                 bool bLower,
                 const CFX_Matrix* pMatrix);
  void DrawArrowBtn(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    bool bMinBtn,
                    const CFX_Matrix* pMatrix);
  void DrawThumb(CFX_Graphics* pGraphics,
                 IFWL_ThemeProvider* pTheme,
                 const CFX_Matrix* pMatrix);
  void Layout();
  void CalcButtonLen();
  CFX_RectF CalcMinButtonRect();
  CFX_RectF CalcMaxButtonRect();
  CFX_RectF CalcThumbButtonRect(const CFX_RectF& rtThumbRect);
  CFX_RectF CalcMinTrackRect(const CFX_RectF& rtMinRect);
  CFX_RectF CalcMaxTrackRect(const CFX_RectF& rtMaxRect);
  FX_FLOAT GetTrackPointPos(const CFX_PointF& point);

  bool SendEvent();
  bool OnScroll(CFWL_EventScroll::Code dwCode, FX_FLOAT fPos);
  void OnLButtonDown(const CFX_PointF& point);
  void OnLButtonUp(const CFX_PointF& point);
  void OnMouseMove(const CFX_PointF& point);
  void OnMouseLeave();
  void OnMouseWheel(const CFX_PointF& delta);
  bool DoScroll(CFWL_EventScroll::Code dwCode, FX_FLOAT fPos);
  void DoMouseDown(int32_t iItem,
                   const CFX_RectF& rtItem,
                   int32_t& iState,
                   const CFX_PointF& point);
  void DoMouseUp(int32_t iItem,
                 const CFX_RectF& rtItem,
                 int32_t& iState,
                 const CFX_PointF& point);
  void DoMouseMove(int32_t iItem,
                   const CFX_RectF& rtItem,
                   int32_t& iState,
                   const CFX_PointF& point);
  void DoMouseLeave(int32_t iItem, const CFX_RectF& rtItem, int32_t& iState);
  void DoMouseHover(int32_t iItem, const CFX_RectF& rtItem, int32_t& iState);

  CFWL_TimerInfo* m_pTimerInfo;
  FX_FLOAT m_fRangeMin;
  FX_FLOAT m_fRangeMax;
  FX_FLOAT m_fPageSize;
  FX_FLOAT m_fStepSize;
  FX_FLOAT m_fPos;
  FX_FLOAT m_fTrackPos;
  int32_t m_iMinButtonState;
  int32_t m_iMaxButtonState;
  int32_t m_iThumbButtonState;
  int32_t m_iMinTrackState;
  int32_t m_iMaxTrackState;
  FX_FLOAT m_fLastTrackPos;
  CFX_PointF m_cpTrackPoint;
  int32_t m_iMouseWheel;
  bool m_bMouseDown;
  FX_FLOAT m_fButtonLen;
  bool m_bMinSize;
  CFX_RectF m_rtClient;
  CFX_RectF m_rtThumb;
  CFX_RectF m_rtMinBtn;
  CFX_RectF m_rtMaxBtn;
  CFX_RectF m_rtMinTrack;
  CFX_RectF m_rtMaxTrack;
  CFWL_ScrollBar::Timer m_Timer;
};

#endif  // XFA_FWL_CFWL_SCROLLBAR_H_
