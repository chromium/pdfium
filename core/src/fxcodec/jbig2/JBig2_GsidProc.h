// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_JBIG2_JBIG2_GSIDPROC_H_
#define CORE_SRC_FXCODEC_JBIG2_JBIG2_GSIDPROC_H_

#include "core/include/fxcrt/fx_system.h"

class CJBig2_ArithDecoder;
class CJBig2_BitStream;
class CJBig2_Image;
class IFX_Pause;
struct JBig2ArithCtx;

class CJBig2_GSIDProc {
 public:
  FX_DWORD* decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                         JBig2ArithCtx* gbContext,
                         IFX_Pause* pPause);

  FX_DWORD* decode_MMR(CJBig2_BitStream* pStream, IFX_Pause* pPause);

 public:
  FX_BOOL GSMMR;
  FX_BOOL GSUSESKIP;
  uint8_t GSBPP;
  FX_DWORD GSW;
  FX_DWORD GSH;
  uint8_t GSTEMPLATE;
  CJBig2_Image* GSKIP;
};

#endif  // CORE_SRC_FXCODEC_JBIG2_JBIG2_GSIDPROC_H_
