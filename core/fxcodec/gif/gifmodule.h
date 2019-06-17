// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_GIF_GIFMODULE_H_
#define CORE_FXCODEC_GIF_GIFMODULE_H_

#include <memory>
#include <utility>

#include "core/fxcodec/codec_module_iface.h"
#include "core/fxcodec/gif/cfx_gif.h"
#include "core/fxcrt/fx_coordinates.h"

namespace fxcodec {

class CFX_DIBAttribute;

class GifModule final : public ModuleIface {
 public:
  class Delegate {
   public:
    virtual void GifRecordCurrentPosition(uint32_t& cur_pos) = 0;
    virtual bool GifInputRecordPositionBuf(uint32_t rcd_pos,
                                           const FX_RECT& img_rc,
                                           int32_t pal_num,
                                           CFX_GifPalette* pal_ptr,
                                           int32_t delay_time,
                                           bool user_input,
                                           int32_t trans_index,
                                           int32_t disposal_method,
                                           bool interlace) = 0;
    virtual void GifReadScanline(int32_t row_num, uint8_t* row_buf) = 0;
  };

  GifModule();
  ~GifModule() override;

  // ModuleIface:
  FX_FILESIZE GetAvailInput(Context* context) const override;
  bool Input(Context* context,
             RetainPtr<CFX_CodecMemory> codec_memory,
             CFX_DIBAttribute* pAttribute) override;

  std::unique_ptr<Context> Start(Delegate* pDelegate);
  CFX_GifDecodeStatus ReadHeader(Context* context,
                                 int* width,
                                 int* height,
                                 int* pal_num,
                                 CFX_GifPalette** pal_pp,
                                 int* bg_index);
  std::pair<CFX_GifDecodeStatus, size_t> LoadFrameInfo(Context* context);
  CFX_GifDecodeStatus LoadFrame(Context* context, size_t frame_num);
};

}  // namespace fxcodec

using GifModule = fxcodec::GifModule;

#endif  // CORE_FXCODEC_GIF_GIFMODULE_H_
