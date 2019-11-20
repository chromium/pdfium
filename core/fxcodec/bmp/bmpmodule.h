// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_BMP_BMPMODULE_H_
#define CORE_FXCODEC_BMP_BMPMODULE_H_

#include <memory>
#include <vector>

#include "core/fxcodec/codec_module_iface.h"
#include "third_party/base/span.h"

namespace fxcodec {

class CFX_DIBAttribute;

class BmpModule final : public ModuleIface {
 public:
  class Delegate {
   public:
    virtual bool BmpInputImagePositionBuf(uint32_t rcd_pos) = 0;
    virtual void BmpReadScanline(uint32_t row_num,
                                 pdfium::span<const uint8_t> row_buf) = 0;
  };

  enum class Status : uint8_t { kFail, kSuccess, kContinue };

  BmpModule();
  ~BmpModule() override;

  // ModuleIface:
  FX_FILESIZE GetAvailInput(Context* pContext) const override;
  bool Input(Context* pContext,
             RetainPtr<CFX_CodecMemory> codec_memory,
             CFX_DIBAttribute* pAttribute) override;

  std::unique_ptr<Context> Start(Delegate* pDelegate);
  Status ReadHeader(Context* pContext,
                    int32_t* width,
                    int32_t* height,
                    bool* tb_flag,
                    int32_t* components,
                    int32_t* pal_num,
                    const std::vector<uint32_t>** palette,
                    CFX_DIBAttribute* pAttribute);
  Status LoadImage(Context* pContext);
};

}  // namespace fxcodec

using BmpModule = fxcodec::BmpModule;

#endif  // CORE_FXCODEC_BMP_BMPMODULE_H_
