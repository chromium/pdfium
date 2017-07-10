// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_FX_TEXT_INT_H_
#define CORE_FXGE_FX_TEXT_INT_H_

#include <map>
#include <memory>

#include "core/fxge/fx_font.h"
#include "core/fxge/fx_freetype.h"

struct CFX_UniqueKeyGen {
  void Generate(int count, ...);

  char m_Key[128];
  int m_KeyLen;
};

class CFX_SizeGlyphCache {
 public:
  CFX_SizeGlyphCache();
  ~CFX_SizeGlyphCache();

  std::map<uint32_t, std::unique_ptr<CFX_GlyphBitmap>> m_GlyphMap;
};

#endif  // CORE_FXGE_FX_TEXT_INT_H_
