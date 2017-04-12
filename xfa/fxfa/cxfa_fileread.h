// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FILEREAD_H_
#define XFA_FXFA_CXFA_FILEREAD_H_

#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_stream.h"

class CPDF_Stream;
class CPDF_StreamAcc;

class CXFA_FileRead : public IFX_SeekableReadStream {
 public:
  explicit CXFA_FileRead(const std::vector<CPDF_Stream*>& streams);
  ~CXFA_FileRead() override;

  // IFX_SeekableReadStream
  FX_FILESIZE GetSize() override;
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;

 private:
  std::vector<CFX_RetainPtr<CPDF_StreamAcc>> m_Data;
};

#endif  // XFA_FXFA_CXFA_FILEREAD_H_
