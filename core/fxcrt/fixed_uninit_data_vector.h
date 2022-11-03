// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_FIXED_UNINIT_DATA_VECTOR_H_
#define CORE_FXCRT_FIXED_UNINIT_DATA_VECTOR_H_

#include "core/fxcrt/fixed_size_data_vector.h"

template <typename T>
using FixedUninitDataVector =
    fxcrt::FixedSizeDataVector<T, fxcrt::DataVectorAllocOption::kUninitialized>;

#endif  // CORE_FXCRT_FIXED_UNINIT_DATA_VECTOR_H_
