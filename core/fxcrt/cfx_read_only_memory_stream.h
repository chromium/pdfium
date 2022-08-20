// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_READ_ONLY_MEMORY_STREAM_H_
#define CORE_FXCRT_CFX_READ_ONLY_MEMORY_STREAM_H_

#include <memory>

#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_ReadOnlySpanStream;

class CFX_ReadOnlyMemoryStream final : public IFX_SeekableReadStream {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  // IFX_SeekableReadStream:
  FX_FILESIZE GetSize() override;
  bool ReadBlockAtOffset(void* buffer,
                         FX_FILESIZE offset,
                         size_t size) override;

 private:
  CFX_ReadOnlyMemoryStream(std::unique_ptr<uint8_t, FxFreeDeleter> data,
                           size_t size);
  ~CFX_ReadOnlyMemoryStream() override;

  const std::unique_ptr<uint8_t, FxFreeDeleter> m_data;
  const RetainPtr<CFX_ReadOnlySpanStream> m_stream;
};

#endif  // CORE_FXCRT_CFX_READ_ONLY_MEMORY_STREAM_H_
