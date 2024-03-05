// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_TEST_LOADER_H_
#define TESTING_TEST_LOADER_H_

#include <stdint.h>

#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/span.h"

class TestLoader {
 public:
  explicit TestLoader(pdfium::span<const uint8_t> span);

  static int GetBlock(void* param,
                      unsigned long pos,
                      unsigned char* pBuf,
                      unsigned long size);

 private:
  const pdfium::raw_span<const uint8_t> m_Span;
};

#endif  // TESTING_TEST_LOADER_H_
