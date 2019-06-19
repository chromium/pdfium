// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CTTFONTDESC_H_
#define CORE_FXGE_CTTFONTDESC_H_

#include <memory>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/observed_ptr.h"
#include "third_party/base/span.h"

class CFX_Face;

class CTTFontDesc {
 public:
  CTTFontDesc(std::unique_ptr<uint8_t, FxFreeDeleter> pData, size_t size);
  ~CTTFontDesc();

  pdfium::span<const uint8_t> FontData() const {
    return {m_pFontData.get(), m_Size};
  }
  void SetFace(size_t index, CFX_Face* face);
  CFX_Face* GetFace(size_t index) const;

 private:
  std::unique_ptr<uint8_t, FxFreeDeleter> const m_pFontData;
  const size_t m_Size;
  ObservedPtr<CFX_Face> m_TTCFaces[16];
};

#endif  // CORE_FXGE_CTTFONTDESC_H_
