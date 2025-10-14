// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_ARITHDECODER_H_
#define CORE_FXCODEC_JBIG2_JBIG2_ARITHDECODER_H_

#include <stdint.h>

#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/unowned_ptr.h"

class CJBig2_BitStream;
struct JBig2ArithQe;

class JBig2ArithCtx {
 public:
  struct JBig2ArithQe {
    uint16_t Qe;
    uint8_t NMPS;
    uint8_t NLPS;
    bool bSwitch;
  };

  JBig2ArithCtx();

  int DecodeNLPS(const JBig2ArithQe& qe);
  int DecodeNMPS(const JBig2ArithQe& qe);

  unsigned int MPS() const { return mps_ ? 1 : 0; }
  unsigned int I() const { return i_; }

 private:
  bool mps_ = false;
  unsigned int i_ = 0;
};
FX_DATA_PARTITION_EXCEPTION(JBig2ArithCtx);

class CJBig2_ArithDecoder {
 public:
  explicit CJBig2_ArithDecoder(CJBig2_BitStream* pStream);
  ~CJBig2_ArithDecoder();

  int Decode(JBig2ArithCtx* pCX);

  bool IsComplete() const { return complete_; }

 private:
  void BYTEIN();
  void ReadValueA();

  bool complete_ = false;
  uint8_t b_;
  unsigned int c_;
  unsigned int a_;
  unsigned int ct_;
  UnownedPtr<CJBig2_BitStream> const stream_;
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_ARITHDECODER_H_
