// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_FORMTP_H_
#define XFA_FWL_THEME_CFWL_FORMTP_H_

#include "xfa/fwl/theme/cfwl_utils.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_FormTP : public CFWL_WidgetTP {
 public:
  CFWL_FormTP();
  virtual ~CFWL_FormTP();

  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual uint32_t SetThemeID(IFWL_Widget* pWidget,
                              uint32_t dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual FX_BOOL DrawText(CFWL_ThemeText* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, uint32_t dwCapacity);
  virtual FWL_ERR GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF& rtPart);

 protected:
  void CalCloseBox(IFWL_Widget* pWidget, CFX_RectF& rect);
  void CalMaxBox(IFWL_Widget* pWidget, CFX_RectF& rect);
  void CalMinBox(IFWL_Widget* pWidget, CFX_RectF& rect);
  void CalCaption(IFWL_Widget* pWidget, CFX_RectF& rect);
  void CalIcon(IFWL_Widget* pWidget, CFX_RectF& rect);

  void DrawFormBorder(CFX_Graphics* pGraphics,
                      const CFX_RectF* pRect,
                      FWLTHEME_STATE eState,
                      CFX_Matrix* pMatrix,
                      int32_t iActive = 0);
  void DrawCaption(CFX_Graphics* pGraphics,
                   const CFX_RectF* pRect,
                   FWLTHEME_STATE eState,
                   CFX_Matrix* pMatrix,
                   int32_t iActive = 0);
  void DrawNarrowCaption(CFX_Graphics* pGraphics,
                         const CFX_RectF* pRect,
                         FWLTHEME_STATE eState,
                         CFX_Matrix* pMatrix,
                         int32_t iActive = 0);
  void DrawCloseBox(CFX_Graphics* pGraphics,
                    const CFX_RectF* pRect,
                    FWLTHEME_STATE eState,
                    CFX_Matrix* pMatrix,
                    int32_t iActive = 0);
  void DrawMinMaxBoxCommon(CFX_Graphics* pGraphics,
                           const CFX_RectF* pRect,
                           FWLTHEME_STATE eState,
                           CFX_Matrix* pMatrix,
                           int32_t iActive = 0);
  void DrawMinimizeBox(CFX_Graphics* pGraphics,
                       const CFX_RectF* pRect,
                       FWLTHEME_STATE eState,
                       CFX_Matrix* pMatrix,
                       int32_t iActive = 0);
  void DrawMaximizeBox(CFX_Graphics* pGraphics,
                       const CFX_RectF* pRect,
                       FWLTHEME_STATE eState,
                       FX_BOOL bMax,
                       CFX_Matrix* pMatrix,
                       int32_t iActive = 0);
  void DrawIconImage(CFX_Graphics* pGraphics,
                     CFX_DIBitmap* pDIBitmap,
                     const CFX_RectF* pRect,
                     FWLTHEME_STATE eState,
                     CFX_Matrix* pMatrix,
                     int32_t iActive = 0);
  void SetThemeData(uint32_t dwID);
  void TransModeColor(FX_ARGB clrFore, FX_ARGB& clrBack);
  void DeactiveForm();
  void InitCaption(FX_BOOL bActive);
  CFX_DIBitmap* m_pActiveBitmap;
  CFX_DIBitmap* m_pDeactivebitmap;
  CFX_RectF m_rtDisCaption;
  CFX_RectF m_rtDisLBorder;
  CFX_RectF m_rtDisRBorder;
  CFX_RectF m_rtDisBBorder;
  struct SBThemeData {
    FX_ARGB clrHeadBK[2][4];
    FX_ARGB clrHeadEdgeLeft[2][3];
    FX_ARGB clrHeadEdgeRight[2][3];
    FX_ARGB clrHeadEdgeTop[2][3];
    FX_ARGB clrHeadEdgeBottom[2][3];
    FX_ARGB clrCloseBtBKStart[2][3];
    FX_ARGB clrCloseBtBKEnd[2][3];
    FX_ARGB clrCloseBtEdgeLight[2][3];
    FX_ARGB clrCloseBtEdgeDark[2][3];
    FX_ARGB clrNormalBtBKStart[2][3];
    FX_ARGB clrNormalBtBKEnd[2][3];
    FX_ARGB clrNormalBtEdgeLight[2][3];
    FX_ARGB clrNormalBtEdgeDark[2][3];
    FX_ARGB clrBtnEdgeOut[2];
    FX_ARGB clrBtnCornerLight[2][3];
    FX_ARGB clrHeadText[2];
    FX_ARGB clrFormBorder[2][5];
    FX_ARGB clrFormBorderLight[2];
    FX_ARGB clrTransWhite;
  } * m_pThemeData;
};

#endif  // XFA_FWL_THEME_CFWL_FORMTP_H_
