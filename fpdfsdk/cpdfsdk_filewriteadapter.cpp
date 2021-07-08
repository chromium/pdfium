// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_filewriteadapter.h"

#include "third_party/base/check.h"

CPDFSDK_FileWriteAdapter::CPDFSDK_FileWriteAdapter(FPDF_FILEWRITE* file_write)
    : file_write_(file_write) {
  DCHECK(file_write_);
}

CPDFSDK_FileWriteAdapter::~CPDFSDK_FileWriteAdapter() = default;

bool CPDFSDK_FileWriteAdapter::WriteBlock(const void* data, size_t size) {
  return file_write_->WriteBlock(file_write_.Get(), data, size) != 0;
}
