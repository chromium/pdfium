// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_SCROLLBAR_H_
#define XFA_FWL_CFWL_SCROLLBAR_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/cfx_timer.h"
#include "core/fxcrt/check.h"
#include "xfa/fwl/cfwl_eventscroll.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_widget.h"

namespace pdfium {

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
    *fMin = range_min_;
    *fMax = range_max_;
  }
  void SetRange(float fMin, float fMax) {
    range_min_ = fMin;
    range_max_ = fMax;
  }
  float GetPageSize() const { return page_size_; }
  void SetPageSize(float fPageSize) { page_size_ = fPageSize; }
  float GetStepSize() const { return step_size_; }
  void SetStepSize(float fStepSize) { step_size_ = fStepSize; }
  float GetPos() const { return pos_; }
  void SetPos(float fPos) { pos_ = fPos; }
  void SetTrackPos(float fTrackPos);

 private:
  CFWL_ScrollBar(CFWL_App* app,
                 const Properties& properties,
                 CFWL_Widget* pOuter);

  bool IsVertical() const { return !!(GetStyleExts() & FWL_STYLEEXT_SCB_Vert); }
  void DrawUpperTrack(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawLowerTrack(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawMaxArrowBtn(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawMinArrowBtn(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
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
                   CFWL_PartState* pState,
                   const CFX_PointF& point);
  void DoMouseUp(int32_t iItem,
                 const CFX_RectF& rtItem,
                 CFWL_PartState* pState,
                 const CFX_PointF& point);
  void DoMouseMove(int32_t iItem,
                   const CFX_RectF& rtItem,
                   CFWL_PartState* pState,
                   const CFX_PointF& point);
  void DoMouseLeave(int32_t iItem,
                    const CFX_RectF& rtItem,
                    CFWL_PartState* pState);
  void DoMouseHover(int32_t iItem,
                    const CFX_RectF& rtItem,
                    CFWL_PartState* pState);

  float range_min_ = 0.0f;
  float range_max_ = -1.0f;
  float page_size_ = 0.0f;
  float step_size_ = 0.0f;
  float pos_ = 0.0f;
  float track_pos_ = 0.0f;
  CFWL_PartState min_button_state_ = CFWL_PartState::kNormal;
  CFWL_PartState max_button_state_ = CFWL_PartState::kNormal;
  CFWL_PartState thumb_button_state_ = CFWL_PartState::kNormal;
  CFWL_PartState min_track_state_ = CFWL_PartState::kNormal;
  CFWL_PartState max_track_state_ = CFWL_PartState::kNormal;
  float last_track_pos_ = 0.0f;
  CFX_PointF track_point_;
  int32_t mouse_wheel_ = 0;
  float button_len_ = 0.0f;
  bool mouse_down_ = false;
  bool min_size_ = false;
  CFX_RectF client_rect_;
  CFX_RectF thumb_rect_;
  CFX_RectF min_btn_rect_;
  CFX_RectF max_btn_rect_;
  CFX_RectF min_track_rect_;
  CFX_RectF max_track_rect_;
  std::unique_ptr<CFX_Timer> timer_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_ScrollBar;

#endif  // XFA_FWL_CFWL_SCROLLBAR_H_
