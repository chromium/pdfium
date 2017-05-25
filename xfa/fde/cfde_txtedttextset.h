// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTTEXTSET_H_
#define XFA_FDE_CFDE_TXTEDTTEXTSET_H_

#include <vector>

#include "xfa/fde/ifde_visualset.h"

class CFDE_TxtEdtPage;

class CFDE_TxtEdtTextSet : public IFDE_VisualSet {
 public:
  explicit CFDE_TxtEdtTextSet(CFDE_TxtEdtPage* pPage);
  ~CFDE_TxtEdtTextSet() override;

  // IFDE_VisualSet
  FDE_VISUALOBJTYPE GetType() override;
  CFX_RectF GetRect(const FDE_TEXTEDITPIECE& hVisualObj) override;

  int32_t GetString(FDE_TEXTEDITPIECE* pPiece, CFX_WideString& wsText);
  CFX_RetainPtr<CFGAS_GEFont> GetFont();
  float GetFontSize();
  FX_ARGB GetFontColor();
  int32_t GetDisplayPos(const FDE_TEXTEDITPIECE& pPiece,
                        FXTEXT_CHARPOS* pCharPos,
                        bool bCharCode = false,
                        CFX_WideString* pWSForms = nullptr);
  std::vector<CFX_RectF> GetCharRects(const FDE_TEXTEDITPIECE* pPiece,
                                      bool bBBox);

 private:
  CFX_UnownedPtr<CFDE_TxtEdtPage> const m_pPage;
};

#endif  // XFA_FDE_CFDE_TXTEDTTEXTSET_H_
