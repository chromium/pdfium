// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_cbbutton.h"

#include <utility>

#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"

namespace {

constexpr float kComboBoxTriangleHalfLength = 3.0f;
constexpr float kComboBoxTriangleQuarterLength =
    kComboBoxTriangleHalfLength * 0.5;

}  // namespace

CPWL_CBButton::CPWL_CBButton(
    const CreateParams& cp,
    std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData)
    : CPWL_Wnd(cp, std::move(pAttachedData)) {}

CPWL_CBButton::~CPWL_CBButton() = default;

void CPWL_CBButton::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                       const CFX_Matrix& mtUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, mtUser2Device);

  CFX_FloatRect rectWnd = CPWL_Wnd::GetWindowRect();
  if (!IsVisible() || rectWnd.IsEmpty())
    return;

  CFX_PointF ptCenter = GetCenterPoint();

  CFX_PointF pt1(ptCenter.x - kComboBoxTriangleHalfLength,
                 ptCenter.y + kComboBoxTriangleQuarterLength);
  CFX_PointF pt2(ptCenter.x + kComboBoxTriangleHalfLength,
                 ptCenter.y + kComboBoxTriangleQuarterLength);
  CFX_PointF pt3(ptCenter.x, ptCenter.y - kComboBoxTriangleQuarterLength);

  if (IsFloatBigger(rectWnd.right - rectWnd.left,
                    kComboBoxTriangleHalfLength * 2) &&
      IsFloatBigger(rectWnd.top - rectWnd.bottom,
                    kComboBoxTriangleHalfLength)) {
    CFX_PathData path;
    path.AppendPoint(pt1, FXPT_TYPE::MoveTo);
    path.AppendPoint(pt2, FXPT_TYPE::LineTo);
    path.AppendPoint(pt3, FXPT_TYPE::LineTo);
    path.AppendPoint(pt1, FXPT_TYPE::LineTo);

    pDevice->DrawPath(&path, &mtUser2Device, nullptr,
                      PWL_DEFAULT_BLACKCOLOR.ToFXColor(GetTransparency()), 0,
                      CFX_FillRenderOptions::EvenOddOptions());
  }
}

bool CPWL_CBButton::OnLButtonDown(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDown(nFlag, point);

  SetCapture();
  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyLButtonDown(this, point);

  return true;
}

bool CPWL_CBButton::OnLButtonUp(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonUp(nFlag, point);

  ReleaseCapture();
  return true;
}
