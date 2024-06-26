// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/utils/file_util.h"

#include <stdio.h>

#include <utility>
#include <vector>

#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "testing/utils/path_service.h"

std::vector<uint8_t> GetFileContents(const char* filename) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open: %s\n", filename);
    return {};
  }
  (void)fseek(file, 0, SEEK_END);
  size_t file_length = ftell(file);
  if (!file_length) {
    return {};
  }
  (void)fseek(file, 0, SEEK_SET);
  std::vector<uint8_t> buffer(file_length);
  size_t bytes_read = fread(buffer.data(), 1, file_length, file);
  (void)fclose(file);
  if (bytes_read != file_length) {
    fprintf(stderr, "Failed to read: %s\n", filename);
    return {};
  }
  return buffer;
}

FileAccessForTesting::FileAccessForTesting(const std::string& file_name) {
  std::string file_path = PathService::GetTestFilePath(file_name);
  if (file_path.empty()) {
    return;
  }

  file_contents_ = GetFileContents(file_path.c_str());
  if (file_contents_.empty()) {
    return;
  }

  m_FileLen = pdfium::checked_cast<unsigned long>(file_contents_.size());
  m_GetBlock = SGetBlock;
  m_Param = this;
}

int FileAccessForTesting::GetBlockImpl(unsigned long pos,
                                       unsigned char* pBuf,
                                       unsigned long size) {
  fxcrt::Copy(pdfium::make_span(file_contents_).subspan(pos, size),
              pdfium::make_span(pBuf, size));
  return size ? 1 : 0;
}

// static
int FileAccessForTesting::SGetBlock(void* param,
                                    unsigned long pos,
                                    unsigned char* pBuf,
                                    unsigned long size) {
  auto* file_access = static_cast<FileAccessForTesting*>(param);
  return file_access->GetBlockImpl(pos, pBuf, size);
}
