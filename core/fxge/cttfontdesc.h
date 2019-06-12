// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CTTFONTDESC_H_
#define CORE_FXGE_CTTFONTDESC_H_

#include <memory>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_face.h"

class CTTFontDesc {
 public:
  explicit CTTFontDesc(std::unique_ptr<uint8_t, FxFreeDeleter> pData);
  ~CTTFontDesc();

  uint8_t* FontData() const { return m_pFontData.get(); }
  void SetFace(size_t index, const RetainPtr<CFX_Face>& face);
  RetainPtr<CFX_Face> GetFace(size_t index) const;

 private:
  std::unique_ptr<uint8_t, FxFreeDeleter> const m_pFontData;
  CFX_Face::ObservedPtr m_TTCFaces[16];
};

#endif  // CORE_FXGE_CTTFONTDESC_H_
