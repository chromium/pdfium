// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_BMP_CFX_BMPCONTEXT_H_
#define CORE_FXCODEC_BMP_CFX_BMPCONTEXT_H_

#include "core/fxcodec/bmp/bmp_decoder.h"
#include "core/fxcodec/bmp/cfx_bmpdecompressor.h"
#include "core/fxcodec/bmp/fx_bmp.h"
#include "core/fxcodec/progressive_decoder_context.h"
#include "core/fxcrt/unowned_ptr.h"

namespace fxcodec {

class CFX_BmpContext final : public ProgressiveDecoderContext {
 public:
  explicit CFX_BmpContext(BmpDecoder::Delegate* pDelegate);
  ~CFX_BmpContext() override;

  CFX_BmpDecompressor bmp_;
  UnownedPtr<BmpDecoder::Delegate> const delegate_;
};

}  // namespace fxcodec

#endif  // CORE_FXCODEC_BMP_CFX_BMPCONTEXT_H_
