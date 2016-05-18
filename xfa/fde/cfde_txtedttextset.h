// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTTEXTSET_H_
#define XFA_FDE_CFDE_TXTEDTTEXTSET_H_

#include "xfa/fde/fde_visualset.h"

class CFDE_TxtEdtPage;

class CFDE_TxtEdtTextSet : public IFDE_TextSet {
 public:
  explicit CFDE_TxtEdtTextSet(CFDE_TxtEdtPage* pPage);
  ~CFDE_TxtEdtTextSet() override;

  FDE_VISUALOBJTYPE GetType() override;
  FX_BOOL GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF& bbox) override;
  FX_BOOL GetMatrix(FDE_HVISUALOBJ hVisualObj, CFX_Matrix& matrix) override;
  FX_BOOL GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) override;
  FX_BOOL GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) override;
  int32_t GetString(FDE_HVISUALOBJ hText, CFX_WideString& wsText) override;
  IFX_Font* GetFont(FDE_HVISUALOBJ hText) override;
  FX_FLOAT GetFontSize(FDE_HVISUALOBJ hText) override;
  FX_ARGB GetFontColor(FDE_HVISUALOBJ hText) override;
  int32_t GetDisplayPos(FDE_HVISUALOBJ hText,
                        FXTEXT_CHARPOS* pCharPos,
                        FX_BOOL bCharCode = FALSE,
                        CFX_WideString* pWSForms = NULL) override;
  int32_t GetCharRects(FDE_HVISUALOBJ hText, CFX_RectFArray& rtArray) override;
  int32_t GetCharRects_Impl(FDE_HVISUALOBJ hText,
                            CFX_RectFArray& rtArray,
                            FX_BOOL bBBox = FALSE);

 private:
  CFDE_TxtEdtPage* const m_pPage;
};

#endif  // XFA_FDE_CFDE_TXTEDTTEXTSET_H_
