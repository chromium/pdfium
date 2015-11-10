// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_JBIG2_JBIG2_HTRDPROC_H_
#define CORE_SRC_FXCODEC_JBIG2_JBIG2_HTRDPROC_H_

#include "core/include/fxcrt/fx_system.h"

#include "JBig2_Image.h"

class CJBig2_ArithDecoder;
class CJBig2_BitStream;
class IFX_Pause;
struct JBig2ArithCtx;

class CJBig2_HTRDProc {
 public:
  CJBig2_Image* decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                             JBig2ArithCtx* gbContext,
                             IFX_Pause* pPause);

  CJBig2_Image* decode_MMR(CJBig2_BitStream* pStream, IFX_Pause* pPause);

 public:
  FX_DWORD HBW;
  FX_DWORD HBH;
  FX_BOOL HMMR;
  uint8_t HTEMPLATE;
  FX_DWORD HNUMPATS;
  CJBig2_Image** HPATS;
  FX_BOOL HDEFPIXEL;
  JBig2ComposeOp HCOMBOP;
  FX_BOOL HENABLESKIP;
  FX_DWORD HGW;
  FX_DWORD HGH;
  int32_t HGX;
  int32_t HGY;
  FX_WORD HRX;
  FX_WORD HRY;
  uint8_t HPW;
  uint8_t HPH;
};

#endif  // CORE_SRC_FXCODEC_JBIG2_JBIG2_HTRDPROC_H_
