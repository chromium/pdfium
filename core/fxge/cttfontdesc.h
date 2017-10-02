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
  CTTFontDesc() : m_Type(0), m_pFontData(nullptr), m_RefCount(0) {}
  ~CTTFontDesc();
  // ret < 0, releaseface not appropriate for this object.
  // ret == 0, object released
  // ret > 0, object still alive, other referrers.
  int ReleaseFace(FXFT_Face face);

  int m_Type;

  union {
    FXFT_Face m_SingleFace;
    FXFT_Face m_TTCFaces[16];
  };
  uint8_t* m_pFontData;
  int m_RefCount;
};

#endif  // CORE_FXGE_CTTFONTDESC_H_
