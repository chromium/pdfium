// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_EDIT_CPDF_STRINGARCHIVESTREAM_H_
#define CORE_FPDFAPI_EDIT_CPDF_STRINGARCHIVESTREAM_H_

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/stl_util.h"

class CPDF_StringArchiveStream final : public IFX_ArchiveStream {
 public:
  explicit CPDF_StringArchiveStream(fxcrt::ostringstream* stream);
  ~CPDF_StringArchiveStream() override;

  // IFX_ArchiveStream:
  bool WriteBlock(const void* pData, size_t size) override;
  FX_FILESIZE CurrentOffset() const override;

 private:
  fxcrt::ostringstream* stream_;
};

#endif  // CORE_FPDFAPI_EDIT_CPDF_STRINGARCHIVESTREAM_H_
