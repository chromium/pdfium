// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_QRCODE_BC_QRCODEWRITER_H_
#define FXBARCODE_QRCODE_BC_QRCODEWRITER_H_

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/widestring.h"
#include "fxbarcode/BC_TwoDimWriter.h"

class CBC_QRCodeWriter final : public CBC_TwoDimWriter {
 public:
  CBC_QRCodeWriter();
  ~CBC_QRCodeWriter() override;

  DataVector<uint8_t> Encode(WideStringView contents,
                             int32_t ecLevel,
                             int32_t* pOutWidth,
                             int32_t* pOutHeight);

  // CBC_TwoDimWriter
  bool SetErrorCorrectionLevel(int32_t level) override;
};

#endif  // FXBARCODE_QRCODE_BC_QRCODEWRITER_H_
