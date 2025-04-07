// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_SymbolDict.h"

#include "core/fxcodec/jbig2/JBig2_Image.h"

CJBig2_SymbolDict::CJBig2_SymbolDict() = default;

CJBig2_SymbolDict::~CJBig2_SymbolDict() = default;

std::unique_ptr<CJBig2_SymbolDict> CJBig2_SymbolDict::DeepCopy() const {
  auto dst = std::make_unique<CJBig2_SymbolDict>();
  for (const auto& image : sdexsyms_) {
    dst->sdexsyms_.push_back(image ? std::make_unique<CJBig2_Image>(*image)
                                   : nullptr);
  }
  dst->gb_contexts_ = gb_contexts_;
  dst->gr_contexts_ = gr_contexts_;
  return dst;
}
