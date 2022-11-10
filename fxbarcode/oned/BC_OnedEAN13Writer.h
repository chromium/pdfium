// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_ONED_BC_ONEDEAN13WRITER_H_
#define FXBARCODE_ONED_BC_ONEDEAN13WRITER_H_

#include <stdint.h>

#include "core/fxcrt/fx_string.h"
#include "fxbarcode/BC_Library.h"
#include "fxbarcode/oned/BC_OnedEANWriter.h"

class CFX_RenderDevice;

class CBC_OnedEAN13Writer final : public CBC_OneDimEANWriter {
 public:
  CBC_OnedEAN13Writer();
  ~CBC_OnedEAN13Writer() override;

  // CBC_OneDimEANWriter:
  DataVector<uint8_t> Encode(const ByteString& contents) override;
  bool CheckContentValidity(WideStringView contents) override;
  WideString FilterContents(WideStringView contents) override;
  int32_t CalcChecksum(const ByteString& contents) override;

 private:
  bool ShowChars(WideStringView contents,
                 CFX_RenderDevice* device,
                 const CFX_Matrix& matrix,
                 int32_t barWidth) override;

  int32_t m_codeWidth;
};

#endif  // FXBARCODE_ONED_BC_ONEDEAN13WRITER_H_
