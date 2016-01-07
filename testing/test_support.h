// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_SUPPORT_H_
#define TESTING_EMBEDDER_TEST_SUPPORT_H_

#include <stdlib.h>
#include <memory>
#include <string>

#include "public/fpdfview.h"
#include "public/fpdf_save.h"

#ifdef PDF_ENABLE_V8
#include "v8/include/v8.h"
#endif  // PDF_ENABLE_V8

namespace pdfium {

// Used with std::unique_ptr to free() objects that can't be deleted.
struct FreeDeleter {
  inline void operator()(void* ptr) const { free(ptr); }
};

}  // namespace pdfium

// Reads the entire contents of a file into a newly alloc'd buffer.
std::unique_ptr<char, pdfium::FreeDeleter> GetFileContents(const char* filename,
                                                           size_t* retlen);

// Converts a FPDF_WIDESTRING to a std::wstring.
// Deals with differences between UTF16LE and wchar_t.
std::wstring GetPlatformWString(const FPDF_WIDESTRING wstr);

// Returns a newly allocated FPDF_WIDESTRING.
// Deals with differences between UTF16LE and wchar_t.
std::unique_ptr<unsigned short, pdfium::FreeDeleter> GetFPDFWideString(
    const std::wstring& wstr);

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

class TestSaver : public FPDF_FILEWRITE {
 public:
  TestSaver();

  void ClearString();
  const std::string& GetString() const { return m_String; }

 private:
  static int WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                const void* data,
                                unsigned long size);

  std::string m_String;
};

#endif  // TESTING_EMBEDDER_TEST_SUPPORT_H_
