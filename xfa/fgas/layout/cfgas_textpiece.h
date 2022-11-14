// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_TEXTPIECE_H_
#define XFA_FGAS_LAYOUT_CFGAS_TEXTPIECE_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/widestring.h"

class CFGAS_GEFont;

class CFGAS_TextPiece {
 public:
  CFGAS_TextPiece();
  ~CFGAS_TextPiece();

  WideString szText;
  std::vector<int32_t> Widths;
  int32_t iChars = 0;
  int32_t iHorScale = 0;
  int32_t iVerScale = 0;
  int32_t iBidiLevel = 0;
  float fFontSize = 0.0f;
  CFX_RectF rtPiece;
  RetainPtr<CFGAS_GEFont> pFont;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_TEXTPIECE_H_
