// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_DATA_AND_BYTES_CONSUMED_H_
#define CORE_FXCODEC_DATA_AND_BYTES_CONSUMED_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_memory_wrappers.h"

namespace fxcodec {

// TODO(crbug.com/pdfium/1872): Replace with DataVectorAndBytesConsumed.
struct DataAndBytesConsumed {
  DataAndBytesConsumed(std::unique_ptr<uint8_t, FxFreeDeleter> data,
                       uint32_t size,
                       uint32_t bytes_consumed);
  DataAndBytesConsumed(DataAndBytesConsumed&) = delete;
  DataAndBytesConsumed& operator=(DataAndBytesConsumed&) = delete;
  DataAndBytesConsumed(DataAndBytesConsumed&&) noexcept;
  DataAndBytesConsumed& operator=(DataAndBytesConsumed&&) noexcept;
  ~DataAndBytesConsumed();

  std::unique_ptr<uint8_t, FxFreeDeleter> data;
  uint32_t size;
  // TODO(thestig): Consider replacing with std::optional<size_t>.
  uint32_t bytes_consumed;
};

// TODO(crbug.com/pdfium/1872): Rename to DataAndBytesConsumed once the existing
// struct of that name is no longer used.
struct DataVectorAndBytesConsumed {
  DataVectorAndBytesConsumed(DataVector<uint8_t> data, uint32_t bytes_consumed);
  DataVectorAndBytesConsumed(DataVectorAndBytesConsumed&) = delete;
  DataVectorAndBytesConsumed& operator=(DataVectorAndBytesConsumed&) = delete;
  DataVectorAndBytesConsumed(DataVectorAndBytesConsumed&&) noexcept;
  DataVectorAndBytesConsumed& operator=(DataVectorAndBytesConsumed&&) noexcept;
  ~DataVectorAndBytesConsumed();

  DataVector<uint8_t> data;
  // TODO(thestig): Consider replacing with std::optional<size_t>.
  uint32_t bytes_consumed;
};

}  // namespace fxcodec

using DataAndBytesConsumed = fxcodec::DataAndBytesConsumed;
using DataVectorAndBytesConsumed = fxcodec::DataVectorAndBytesConsumed;

#endif  // CORE_FXCODEC_DATA_AND_BYTES_CONSUMED_H_
