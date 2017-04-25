// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_CODEC_CCODEC_TIFFMODULE_H_
#define CORE_FXCODEC_CODEC_CCODEC_TIFFMODULE_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/dib/cfx_dibitmap.h"

class CCodec_TiffContext;
class CFX_DIBAttribute;

class CCodec_TiffModule {
 public:
  CCodec_TiffContext* CreateDecoder(
      const CFX_RetainPtr<IFX_SeekableReadStream>& file_ptr);
  bool LoadFrameInfo(CCodec_TiffContext* ctx,
                     int32_t frame,
                     int32_t* width,
                     int32_t* height,
                     int32_t* comps,
                     int32_t* bpc,
                     CFX_DIBAttribute* pAttribute);
  bool Decode(CCodec_TiffContext* ctx,
              const CFX_RetainPtr<CFX_DIBitmap>& pDIBitmap);
  void DestroyDecoder(CCodec_TiffContext* ctx);
};

#endif  // CORE_FXCODEC_CODEC_CCODEC_TIFFMODULE_H_
