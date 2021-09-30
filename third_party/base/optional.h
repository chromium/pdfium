// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_OPTIONAL_H_
#define THIRD_PARTY_BASE_OPTIONAL_H_

#include "third_party/abseil-cpp/absl/types/optional.h"

namespace pdfium {

using absl::make_optional;
using absl::nullopt;
using absl::nullopt_t;

}  // namespace pdfium

template <class T>
using Optional = absl::optional<T>;

#endif  // THIRD_PARTY_BASE_OPTIONAL_H_
