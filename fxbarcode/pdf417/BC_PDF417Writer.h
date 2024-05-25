// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_PDF417_BC_PDF417WRITER_H_
#define FXBARCODE_PDF417_BC_PDF417WRITER_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/widestring.h"
#include "fxbarcode/BC_TwoDimWriter.h"

class CBC_PDF417Writer final : public CBC_TwoDimWriter {
 public:
  struct EncodeResult {
    EncodeResult(DataVector<uint8_t> data, int32_t width, int32_t height);
    ~EncodeResult();

    DataVector<uint8_t> data;
    int32_t width;
    int32_t height;
  };

  CBC_PDF417Writer();
  ~CBC_PDF417Writer() override;

  EncodeResult Encode(WideStringView contents) const;

  // CBC_TwoDimWriter
  bool SetErrorCorrectionLevel(int32_t level) override;
};

#endif  // FXBARCODE_PDF417_BC_PDF417WRITER_H_
