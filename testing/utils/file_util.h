// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_UTILS_FILE_UTIL_H_
#define TESTING_UTILS_FILE_UTIL_H_

#include <stdlib.h>

#include <memory>
#include <string>

#include "public/fpdfview.h"
#include "testing/free_deleter.h"

// Reads the entire contents of a file into a newly alloc'd buffer.
std::unique_ptr<char, pdfium::FreeDeleter> GetFileContents(const char* filename,
                                                           size_t* retlen);

// Use an ordinary file anywhere a FPDF_FILEACCESS is required.
class FileAccessForTesting final : public FPDF_FILEACCESS {
 public:
  explicit FileAccessForTesting(const std::string& file_name);

 private:
  static int SGetBlock(void* param,
                       unsigned long pos,
                       unsigned char* pBuf,
                       unsigned long size);

  int GetBlockImpl(unsigned long pos, unsigned char* pBuf, unsigned long size);

  size_t file_length_;
  std::unique_ptr<char, pdfium::FreeDeleter> file_contents_;
};

#endif  // TESTING_UTILS_FILE_UTIL_H_
