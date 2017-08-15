// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTTEXTSET_H_
#define XFA_FDE_CFDE_TXTEDTTEXTSET_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fde/cfde_txtedtpage.h"
#include "xfa/fgas/font/cfgas_gefont.h"

class CFDE_TxtEdtTextSet {
 public:
  explicit CFDE_TxtEdtTextSet(CFDE_TxtEdtPage* pPage);
  ~CFDE_TxtEdtTextSet();

  CFX_RectF GetRect(const FDE_TEXTEDITPIECE& hVisualObj) const;
  int32_t GetString(FDE_TEXTEDITPIECE* pPiece, CFX_WideString& wsText) const;
  CFX_RetainPtr<CFGAS_GEFont> GetFont() const;
  float GetFontSize() const;
  FX_ARGB GetFontColor() const;
  int32_t GetDisplayPos(const FDE_TEXTEDITPIECE& pPiece,
                        FXTEXT_CHARPOS* pCharPos) const;
  std::vector<CFX_RectF> GetCharRects(const FDE_TEXTEDITPIECE* pPiece,
                                      bool bBBox) const;

 private:
  CFX_UnownedPtr<CFDE_TxtEdtPage> const m_pPage;
};

#endif  // XFA_FDE_CFDE_TXTEDTTEXTSET_H_
