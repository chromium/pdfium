// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fsdk_filewriteadapter.h"

FSDK_FileWriteAdapter::FSDK_FileWriteAdapter(FPDF_FILEWRITE* fileWriteStruct)
    : fileWriteStruct_(fileWriteStruct) {
  ASSERT(fileWriteStruct_);
}

FSDK_FileWriteAdapter::~FSDK_FileWriteAdapter() {}

bool FSDK_FileWriteAdapter::WriteBlock(const void* data, size_t size) {
  return fileWriteStruct_->WriteBlock(fileWriteStruct_, data, size) != 0;
}

bool FSDK_FileWriteAdapter::WriteString(const ByteStringView& str) {
  return WriteBlock(str.unterminated_c_str(), str.GetLength());
}
