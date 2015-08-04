// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_SCROLLBARTP_H
#define _FWL_SCROLLBARTP_H
class CFWL_WidgetTP;
class CFWL_ScrollBarTP;
class CFWL_ScrollBarTP : public CFWL_WidgetTP {
 public:
  CFWL_ScrollBarTP();
  virtual ~CFWL_ScrollBarTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_DWORD SetThemeID(IFWL_Widget* pWidget,
                              FX_DWORD dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, FX_DWORD dwCapacity);

 protected:
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
  void SetThemeData(FX_DWORD dwID);
  struct SBThemeData {
    FX_ARGB clrPawColorLight[4];
    FX_ARGB clrPawColorDark[4];
    FX_ARGB clrBtnBK[4][2];
    FX_ARGB clrBtnBorder[4];
    FX_ARGB clrTrackBKStart;
    FX_ARGB clrTrackBKEnd;
  } * m_pThemeData;
};
#endif
