// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_INVALID_SEEKABLE_READ_STREAM_H_
#define TESTING_INVALID_SEEKABLE_READ_STREAM_H_

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"

// A stream used for testing where reads always fail.
class InvalidSeekableReadStream final : public IFX_SeekableReadStream {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  // IFX_SeekableReadStream overrides:
  bool ReadBlockAtOffset(pdfium::span<uint8_t> buffer,
                         FX_FILESIZE offset) override;
  FX_FILESIZE GetSize() override;

 private:
  explicit InvalidSeekableReadStream(FX_FILESIZE data_size);
  ~InvalidSeekableReadStream() override;

  const FX_FILESIZE data_size_;
};

#endif  // TESTING_INVALID_SEEKABLE_READ_STREAM_H_
