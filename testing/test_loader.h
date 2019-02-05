// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_TEST_LOADER_H_
#define TESTING_TEST_LOADER_H_

#include "third_party/base/span.h"

class TestLoader {
 public:
  explicit TestLoader(pdfium::span<const char> span);

  static int GetBlock(void* param,
                      unsigned long pos,
                      unsigned char* pBuf,
                      unsigned long size);

 private:
  const pdfium::span<const char> m_Span;
};

#endif  // TESTING_TEST_LOADER_H_
