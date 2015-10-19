// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_SUPPORT_H_
#define TESTING_EMBEDDER_TEST_SUPPORT_H_

#include <stdlib.h>
#include <string>

#ifdef PDF_ENABLE_V8
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"
#endif

// Reads the entire contents of a file into a newly malloc'd buffer.
char* GetFileContents(const char* filename, size_t* retlen);

#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
bool InitializeV8ForPDFium(const std::string& exe_path,
                           const std::string& bin_dir,
                           v8::StartupData* natives_blob,
                           v8::StartupData* snapshot_blob,
                           v8::Platform** platform);
#else   // V8_USE_EXTERNAL_STARTUP_DATA
bool InitializeV8ForPDFium(v8::Platform** platform);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

class TestLoader {
 public:
  TestLoader(const char* pBuf, size_t len);
  static int GetBlock(void* param,
                      unsigned long pos,
                      unsigned char* pBuf,
                      unsigned long size);

 private:
  const char* const m_pBuf;
  const size_t m_Len;
};

#endif  // TESTING_EMBEDDER_TEST_SUPPORT_H_
