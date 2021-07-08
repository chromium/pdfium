// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/string_write_stream.h"

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/widestring.h"
#include "third_party/base/check_op.h"

StringWriteStream::StringWriteStream() = default;

StringWriteStream::~StringWriteStream() = default;

bool StringWriteStream::WriteBlock(const void* pData, size_t size) {
  stream_.write(static_cast<const char*>(pData), size);
  return true;
}
