// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_SCROLLBARTP_H_
#define XFA_FWL_THEME_CFWL_SCROLLBARTP_H_

#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_ScrollBarTP : public CFWL_WidgetTP {
 public:
  CFWL_ScrollBarTP();
  virtual ~CFWL_ScrollBarTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual uint32_t SetThemeID(IFWL_Widget* pWidget,
                              uint32_t dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, uint32_t dwCapacity);

 protected:
  struct SBThemeData {
    FX_ARGB clrPawColorLight[4];
    FX_ARGB clrPawColorDark[4];
    FX_ARGB clrBtnBK[4][2];
    FX_ARGB clrBtnBorder[4];
    FX_ARGB clrTrackBKStart;
    FX_ARGB clrTrackBKEnd;
  };

  void DrawThumbBtn(CFX_Graphics* pGraphics,
                    const CFX_RectF* pRect,
                    FX_BOOL bVert,
                    FWLTHEME_STATE eState,
                    FX_BOOL bPawButton = TRUE,
                    CFX_Matrix* pMatrix = NULL);
  void DrawTrack(CFX_Graphics* pGraphics,
                 const CFX_RectF* pRect,
                 FX_BOOL bVert,
                 FWLTHEME_STATE eState,
                 FX_BOOL bLowerTrack,
                 CFX_Matrix* pMatrix = NULL);
  void DrawMaxMinBtn(CFX_Graphics* pGraphics,
                     const CFX_RectF* pRect,
                     FWLTHEME_DIRECTION eDict,
                     FWLTHEME_STATE eState,
                     CFX_Matrix* pMatrix = NULL);
  void DrawPaw(CFX_Graphics* pGraphics,
               const CFX_RectF* pRect,
               FX_BOOL bVert,
               FWLTHEME_STATE eState,
               CFX_Matrix* pMatrix = NULL);
  void SetThemeData(uint32_t dwID);

  struct SBThemeData* m_pThemeData;
};

#endif  // XFA_FWL_THEME_CFWL_SCROLLBARTP_H_
