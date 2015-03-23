// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDFIUM_THIRD_PARTY_BASE_LOGGING_H_
#define PDFIUM_THIRD_PARTY_BASE_LOGGING_H_

#include <assert.h>
#include <stdlib.h>

#define CHECK(condition)                                                \
  if (!(condition)) {                                                   \
    abort();                                                            \
    *(reinterpret_cast<volatile char*>(NULL) + 42) = 0x42;              \
  }

#define NOTREACHED() assert(false)

#endif  // PDFIUM_THIRD_PARTY_BASE_LOGGING_H_
