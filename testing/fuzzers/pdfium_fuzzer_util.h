// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FUZZERS_PDFIUM_FUZZER_UTIL_H_
#define TESTING_FUZZERS_PDFIUM_FUZZER_UTIL_H_

#include <stdint.h>

// Returns an integer from the first 4 bytes of |data|.
int GetInteger(const uint8_t* data);

#endif  // TESTING_FUZZERS_PDFIUM_FUZZER_UTIL_H_
