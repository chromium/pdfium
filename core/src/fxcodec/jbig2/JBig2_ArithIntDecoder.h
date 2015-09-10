// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_ARITH_INT_DECODER_H_
#define _JBIG2_ARITH_INT_DECODER_H_
#include "JBig2_Module.h"
#include "JBig2_ArithDecoder.h"
class CJBig2_ArithIntDecoder {
 public:
  CJBig2_ArithIntDecoder();

  ~CJBig2_ArithIntDecoder();

  int decode(CJBig2_ArithDecoder* pArithDecoder, int* nResult);

 private:
  JBig2ArithCtx* IAx;
};
class CJBig2_ArithIaidDecoder {
 public:
  CJBig2_ArithIaidDecoder(unsigned char SBSYMCODELENA);

  ~CJBig2_ArithIaidDecoder();

  int decode(CJBig2_ArithDecoder* pArithDecoder, int* nResult);

 private:
  JBig2ArithCtx* IAID;

  unsigned char SBSYMCODELEN;
};
#endif
