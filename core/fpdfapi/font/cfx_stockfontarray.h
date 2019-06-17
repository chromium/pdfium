// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CFX_STOCKFONTARRAY_H_
#define CORE_FPDFAPI_FONT_CFX_STOCKFONTARRAY_H_

#include <memory>

#include "core/fxge/cfx_fontmapper.h"

class CPDF_Font;

class CFX_StockFontArray {
 public:
  CFX_StockFontArray();
  ~CFX_StockFontArray();

  // Takes ownership of |pFont|, returns unowned pointer to it.
  CPDF_Font* SetFont(CFX_FontMapper::StandardFont index,
                     std::unique_ptr<CPDF_Font> pFont);
  CPDF_Font* GetFont(CFX_FontMapper::StandardFont index) const;

 private:
  std::unique_ptr<CPDF_Font> m_StockFonts[14];
};

#endif  // CORE_FPDFAPI_FONT_CFX_STOCKFONTARRAY_H_
