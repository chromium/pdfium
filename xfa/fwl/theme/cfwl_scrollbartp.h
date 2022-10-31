// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_SCROLLBARTP_H_
#define XFA_FWL_THEME_CFWL_SCROLLBARTP_H_

#include <memory>

#include "fxjs/gc/heap.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_ScrollBarTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_ScrollBarTP() override;

  // CFWL_WidgetTP:
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;

 private:
  struct SBThemeData {
    FX_ARGB clrTrackBKStart;
    FX_ARGB clrTrackBKEnd;
    FX_ARGB clrBtnBK[4][2];
    FX_ARGB clrBtnBorder[4];
  };

  CFWL_ScrollBarTP();

  void DrawThumbBtn(CFGAS_GEGraphics* pGraphics,
                    const CFX_RectF& rect,
                    bool bVert,
                    FWLTHEME_STATE eState,
                    const CFX_Matrix& matrix);
  void DrawTrack(CFGAS_GEGraphics* pGraphics,
                 const CFX_RectF& rect,
                 bool bVert,
                 FWLTHEME_STATE eState,
                 bool bLowerTrack,
                 const CFX_Matrix& matrix);
  void DrawMaxMinBtn(CFGAS_GEGraphics* pGraphics,
                     const CFX_RectF& rect,
                     FWLTHEME_DIRECTION eDict,
                     FWLTHEME_STATE eState,
                     const CFX_Matrix& matrix);
  void SetThemeData();

  std::unique_ptr<SBThemeData> m_pThemeData;
};

#endif  // XFA_FWL_THEME_CFWL_SCROLLBARTP_H_
