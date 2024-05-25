// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_ONED_BC_ONEDCODE39WRITER_H_
#define FXBARCODE_ONED_BC_ONEDCODE39WRITER_H_

#include <stddef.h>

#include "fxbarcode/BC_Library.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

class CBC_OnedCode39Writer final : public CBC_OneDimWriter {
 public:
  CBC_OnedCode39Writer();
  ~CBC_OnedCode39Writer() override;

  // CBC_OneDimWriter
  DataVector<uint8_t> Encode(const ByteString& contents) override;
  bool RenderResult(WideStringView contents,
                    pdfium::span<const uint8_t> code) override;
  bool CheckContentValidity(WideStringView contents) override;
  WideString FilterContents(WideStringView contents) override;
  void SetTextLocation(BC_TEXT_LOC location) override;
  bool SetWideNarrowRatio(int8_t ratio) override;

  WideString RenderTextContents(WideStringView contents);
  bool encodedContents(WideStringView contents, WideString* result);

 private:
  static constexpr size_t kArraySize = 9;

  int8_t m_iWideNarrRatio = 3;
};

#endif  // FXBARCODE_ONED_BC_ONEDCODE39WRITER_H_
