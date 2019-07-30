// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_TYPE3CACHE_H_
#define CORE_FPDFAPI_RENDER_CPDF_TYPE3CACHE_H_

#include <map>
#include <memory>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_GlyphBitmap;
class CFX_Matrix;
class CPDF_Type3Font;
class CPDF_Type3GlyphMap;

class CPDF_Type3Cache final : public Retainable, public Observable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  const CFX_GlyphBitmap* LoadGlyph(uint32_t charcode,
                                   const CFX_Matrix* pMatrix);

 private:
  explicit CPDF_Type3Cache(CPDF_Type3Font* pFont);
  ~CPDF_Type3Cache() override;

  std::unique_ptr<CFX_GlyphBitmap> RenderGlyph(CPDF_Type3GlyphMap* pSize,
                                               uint32_t charcode,
                                               const CFX_Matrix* pMatrix);

  RetainPtr<CPDF_Type3Font> const m_pFont;
  std::map<ByteString, std::unique_ptr<CPDF_Type3GlyphMap>> m_SizeMap;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_TYPE3CACHE_H_
