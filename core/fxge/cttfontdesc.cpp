// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cttfontdesc.h"

#include <utility>

#include "core/fxge/cfx_face.h"

CTTFontDesc::CTTFontDesc(std::unique_ptr<uint8_t, FxFreeDeleter> pData)
    : m_pFontData(std::move(pData)) {
}

CTTFontDesc::~CTTFontDesc() = default;

void CTTFontDesc::SetFace(size_t index, CFX_Face* face) {
  ASSERT(index < FX_ArraySize(m_TTCFaces));
  m_TTCFaces[index].Reset(face);
}

CFX_Face* CTTFontDesc::GetFace(size_t index) const {
  ASSERT(index < FX_ArraySize(m_TTCFaces));
  return m_TTCFaces[index].Get();
}
