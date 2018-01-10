// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cttfontdesc.h"

#include "core/fxge/fx_freetype.h"
#include "third_party/base/stl_util.h"

CTTFontDesc::CTTFontDesc(uint8_t* pData, FXFT_Face face)
    : m_bIsTTC(false), m_SingleFace(face), m_pFontData(pData) {}

CTTFontDesc::CTTFontDesc(uint8_t* pData, size_t index, FXFT_Face face)
    : m_bIsTTC(true), m_pFontData(pData) {
  for (size_t i = 0; i < FX_ArraySize(m_TTCFaces); i++)
    m_TTCFaces[i] = nullptr;
  SetTTCFace(index, face);
}

CTTFontDesc::~CTTFontDesc() {
  ASSERT(m_RefCount == 0);
  if (m_bIsTTC) {
    for (size_t i = 0; i < FX_ArraySize(m_TTCFaces); i++) {
      if (m_TTCFaces[i])
        FXFT_Done_Face(m_TTCFaces[i]);
    }
  } else {
    if (m_SingleFace)
      FXFT_Done_Face(m_SingleFace);
  }
  FX_Free(m_pFontData);
}

void CTTFontDesc::SetTTCFace(size_t index, FXFT_Face face) {
  ASSERT(m_bIsTTC);
  ASSERT(index < FX_ArraySize(m_TTCFaces));
  m_TTCFaces[index] = face;
}

void CTTFontDesc::AddRef() {
  ASSERT(m_RefCount > 0);
  ++m_RefCount;
}

CTTFontDesc::ReleaseStatus CTTFontDesc::ReleaseFace(FXFT_Face face) {
  if (m_bIsTTC) {
    if (!pdfium::ContainsValue(m_TTCFaces, face))
      return kNotAppropriate;
  } else {
    if (m_SingleFace != face)
      return kNotAppropriate;
  }
  ASSERT(m_RefCount > 0);
  return --m_RefCount == 0 ? kReleased : kNotReleased;
}

FXFT_Face CTTFontDesc::SingleFace() const {
  ASSERT(!m_bIsTTC);
  return m_SingleFace;
}

FXFT_Face CTTFontDesc::TTCFace(size_t index) const {
  ASSERT(m_bIsTTC);
  ASSERT(index < FX_ArraySize(m_TTCFaces));
  return m_TTCFaces[index];
}
