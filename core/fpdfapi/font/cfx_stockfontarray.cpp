// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cfx_stockfontarray.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/fx_memory.h"

CFX_StockFontArray::CFX_StockFontArray() = default;

CFX_StockFontArray::~CFX_StockFontArray() {
  for (size_t i = 0; i < FX_ArraySize(m_StockFonts); ++i) {
    if (m_StockFonts[i]) {
      RetainPtr<CPDF_Dictionary> destroy(m_StockFonts[i]->GetFontDict());
      m_StockFonts[i]->ClearFontDict();
    }
  }
}

RetainPtr<CPDF_Font> CFX_StockFontArray::GetFont(
    CFX_FontMapper::StandardFont index) const {
  if (index >= FX_ArraySize(m_StockFonts))
    return nullptr;
  return m_StockFonts[index];
}

void CFX_StockFontArray::SetFont(CFX_FontMapper::StandardFont index,
                                 const RetainPtr<CPDF_Font>& pFont) {
  if (index < FX_ArraySize(m_StockFonts))
    m_StockFonts[index] = pFont;
}
