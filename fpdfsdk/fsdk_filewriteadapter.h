// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FSDK_FILEWRITEADAPTER_H_
#define FPDFSDK_FSDK_FILEWRITEADAPTER_H_

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"
#include "public/fpdf_save.h"

class FSDK_FileWriteAdapter : public IFX_WriteStream {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  bool WriteBlock(const void* data, size_t size) override;
  bool WriteString(const ByteStringView& str) override;

 private:
  explicit FSDK_FileWriteAdapter(FPDF_FILEWRITE* fileWriteStruct);
  ~FSDK_FileWriteAdapter() override;

  FPDF_FILEWRITE* fileWriteStruct_;
};

#endif  // FPDFSDK_FSDK_FILEWRITEADAPTER_H_
