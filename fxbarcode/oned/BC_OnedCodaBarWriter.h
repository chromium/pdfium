// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_
#define FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_

#include <stdint.h>

#include "core/fxcrt/fx_string.h"
#include "fxbarcode/BC_Library.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

class CBC_OnedCodaBarWriter final : public CBC_OneDimWriter {
 public:
  CBC_OnedCodaBarWriter();
  ~CBC_OnedCodaBarWriter() override;

  // CBC_OneDimWriter:
  DataVector<uint8_t> Encode(const ByteString& contents) override;
  bool RenderResult(WideStringView contents,
                    pdfium::span<const uint8_t> code) override;
  bool CheckContentValidity(WideStringView contents) override;
  WideString FilterContents(WideStringView contents) override;
  void SetDataLength(int32_t length) override;
  void SetTextLocation(BC_TEXT_LOC location) override;
  bool SetWideNarrowRatio(int8_t ratio) override;
  bool SetStartChar(char start) override;
  bool SetEndChar(char end) override;

  WideString encodedContents(WideStringView contents);

 private:
  char m_chStart = 'A';
  char m_chEnd = 'B';
  int8_t m_iWideNarrRatio = 2;
};

#endif  // FXBARCODE_ONED_BC_ONEDCODABARWRITER_H_
