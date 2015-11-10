// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_JBIG2_JBIG2_PDDPROC_H_
#define CORE_SRC_FXCODEC_JBIG2_JBIG2_PDDPROC_H_

#include "core/include/fxcrt/fx_system.h"

class CJBig2_ArithDecoder;
class CJBig2_BitStream;
class CJBig2_PatternDict;
class IFX_Pause;
struct JBig2ArithCtx;

class CJBig2_PDDProc {
 public:
  CJBig2_PatternDict* decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                                   JBig2ArithCtx* gbContext,
                                   IFX_Pause* pPause);

  CJBig2_PatternDict* decode_MMR(CJBig2_BitStream* pStream, IFX_Pause* pPause);

 public:
  FX_BOOL HDMMR;
  uint8_t HDPW;
  uint8_t HDPH;
  FX_DWORD GRAYMAX;
  uint8_t HDTEMPLATE;
};

#endif  // CORE_SRC_FXCODEC_JBIG2_JBIG2_PDDPROC_H_
