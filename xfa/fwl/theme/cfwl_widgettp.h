// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_WIDGETTP_H_
#define XFA_FWL_THEME_CFWL_WIDGETTP_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "xfa/fwl/theme/cfwl_utils.h"

class CFDE_TextOut;
class CFGAS_GEFont;
class CFGAS_GEGraphics;
class CFWL_ThemeBackground;
class CFWL_ThemeText;

class CFWL_WidgetTP : public cppgc::GarbageCollected<CFWL_WidgetTP> {
 public:
  virtual ~CFWL_WidgetTP();

  virtual void DrawBackground(const CFWL_ThemeBackground& pParams);
  virtual void DrawText(const CFWL_ThemeText& pParams);

  const RetainPtr<CFGAS_GEFont>& GetFont() const;

  // Non-virtual, nothing to trace in subclasses at present.
  void Trace(cppgc::Visitor* visitor) const;

 protected:
  struct CColorData {
    FX_ARGB clrBorder[4];  // Indexed by enum FWLTHEME_STATE - 1.
    FX_ARGB clrStart[4];   // Indexed by enum FWLTHEME_STATE - 1.
    FX_ARGB clrEnd[4];     // Indexed by enum FWLTHEME_STATE - 1.
    FX_ARGB clrSign[4];    // Indexed by enum FWLTHEME_STATE - 1.
  };

  CFWL_WidgetTP();

  void InitializeArrowColorData();
  void EnsureTTOInitialized();

  void DrawBorder(CFGAS_GEGraphics* pGraphics,
                  const CFX_RectF& rect,
                  const CFX_Matrix& matrix);
  void FillBackground(CFGAS_GEGraphics* pGraphics,
                      const CFX_RectF& rect,
                      const CFX_Matrix& matrix);
  void FillSolidRect(CFGAS_GEGraphics* pGraphics,
                     FX_ARGB fillColor,
                     const CFX_RectF& rect,
                     const CFX_Matrix& matrix);
  void DrawFocus(CFGAS_GEGraphics* pGraphics,
                 const CFX_RectF& rect,
                 const CFX_Matrix& matrix);
  void DrawArrow(CFGAS_GEGraphics* pGraphics,
                 const CFX_RectF& rect,
                 FWLTHEME_DIRECTION eDict,
                 FX_ARGB argSign,
                 const CFX_Matrix& matrix);
  void DrawBtn(CFGAS_GEGraphics* pGraphics,
               const CFX_RectF& rect,
               FWLTHEME_STATE eState,
               const CFX_Matrix& matrix);
  void DrawArrowBtn(CFGAS_GEGraphics* pGraphics,
                    const CFX_RectF& rect,
                    FWLTHEME_DIRECTION eDict,
                    FWLTHEME_STATE eState,
                    const CFX_Matrix& matrix);

  std::unique_ptr<CFDE_TextOut> m_pTextOut;
  RetainPtr<CFGAS_GEFont> m_pFGASFont;
  std::unique_ptr<CColorData> m_pColorData;
};

#endif  // XFA_FWL_THEME_CFWL_WIDGETTP_H_
