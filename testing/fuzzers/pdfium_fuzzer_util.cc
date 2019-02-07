// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fuzzers/pdfium_fuzzer_util.h"

int GetInteger(const uint8_t* data) {
  return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}
