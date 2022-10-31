// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/fx_codec.h"

#include <utility>

#include "core/fxge/dib/fx_dib.h"

namespace fxcodec {

#ifdef PDF_ENABLE_XFA
CFX_DIBAttribute::CFX_DIBAttribute() = default;

CFX_DIBAttribute::~CFX_DIBAttribute() = default;
#endif  // PDF_ENABLE_XFA

void ReverseRGB(uint8_t* pDestBuf, const uint8_t* pSrcBuf, int pixels) {
  if (pDestBuf == pSrcBuf) {
    for (int i = 0; i < pixels; i++) {
      std::swap(pDestBuf[0], pDestBuf[2]);
      pDestBuf += 3;
    }
  } else {
    for (int i = 0; i < pixels; i++) {
      ReverseCopy3Bytes(pDestBuf, pSrcBuf);
      pDestBuf += 3;
      pSrcBuf += 3;
    }
  }
}

}  // namespace fxcodec
