// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CFX_STOCKFONTARRAY_H_
#define CORE_FPDFAPI_FONT_CFX_STOCKFONTARRAY_H_

#include <memory>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_fontmapper.h"

class CPDF_Font;

class CFX_StockFontArray {
 public:
  CFX_StockFontArray();
  ~CFX_StockFontArray();

  RetainPtr<CPDF_Font> GetFont(CFX_FontMapper::StandardFont index) const;
  void SetFont(CFX_FontMapper::StandardFont index,
               const RetainPtr<CPDF_Font>& pFont);

 private:
  RetainPtr<CPDF_Font> m_StockFonts[14];
};

#endif  // CORE_FPDFAPI_FONT_CFX_STOCKFONTARRAY_H_
