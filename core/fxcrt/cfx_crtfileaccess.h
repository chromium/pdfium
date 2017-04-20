// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_CRTFILEACCESS_H_
#define CORE_FXCRT_CFX_CRTFILEACCESS_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_string.h"

class CFX_CRTFileAccess : public CFX_Retainable {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  CFX_RetainPtr<IFX_SeekableStream> CreateFileStream(uint32_t dwModes);

 private:
  explicit CFX_CRTFileAccess(const CFX_WideStringC& wsPath);
  ~CFX_CRTFileAccess() override;

  CFX_WideString m_path;
};

#endif  // CORE_FXCRT_CFX_CRTFILEACCESS_H_
