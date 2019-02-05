// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/test_loader.h"

#include <string.h>

#include "third_party/base/logging.h"

TestLoader::TestLoader(pdfium::span<const char> span) : m_Span(span) {}

// static
int TestLoader::GetBlock(void* param,
                         unsigned long pos,
                         unsigned char* pBuf,
                         unsigned long size) {
  TestLoader* pLoader = static_cast<TestLoader*>(param);
  if (pos + size < pos || pos + size > pLoader->m_Span.size()) {
    NOTREACHED();
    return 0;
  }

  memcpy(pBuf, &pLoader->m_Span[pos], size);
  return 1;
}
