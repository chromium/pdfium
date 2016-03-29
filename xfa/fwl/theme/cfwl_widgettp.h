// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_WIDGETTP_H_
#define XFA_FWL_THEME_CFWL_WIDGETTP_H_

#include <memory>
#include <vector>

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fxgraphics/include/cfx_graphics.h"
#include "xfa/fwl/theme/cfwl_utils.h"

class IFWL_Widget;
class IFDE_TextOut;
class IFX_Font;
class IFX_FontMgr;
class CFWL_ArrowData;
class CFWL_ThemeBackground;
class CFWL_ThemePart;
class CFWL_ThemeText;

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
class IFX_FontSourceEnum;
#endif

class CFWL_WidgetTP {
 public:
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual uint32_t GetThemeID(IFWL_Widget* pWidget);
  virtual uint32_t SetThemeID(IFWL_Widget* pWidget,
                              uint32_t dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FWL_ERR GetThemeMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix);
  virtual FWL_ERR SetThemeMatrix(IFWL_Widget* pWidget,
                                 const CFX_Matrix& matrix);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual FX_BOOL DrawText(CFWL_ThemeText* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, uint32_t dwCapacity);
  virtual FX_BOOL IsCustomizedLayout(IFWL_Widget* pWidget);
  virtual FWL_ERR GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF& rtPart);
  virtual FX_BOOL IsInPart(CFWL_ThemePart* pThemePart,
                           FX_FLOAT fx,
                           FX_FLOAT fy);
  virtual FX_BOOL CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual ~CFWL_WidgetTP();
  FWL_ERR SetFont(IFWL_Widget* pWidget,
                  const FX_WCHAR* strFont,
                  FX_FLOAT fFontSize,
                  FX_ARGB rgbFont);
  FWL_ERR SetFont(IFWL_Widget* pWidget,
                  IFX_Font* pFont,
                  FX_FLOAT fFontSize,
                  FX_ARGB rgbFont);
  IFX_Font* GetFont(IFWL_Widget* pWidget);

 protected:
  CFWL_WidgetTP();
  FX_ERR InitTTO();
  FX_ERR FinalizeTTO();
  void DrawEdge(CFX_Graphics* pGraphics,
                uint32_t dwStyles,
                const CFX_RectF* pRect,
                CFX_Matrix* pMatrix = NULL);
  void Draw3DRect(CFX_Graphics* pGraphics,
                  FWLTHEME_EDGE eType,
                  FX_FLOAT fWidth,
                  const CFX_RectF* pRect,
                  FX_ARGB cr1,
                  FX_ARGB cr2,
                  FX_ARGB cr3,
                  FX_ARGB cr4,
                  CFX_Matrix* pMatrix = NULL);
  void Draw3DCircle(CFX_Graphics* pGraphics,
                    FWLTHEME_EDGE eType,
                    FX_FLOAT fWidth,
                    const CFX_RectF* pRect,
                    FX_ARGB cr1,
                    FX_ARGB cr2,
                    FX_ARGB cr3,
                    FX_ARGB cr4,
                    CFX_Matrix* pMatrix = NULL);
  void DrawBorder(CFX_Graphics* pGraphics,
                  const CFX_RectF* pRect,
                  CFX_Matrix* pMatrix = NULL);
  void FillBackground(CFX_Graphics* pGraphics,
                      const CFX_RectF* pRect,
                      CFX_Matrix* pMatrix = NULL);
  void FillSoildRect(CFX_Graphics* pGraphics,
                     FX_ARGB fillColor,
                     const CFX_RectF* pRect,
                     CFX_Matrix* pMatrix = NULL);
  void DrawAxialShading(CFX_Graphics* pGraphics,
                        FX_FLOAT fx1,
                        FX_FLOAT fy1,
                        FX_FLOAT fx2,
                        FX_FLOAT fy2,
                        FX_ARGB beginColor,
                        FX_ARGB endColor,
                        CFX_Path* path,
                        int32_t fillMode = FXFILL_WINDING,
                        CFX_Matrix* pMatrix = NULL);
  void DrawAnnulusRect(CFX_Graphics* pGraphics,
                       FX_ARGB fillColor,
                       const CFX_RectF* pRect,
                       FX_FLOAT fRingWidth = 1,
                       CFX_Matrix* pMatrix = NULL);
  void DrawAnnulusCircle(CFX_Graphics* pGraphics,
                         FX_ARGB fillColor,
                         const CFX_RectF* pRect,
                         FX_FLOAT fWidth = 1,
                         CFX_Matrix* pMatrix = NULL);
  void DrawFocus(CFX_Graphics* pGraphics,
                 const CFX_RectF* pRect,
                 CFX_Matrix* pMatrix = NULL);
  void DrawArrow(CFX_Graphics* pGraphics,
                 const CFX_RectF* pRect,
                 FWLTHEME_DIRECTION eDict,
                 FX_ARGB argbFill,
                 FX_BOOL bPressed,
                 CFX_Matrix* pMatrix = NULL);
  void DrawArrow(CFX_Graphics* pGraphics,
                 const CFX_RectF* pRect,
                 FWLTHEME_DIRECTION eDict,
                 FX_ARGB argSign,
                 CFX_Matrix* pMatrix = NULL);
  void DrawBtn(CFX_Graphics* pGraphics,
               const CFX_RectF* pRect,
               FWLTHEME_STATE eState,
               CFX_Matrix* pMatrix = NULL);
  void DrawArrowBtn(CFX_Graphics* pGraphics,
                    const CFX_RectF* pRect,
                    FWLTHEME_DIRECTION eDict,
                    FWLTHEME_STATE eState,
                    CFX_Matrix* pMatrix = NULL);
  FWLCOLOR BlendColor(FWLCOLOR srcColor, FWLCOLOR renderColor, uint8_t scale);
  uint32_t m_dwRefCount;
  IFDE_TextOut* m_pTextOut;
  IFX_Font* m_pFDEFont;
  FX_FLOAT m_fValue;
  uint32_t m_dwValue;
  CFX_RectF m_rtMargin;
  uint32_t m_dwThemeID;
  CFX_Matrix _ctm;
};
FX_BOOL FWLTHEME_Init();
void FWLTHEME_Release();
uint32_t FWL_GetThemeLayout(uint32_t dwThemeID);
uint32_t FWL_GetThemeColor(uint32_t dwThemeID);
uint32_t FWL_MakeThemeID(uint32_t dwLayout, uint32_t dwColor);

class CFWL_ArrowData {
 public:
  static CFWL_ArrowData* GetInstance();
  static FX_BOOL IsInstance();
  static void DestroyInstance();
  virtual ~CFWL_ArrowData();
  void SetColorData(uint32_t dwID);

  class CColorData {
   public:
    FX_ARGB clrBorder[4];
    FX_ARGB clrStart[4];
    FX_ARGB clrEnd[4];
    FX_ARGB clrSign[4];
  } * m_pColorData;

 protected:
  CFWL_ArrowData();
  static CFWL_ArrowData* m_pInstance;
};

class CFWL_FontData {
 public:
  CFWL_FontData();
  virtual ~CFWL_FontData();
  FX_BOOL Equal(const CFX_WideStringC& wsFontFamily,
                uint32_t dwFontStyles,
                uint16_t wCodePage);
  FX_BOOL LoadFont(const CFX_WideStringC& wsFontFamily,
                   uint32_t dwFontStyles,
                   uint16_t wCodePage);
  IFX_Font* GetFont() const { return m_pFont; }

 protected:
  CFX_WideString m_wsFamily;
  uint32_t m_dwStyles;
  uint32_t m_dwCodePage;
  IFX_Font* m_pFont;
  IFX_FontMgr* m_pFontMgr;
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  IFX_FontSourceEnum* m_pFontSource;
#endif
};

class CFWL_FontManager {
 public:
  static CFWL_FontManager* GetInstance();
  static void DestroyInstance();

  IFX_Font* FindFont(const CFX_WideStringC& wsFontFamily,
                     uint32_t dwFontStyles,
                     uint16_t dwCodePage);

 protected:
  CFWL_FontManager();
  virtual ~CFWL_FontManager();

  static CFWL_FontManager* s_FontManager;
  std::vector<std::unique_ptr<CFWL_FontData>> m_FontsArray;
};

#endif  // XFA_FWL_THEME_CFWL_WIDGETTP_H_
