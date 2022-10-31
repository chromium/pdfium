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
  CBC_PDF417Writer();
  ~CBC_PDF417Writer() override;

  DataVector<uint8_t> Encode(WideStringView contents,
                             int32_t* pOutWidth,
                             int32_t* pOutHeight);

  // CBC_TwoDimWriter
  bool SetErrorCorrectionLevel(int32_t level) override;

 private:
  void RotateArray(DataVector<uint8_t>* bitarray,
                   int32_t width,
                   int32_t height);
};

#endif  // FXBARCODE_PDF417_BC_PDF417WRITER_H_
