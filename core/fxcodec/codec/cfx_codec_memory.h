// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_CODEC_CFX_CODEC_MEMORY_H_
#define CORE_FXCODEC_CODEC_CFX_CODEC_MEMORY_H_

#include "core/fxcrt/retain_ptr.h"
#include "third_party/base/span.h"

class CFX_CodecMemory final : public Retainable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  uint8_t* GetBuffer() { return buffer_.data(); }
  size_t GetSize() const { return buffer_.size(); }
  size_t GetPosition() const { return pos_; }
  bool IsEOF() const { return pos_ >= buffer_.size(); }
  size_t ReadBlock(void* buffer, size_t size);

  // Sets the cursor position to |pos| if possible.
  bool Seek(size_t pos);

 private:
  explicit CFX_CodecMemory(pdfium::span<uint8_t> buffer);
  ~CFX_CodecMemory() override;

  pdfium::span<uint8_t> const buffer_;
  size_t pos_ = 0;
};

#endif  // CORE_FXCODEC_CODEC_CFX_CODEC_MEMORY_H_
