// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_CHECKBOXTP_H_
#define XFA_FWL_THEME_CFWL_CHECKBOXTP_H_

#include <memory>

#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_Widget;

class CFWL_CheckBoxTP final : public CFWL_WidgetTP {
 public:
  CFWL_CheckBoxTP();
  ~CFWL_CheckBoxTP() override;

  // CFWL_WidgetTP
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;
  void DrawText(const CFWL_ThemeText& pParams) override;

 private:
  void DrawCheckSign(CFWL_Widget* pWidget,
                     CXFA_Graphics* pGraphics,
                     const CFX_RectF& pRtBox,
                     int32_t iState,
                     const CFX_Matrix& matrix);
  void DrawSignCheck(CXFA_Graphics* pGraphics,
                     const CFX_RectF& rtSign,
                     FX_ARGB argbFill,
                     const CFX_Matrix& matrix);
  void DrawSignCircle(CXFA_Graphics* pGraphics,
                      const CFX_RectF& rtSign,
                      FX_ARGB argbFill,
                      const CFX_Matrix& matrix);
  void DrawSignCross(CXFA_Graphics* pGraphics,
                     const CFX_RectF& rtSign,
                     FX_ARGB argbFill,
                     const CFX_Matrix& matrix);
  void DrawSignDiamond(CXFA_Graphics* pGraphics,
                       const CFX_RectF& rtSign,
                       FX_ARGB argbFill,
                       const CFX_Matrix& matrix);
  void DrawSignSquare(CXFA_Graphics* pGraphics,
                      const CFX_RectF& rtSign,
                      FX_ARGB argbFill,
                      const CFX_Matrix& matrix);
  void DrawSignStar(CXFA_Graphics* pGraphics,
                    const CFX_RectF& rtSign,
                    FX_ARGB argbFill,
                    const CFX_Matrix& matrix);

  void InitCheckPath(float fCheckLen);

  std::unique_ptr<CXFA_GEPath> m_pCheckPath;
};

#endif  // XFA_FWL_THEME_CFWL_CHECKBOXTP_H_
