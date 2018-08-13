// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_CODEC_CFX_CODEC_MEMORY_H_
#define CORE_FXCODEC_CODEC_CFX_CODEC_MEMORY_H_

#include "core/fxcrt/retain_ptr.h"

class CFX_CodecMemory : public Retainable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  uint8_t* GetBuffer() { return buffer_; }
  size_t GetSize() const { return size_; }
  size_t GetPosition() const { return pos_; }
  bool IsEOF() const { return pos_ >= size_; }
  size_t ReadBlock(void* buffer, size_t size);

  // Sets the cursor position to |pos| if possible.
  bool Seek(size_t pos);

 private:
  CFX_CodecMemory(uint8_t* buffer, size_t size);
  ~CFX_CodecMemory() override;

  uint8_t* const buffer_;
  const size_t size_;
  size_t pos_ = 0;
};

#endif  // CORE_FXCODEC_CODEC_CFX_CODEC_MEMORY_H_
