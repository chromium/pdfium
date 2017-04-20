// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_crtfileaccess.h"

CFX_CRTFileAccess::CFX_CRTFileAccess(const CFX_WideStringC& wsPath)
    : m_path(wsPath) {}

CFX_CRTFileAccess::~CFX_CRTFileAccess() {}

CFX_RetainPtr<IFX_SeekableStream> CFX_CRTFileAccess::CreateFileStream(
    uint32_t dwModes) {
  return IFX_SeekableStream::CreateFromFilename(m_path.c_str(), dwModes);
}
