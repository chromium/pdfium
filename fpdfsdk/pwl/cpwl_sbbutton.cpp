// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_sbbutton.h"

#include <utility>
#include <vector>

#include "core/fxge/cfx_renderdevice.h"
#include "third_party/base/cxx17_backports.h"

CPWL_SBButton::CPWL_SBButton(
    const CreateParams& cp,
    std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData,
    Type eButtonType)
    : CPWL_Wnd(cp, std::move(pAttachedData)), m_eSBButtonType(eButtonType) {
  GetCreationParams()->eCursorType = IPWL_SystemHandler::CursorStyle::kArrow;
}

CPWL_SBButton::~CPWL_SBButton() = default;

void CPWL_SBButton::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                       const CFX_Matrix& mtUser2Device) {
  if (!IsVisible())
    return;

  CFX_FloatRect rectWnd = GetWindowRect();
  if (rectWnd.IsEmpty())
    return;

  CFX_PointF ptCenter = GetCenterPoint();
  int32_t nTransparency = GetTransparency();

  // draw border
  pDevice->DrawStrokeRect(mtUser2Device, rectWnd,
                          ArgbEncode(nTransparency, 100, 100, 100), 0.0f);
  pDevice->DrawStrokeRect(mtUser2Device, rectWnd.GetDeflated(0.5f, 0.5f),
                          ArgbEncode(nTransparency, 255, 255, 255), 1.0f);

  if (m_eSBButtonType != Type::kPosButton) {
    // draw background
    pDevice->DrawShadow(mtUser2Device, true, false,
                        rectWnd.GetDeflated(1.0f, 1.0f), nTransparency, 80,
                        220);
    // draw arrow
    if (rectWnd.top - rectWnd.bottom > 6.0f) {
      float fX = rectWnd.left + 1.5f;
      float fY = rectWnd.bottom;
      std::vector<CFX_PointF> pts;
      static constexpr float kOffsetsX[] = {2.5f, 2.5f, 4.5f, 6.5f,
                                            6.5f, 4.5f, 2.5f};
      static constexpr float kOffsetsY[] = {5.0f, 6.0f, 4.0f, 6.0f,
                                            5.0f, 3.0f, 5.0f};
      static constexpr float kOffsetsMinY[] = {4.0f, 3.0f, 5.0f, 3.0f,
                                               4.0f, 6.0f, 4.0f};
      static_assert(pdfium::size(kOffsetsX) == pdfium::size(kOffsetsY),
                    "Wrong offset count");
      static_assert(pdfium::size(kOffsetsX) == pdfium::size(kOffsetsMinY),
                    "Wrong offset count");
      const float* pOffsetsY =
          m_eSBButtonType == Type::kMinButton ? kOffsetsMinY : kOffsetsY;
      for (size_t i = 0; i < pdfium::size(kOffsetsX); ++i)
        pts.push_back(CFX_PointF(fX + kOffsetsX[i], fY + pOffsetsY[i]));
      pDevice->DrawFillArea(mtUser2Device, pts,
                            ArgbEncode(nTransparency, 255, 255, 255));
    }
    return;
  }

  // draw shadow effect
  CFX_PointF ptTop = CFX_PointF(rectWnd.left, rectWnd.top - 1.0f);
  CFX_PointF ptBottom = CFX_PointF(rectWnd.left, rectWnd.bottom + 1.0f);

  ptTop.x += 1.5f;
  ptBottom.x += 1.5f;

  const FX_COLORREF refs[] = {ArgbEncode(nTransparency, 210, 210, 210),
                              ArgbEncode(nTransparency, 220, 220, 220),
                              ArgbEncode(nTransparency, 240, 240, 240),
                              ArgbEncode(nTransparency, 240, 240, 240),
                              ArgbEncode(nTransparency, 210, 210, 210),
                              ArgbEncode(nTransparency, 180, 180, 180),
                              ArgbEncode(nTransparency, 150, 150, 150),
                              ArgbEncode(nTransparency, 150, 150, 150),
                              ArgbEncode(nTransparency, 180, 180, 180),
                              ArgbEncode(nTransparency, 210, 210, 210)};
  for (FX_COLORREF ref : refs) {
    pDevice->DrawStrokeLine(&mtUser2Device, ptTop, ptBottom, ref, 1.0f);

    ptTop.x += 1.0f;
    ptBottom.x += 1.0f;
  }

  // draw friction
  if (rectWnd.Height() <= 8.0f)
    return;

  FX_COLORREF crStroke = ArgbEncode(nTransparency, 120, 120, 120);
  float nFrictionWidth = 5.0f;
  float nFrictionHeight = 5.5f;
  CFX_PointF ptLeft = CFX_PointF(ptCenter.x - nFrictionWidth / 2.0f,
                                 ptCenter.y - nFrictionHeight / 2.0f + 0.5f);
  CFX_PointF ptRight = CFX_PointF(ptCenter.x + nFrictionWidth / 2.0f,
                                  ptCenter.y - nFrictionHeight / 2.0f + 0.5f);

  for (size_t i = 0; i < 3; ++i) {
    pDevice->DrawStrokeLine(&mtUser2Device, ptLeft, ptRight, crStroke, 1.0f);
    ptLeft.y += 2.0f;
    ptRight.y += 2.0f;
  }
}

bool CPWL_SBButton::OnLButtonDown(FWL_EventFlagMask nFlag,
                                  const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDown(nFlag, point);

  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyLButtonDown(this, point);

  m_bMouseDown = true;
  SetCapture();

  return true;
}

bool CPWL_SBButton::OnLButtonUp(FWL_EventFlagMask nFlag,
                                const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonUp(nFlag, point);

  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyLButtonUp(this, point);

  m_bMouseDown = false;
  ReleaseCapture();

  return true;
}

bool CPWL_SBButton::OnMouseMove(FWL_EventFlagMask nFlag,
                                const CFX_PointF& point) {
  CPWL_Wnd::OnMouseMove(nFlag, point);

  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyMouseMove(this, point);

  return true;
}
