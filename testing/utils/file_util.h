// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_UTILS_FILE_UTIL_H_
#define TESTING_UTILS_FILE_UTIL_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "public/fpdfview.h"

// Reads the entire contents of a file into a vector. Returns an empty vector on
// failure. Note that this function assumes reading an empty file is not a valid
// use case, and treats such an action as a failure.
std::vector<uint8_t> GetFileContents(const char* filename);

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

  std::vector<uint8_t> file_contents_;
};

#endif  // TESTING_UTILS_FILE_UTIL_H_
