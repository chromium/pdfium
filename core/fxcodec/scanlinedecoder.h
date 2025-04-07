// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_SCANLINEDECODER_H_
#define CORE_FXCODEC_SCANLINEDECODER_H_

#include <stdint.h>

#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/span.h"

class PauseIndicatorIface;

namespace fxcodec {

class ScanlineDecoder {
 public:
  ScanlineDecoder();
  ScanlineDecoder(int nOrigWidth,
                  int nOrigHeight,
                  int nOutputWidth,
                  int nOutputHeight,
                  int nComps,
                  int nBpc,
                  uint32_t nPitch);
  virtual ~ScanlineDecoder();

  pdfium::span<const uint8_t> GetScanline(int line);
  bool SkipToScanline(int line, PauseIndicatorIface* pPause);

  int GetWidth() const { return output_width_; }
  int GetHeight() const { return output_height_; }
  int CountComps() const { return comps_; }
  int GetBPC() const { return bpc_; }

  virtual uint32_t GetSrcOffset() = 0;

 protected:
  [[nodiscard]] virtual bool Rewind() = 0;
  virtual pdfium::span<uint8_t> GetNextLine() = 0;

  int orig_width_;
  int orig_height_;
  int output_width_;
  int output_height_;
  int comps_;
  int bpc_;
  uint32_t pitch_;
  int next_line_ = -1;
  pdfium::raw_span<uint8_t> last_scanline_;
};

}  // namespace fxcodec

using ScanlineDecoder = fxcodec::ScanlineDecoder;

#endif  // CORE_FXCODEC_SCANLINEDECODER_H_
