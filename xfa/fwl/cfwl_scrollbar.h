// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_SCROLLBAR_H_
#define XFA_FWL_CFWL_SCROLLBAR_H_

#include <memory>

#include "core/fxcrt/cfx_timer.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/check.h"
#include "xfa/fwl/cfwl_eventscroll.h"
#include "xfa/fwl/cfwl_widget.h"

#define FWL_STYLEEXT_SCB_Horz (0L << 0)
#define FWL_STYLEEXT_SCB_Vert (1L << 0)

class CFWL_ScrollBar final : public CFWL_Widget,
                             public CFX_Timer::CallbackIface {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_ScrollBar() override;

  // CFWL_Widget:
  FWL_Type GetClassID() const override;
  void Update() override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  // CFX_Timer::CallbackIface:
  void OnTimerFired() override;

  void GetRange(float* fMin, float* fMax) const {
    DCHECK(fMin);
    DCHECK(fMax);
    *fMin = m_fRangeMin;
    *fMax = m_fRangeMax;
  }
  void SetRange(float fMin, float fMax) {
    m_fRangeMin = fMin;
    m_fRangeMax = fMax;
  }
  float GetPageSize() const { return m_fPageSize; }
  void SetPageSize(float fPageSize) { m_fPageSize = fPageSize; }
  float GetStepSize() const { return m_fStepSize; }
  void SetStepSize(float fStepSize) { m_fStepSize = fStepSize; }
  float GetPos() const { return m_fPos; }
  void SetPos(float fPos) { m_fPos = fPos; }
  void SetTrackPos(float fTrackPos);

 private:
  CFWL_ScrollBar(CFWL_App* app,
                 const Properties& properties,
                 CFWL_Widget* pOuter);

  bool IsVertical() const { return !!(GetStyleExts() & FWL_STYLEEXT_SCB_Vert); }
  void DrawTrack(CFGAS_GEGraphics* pGraphics,
                 bool bLower,
                 const CFX_Matrix& mtMatrix);
  void DrawArrowBtn(CFGAS_GEGraphics* pGraphics,
                    bool bMinBtn,
                    const CFX_Matrix& mtMatrix);
  void DrawThumb(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void Layout();
  void CalcButtonLen();
  CFX_RectF CalcMinButtonRect();
  CFX_RectF CalcMaxButtonRect();
  CFX_RectF CalcThumbButtonRect(const CFX_RectF& rtThumbRect);
  CFX_RectF CalcMinTrackRect(const CFX_RectF& rtMinRect);
  CFX_RectF CalcMaxTrackRect(const CFX_RectF& rtMaxRect);
  float GetTrackPointPos(const CFX_PointF& point);

  bool SendEvent();
  bool OnScroll(CFWL_EventScroll::Code dwCode, float fPos);
  void OnLButtonDown(const CFX_PointF& point);
  void OnLButtonUp(const CFX_PointF& point);
  void OnMouseMove(const CFX_PointF& point);
  void OnMouseLeave();
  void OnMouseWheel(const CFX_Vector& delta);
  bool DoScroll(CFWL_EventScroll::Code dwCode, float fPos);
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

  float m_fRangeMin = 0.0f;
  float m_fRangeMax = -1.0f;
  float m_fPageSize = 0.0f;
  float m_fStepSize = 0.0f;
  float m_fPos = 0.0f;
  float m_fTrackPos = 0.0f;
  int32_t m_iMinButtonState = CFWL_PartState_Normal;
  int32_t m_iMaxButtonState = CFWL_PartState_Normal;
  int32_t m_iThumbButtonState = CFWL_PartState_Normal;
  int32_t m_iMinTrackState = CFWL_PartState_Normal;
  int32_t m_iMaxTrackState = CFWL_PartState_Normal;
  float m_fLastTrackPos = 0.0f;
  CFX_PointF m_cpTrackPoint;
  int32_t m_iMouseWheel = 0;
  float m_fButtonLen = 0.0f;
  bool m_bMouseDown = false;
  bool m_bMinSize = false;
  CFX_RectF m_ClientRect;
  CFX_RectF m_ThumbRect;
  CFX_RectF m_MinBtnRect;
  CFX_RectF m_MaxBtnRect;
  CFX_RectF m_MinTrackRect;
  CFX_RectF m_MaxTrackRect;
  std::unique_ptr<CFX_Timer> m_pTimer;
};

#endif  // XFA_FWL_CFWL_SCROLLBAR_H_
