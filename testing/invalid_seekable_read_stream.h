// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_INVALID_SEEKABLE_READ_STREAM_H_
#define TESTING_INVALID_SEEKABLE_READ_STREAM_H_

#include "core/fxcrt/fx_stream.h"

// A stream used for testing where reads always fail.
class InvalidSeekableReadStream final : public IFX_SeekableReadStream {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFX_SeekableReadStream overrides:
  bool ReadBlockAtOffset(void* buffer,
                         FX_FILESIZE offset,
                         size_t size) override;
  FX_FILESIZE GetSize() override;

 private:
  explicit InvalidSeekableReadStream(FX_FILESIZE data_size);
  ~InvalidSeekableReadStream() override;

  const FX_FILESIZE data_size_;
};

#endif  // TESTING_INVALID_SEEKABLE_READ_STREAM_H_
