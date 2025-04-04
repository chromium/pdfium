// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_CHECKBOXTP_H_
#define XFA_FWL_THEME_CFWL_CHECKBOXTP_H_

#include <memory>

#include "fxjs/gc/heap.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFGAS_GEPath;

namespace pdfium {

class CFWL_Widget;

class CFWL_CheckBoxTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_CheckBoxTP() override;

  // CFWL_WidgetTP
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;
  void DrawText(const CFWL_ThemeText& pParams) override;

 private:
  CFWL_CheckBoxTP();

  void DrawCheckSign(CFWL_Widget* pWidget,
                     CFGAS_GEGraphics* pGraphics,
                     const CFX_RectF& pRtBox,
                     Mask<CFWL_PartState> iState,
                     const CFX_Matrix& matrix);
  void DrawSignCheck(CFGAS_GEGraphics* pGraphics,
                     const CFX_RectF& rtSign,
                     FX_ARGB argbFill,
                     const CFX_Matrix& matrix);
  void DrawSignCircle(CFGAS_GEGraphics* pGraphics,
                      const CFX_RectF& rtSign,
                      FX_ARGB argbFill,
                      const CFX_Matrix& matrix);
  void DrawSignCross(CFGAS_GEGraphics* pGraphics,
                     const CFX_RectF& rtSign,
                     FX_ARGB argbFill,
                     const CFX_Matrix& matrix);
  void DrawSignDiamond(CFGAS_GEGraphics* pGraphics,
                       const CFX_RectF& rtSign,
                       FX_ARGB argbFill,
                       const CFX_Matrix& matrix);
  void DrawSignSquare(CFGAS_GEGraphics* pGraphics,
                      const CFX_RectF& rtSign,
                      FX_ARGB argbFill,
                      const CFX_Matrix& matrix);
  void DrawSignStar(CFGAS_GEGraphics* pGraphics,
                    const CFX_RectF& rtSign,
                    FX_ARGB argbFill,
                    const CFX_Matrix& matrix);

  void EnsureCheckPathInitialized(float fCheckLen);

  std::unique_ptr<CFGAS_GEPath> check_path_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_CheckBoxTP;

#endif  // XFA_FWL_THEME_CFWL_CHECKBOXTP_H_
