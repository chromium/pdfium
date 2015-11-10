// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_JBIG2_JBIG2_ARITHINTDECODER_H_
#define CORE_SRC_FXCODEC_JBIG2_JBIG2_ARITHINTDECODER_H_

#include <vector>

#include "JBig2_ArithDecoder.h"
#include "core/include/fxcrt/fx_system.h"

class CJBig2_ArithIntDecoder {
 public:
  CJBig2_ArithIntDecoder();
  ~CJBig2_ArithIntDecoder();

  // Returns true on success, and false when an OOB condition occurs. Many
  // callers can tolerate OOB and do not check the return value.
  bool decode(CJBig2_ArithDecoder* pArithDecoder, int* nResult);

 private:
  std::vector<JBig2ArithCtx> m_IAx;
};

class CJBig2_ArithIaidDecoder {
 public:
  explicit CJBig2_ArithIaidDecoder(unsigned char SBSYMCODELENA);
  ~CJBig2_ArithIaidDecoder();

  void decode(CJBig2_ArithDecoder* pArithDecoder, FX_DWORD* nResult);

 private:
  std::vector<JBig2ArithCtx> m_IAID;

  const unsigned char SBSYMCODELEN;
};

#endif  // CORE_SRC_FXCODEC_JBIG2_JBIG2_ARITHINTDECODER_H_
