// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_STRING_WRITE_STREAM_H_
#define TESTING_STRING_WRITE_STREAM_H_

#include <sstream>
#include <string>

#include "core/fxcrt/fx_stream.h"

class StringWriteStream final : public IFX_RetainableWriteStream {
 public:
  StringWriteStream();
  ~StringWriteStream() override;

  // IFX_WriteStream:
  bool WriteBlock(pdfium::span<const uint8_t> buffer) override;

  std::string ToString() const { return stream_.str(); }

 private:
  std::ostringstream stream_;
};

#endif  // TESTING_STRING_WRITE_STREAM_H_
