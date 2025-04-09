// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/scanlinedecoder.h"

#include "core/fxcrt/pauseindicator_iface.h"

namespace fxcodec {

ScanlineDecoder::ScanlineDecoder() : ScanlineDecoder(0, 0, 0, 0, 0, 0, 0) {}

ScanlineDecoder::ScanlineDecoder(int nOrigWidth,
                                 int nOrigHeight,
                                 int nOutputWidth,
                                 int nOutputHeight,
                                 int nComps,
                                 int nBpc,
                                 uint32_t nPitch)
    : orig_width_(nOrigWidth),
      orig_height_(nOrigHeight),
      output_width_(nOutputWidth),
      output_height_(nOutputHeight),
      comps_(nComps),
      bpc_(nBpc),
      pitch_(nPitch) {}

ScanlineDecoder::~ScanlineDecoder() = default;

pdfium::span<const uint8_t> ScanlineDecoder::GetScanline(int line) {
  if (next_line_ == line + 1) {
    return last_scanline_;
  }

  if (next_line_ < 0 || next_line_ > line) {
    if (!Rewind()) {
      return pdfium::span<const uint8_t>();
    }
    next_line_ = 0;
  }
  while (next_line_ < line) {
    GetNextLine();
    next_line_++;
  }
  last_scanline_ = GetNextLine();
  next_line_++;
  return last_scanline_;
}

bool ScanlineDecoder::SkipToScanline(int line, PauseIndicatorIface* pPause) {
  if (next_line_ == line || next_line_ == line + 1) {
    return false;
  }

  if (next_line_ < 0 || next_line_ > line) {
    if (!Rewind()) {
      return false;
    }
    next_line_ = 0;
  }
  last_scanline_ = pdfium::span<uint8_t>();
  while (next_line_ < line) {
    last_scanline_ = GetNextLine();
    next_line_++;
    if (pPause && pPause->NeedToPauseNow()) {
      return true;
    }
  }
  return false;
}

}  // namespace fxcodec
