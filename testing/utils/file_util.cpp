// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/utils/file_util.h"

#include <stdio.h>
#include <string.h>

#include "testing/utils/path_service.h"

std::unique_ptr<char, pdfium::FreeDeleter> GetFileContents(const char* filename,
                                                           size_t* retlen) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open: %s\n", filename);
    return nullptr;
  }
  (void)fseek(file, 0, SEEK_END);
  size_t file_length = ftell(file);
  if (!file_length) {
    return nullptr;
  }
  (void)fseek(file, 0, SEEK_SET);
  std::unique_ptr<char, pdfium::FreeDeleter> buffer(
      static_cast<char*>(malloc(file_length)));
  if (!buffer) {
    return nullptr;
  }
  size_t bytes_read = fread(buffer.get(), 1, file_length, file);
  (void)fclose(file);
  if (bytes_read != file_length) {
    fprintf(stderr, "Failed to read: %s\n", filename);
    return nullptr;
  }
  *retlen = bytes_read;
  return buffer;
}

FileAccessForTesting::FileAccessForTesting(const std::string& file_name) {
  std::string file_path;
  if (!PathService::GetTestFilePath(file_name, &file_path))
    return;

  file_contents_ = GetFileContents(file_path.c_str(), &file_length_);
  if (!file_contents_)
    return;

  m_FileLen = static_cast<unsigned long>(file_length_);
  m_GetBlock = SGetBlock;
  m_Param = this;
}

int FileAccessForTesting::GetBlockImpl(unsigned long pos,
                                       unsigned char* pBuf,
                                       unsigned long size) {
  memcpy(pBuf, file_contents_.get() + pos, size);
  return size;
}

// static
int FileAccessForTesting::SGetBlock(void* param,
                                    unsigned long pos,
                                    unsigned char* pBuf,
                                    unsigned long size) {
  auto* file_access = static_cast<FileAccessForTesting*>(param);
  return file_access->GetBlockImpl(pos, pBuf, size);
}
