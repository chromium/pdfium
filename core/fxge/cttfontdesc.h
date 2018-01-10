// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CTTFONTDESC_H_
#define CORE_FXGE_CTTFONTDESC_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_font.h"

class CTTFontDesc {
 public:
  enum ReleaseStatus {
    kNotAppropriate,  // ReleaseFace() not appropriate for given object.
    kReleased,
    kNotReleased  // Object still alive.
  };

  // Single face ctor.
  CTTFontDesc(uint8_t* pData, FXFT_Face face);

  // TTC face ctor.
  CTTFontDesc(uint8_t* pData, size_t index, FXFT_Face face);

  ~CTTFontDesc();

  void SetTTCFace(size_t index, FXFT_Face face);

  void AddRef();

  // May not decrement refcount, depending on the value of |face|.
  ReleaseStatus ReleaseFace(FXFT_Face face);

  uint8_t* FontData() const { return m_pFontData; }

  FXFT_Face SingleFace() const;
  FXFT_Face TTCFace(size_t index) const;

 private:
  const bool m_bIsTTC;

  union {
    const FXFT_Face m_SingleFace;
    FXFT_Face m_TTCFaces[16];
  };
  uint8_t* const m_pFontData;
  int m_RefCount = 1;
};

#endif  // CORE_FXGE_CTTFONTDESC_H_
