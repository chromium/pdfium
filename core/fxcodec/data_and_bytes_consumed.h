// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_DATA_AND_BYTES_CONSUMED_H_
#define CORE_FXCODEC_DATA_AND_BYTES_CONSUMED_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/fx_memory_wrappers.h"

namespace fxcodec {

struct DataAndBytesConsumed {
  DataAndBytesConsumed(std::unique_ptr<uint8_t, FxFreeDeleter> data,
                       uint32_t size,
                       uint32_t bytes_consumed);
  DataAndBytesConsumed(DataAndBytesConsumed&) = delete;
  DataAndBytesConsumed& operator=(DataAndBytesConsumed&) = delete;
  DataAndBytesConsumed(DataAndBytesConsumed&&) noexcept;
  DataAndBytesConsumed& operator=(DataAndBytesConsumed&&) noexcept;
  ~DataAndBytesConsumed();

  // TODO(crbug.com/pdfium/1872): Replace with DataVector.
  std::unique_ptr<uint8_t, FxFreeDeleter> data;
  uint32_t size;
  // TODO(thestig): Consider replacing with std::optional<size_t>.
  uint32_t bytes_consumed;
};

}  // namespace fxcodec

using DataAndBytesConsumed = fxcodec::DataAndBytesConsumed;

#endif  // CORE_FXCODEC_DATA_AND_BYTES_CONSUMED_H_
