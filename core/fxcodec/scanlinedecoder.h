// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_SCANLINEDECODER_H_
#define CORE_FXCODEC_SCANLINEDECODER_H_

#include "core/fxcrt/fx_system.h"

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

  const uint8_t* GetScanline(int line);
  bool SkipToScanline(int line, PauseIndicatorIface* pPause);

  int GetWidth() const { return m_OutputWidth; }
  int GetHeight() const { return m_OutputHeight; }
  int CountComps() const { return m_nComps; }
  int GetBPC() const { return m_bpc; }

  virtual uint32_t GetSrcOffset() = 0;

 protected:
  virtual bool v_Rewind() = 0;
  virtual uint8_t* v_GetNextLine() = 0;

  uint8_t* ReadNextLine();

  int m_OrigWidth;
  int m_OrigHeight;
  int m_OutputWidth;
  int m_OutputHeight;
  int m_nComps;
  int m_bpc;
  uint32_t m_Pitch;
  int m_NextLine = -1;
  uint8_t* m_pLastScanline = nullptr;
};

}  // namespace fxcodec

using ScanlineDecoder = fxcodec::ScanlineDecoder;

#endif  // CORE_FXCODEC_SCANLINEDECODER_H_
