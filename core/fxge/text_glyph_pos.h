// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_TEXT_GLYPH_POS_H_
#define CORE_FXGE_TEXT_GLYPH_POS_H_

#include "core/fxcrt/fx_coordinates.h"

class CFX_GlyphBitmap;

class TextGlyphPos {
 public:
  TextGlyphPos();
  TextGlyphPos(const TextGlyphPos&);
  ~TextGlyphPos();

  const CFX_GlyphBitmap* m_pGlyph = nullptr;
  CFX_Point m_Origin;
  CFX_PointF m_fOrigin;
};

#endif  // CORE_FXGE_TEXT_GLYPH_POS_H_
