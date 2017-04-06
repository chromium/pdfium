// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_IFX_SCANLINECOMPOSER_H_
#define CORE_FXGE_DIB_IFX_SCANLINECOMPOSER_H_

#include "core/fxge/fx_dib.h"

class IFX_ScanlineComposer {
 public:
  virtual ~IFX_ScanlineComposer() {}

  virtual void ComposeScanline(int line,
                               const uint8_t* scanline,
                               const uint8_t* scan_extra_alpha) = 0;

  virtual bool SetInfo(int width,
                       int height,
                       FXDIB_Format src_format,
                       uint32_t* pSrcPalette) = 0;
};

#endif  // CORE_FXGE_DIB_IFX_SCANLINECOMPOSER_H_
