// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_ONED_BC_ONEDCODE128WRITER_H_
#define FXBARCODE_ONED_BC_ONEDCODE128WRITER_H_

#include <stdint.h>

#include <vector>

#include "core/fxcrt/fx_string.h"
#include "fxbarcode/BC_Library.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

class CBC_OnedCode128Writer final : public CBC_OneDimWriter {
 public:
  explicit CBC_OnedCode128Writer(BC_TYPE type);
  ~CBC_OnedCode128Writer() override;

  // Exposed for testing.
  static int32_t Encode128B(const ByteString& contents,
                            std::vector<int32_t>* patterns);
  static int32_t Encode128C(const ByteString& contents,
                            std::vector<int32_t>* patterns);

  // CBC_OneDimWriter
  DataVector<uint8_t> Encode(const ByteString& contents) override;
  bool CheckContentValidity(WideStringView contents) override;
  WideString FilterContents(WideStringView contents) override;
  void SetTextLocation(BC_TEXT_LOC location) override;

  BC_TYPE GetType() const { return code_format_; }

 private:
  const BC_TYPE code_format_;
};

#endif  // FXBARCODE_ONED_BC_ONEDCODE128WRITER_H_
