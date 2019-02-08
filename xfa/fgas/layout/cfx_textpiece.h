// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFX_TEXTPIECE_H_
#define XFA_FGAS_LAYOUT_CFX_TEXTPIECE_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"

class CFGAS_GEFont;

class CFX_TextPiece {
 public:
  CFX_TextPiece();
  ~CFX_TextPiece();

  WideString szText;
  std::vector<int32_t> Widths;
  int32_t iChars;
  int32_t iHorScale;
  int32_t iVerScale;
  int32_t iBidiLevel;
  float fFontSize;
  CFX_RectF rtPiece;
  RetainPtr<CFGAS_GEFont> pFont;
};

#endif  // XFA_FGAS_LAYOUT_CFX_TEXTPIECE_H_
