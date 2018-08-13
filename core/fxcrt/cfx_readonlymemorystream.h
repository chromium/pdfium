// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_READONLYMEMORYSTREAM_H_
#define CORE_FXCRT_CFX_READONLYMEMORYSTREAM_H_

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_ReadOnlyMemoryStream final : public IFX_SeekableReadStream {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFX_SeekableReadStream:
  FX_FILESIZE GetSize() override;
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;

 private:
  CFX_ReadOnlyMemoryStream(const uint8_t* pBuf, FX_FILESIZE size);
  ~CFX_ReadOnlyMemoryStream() override;

  const uint8_t* const m_pBuf;
  const FX_FILESIZE m_size;
};

#endif  // CORE_FXCRT_CFX_READONLYMEMORYSTREAM_H_
