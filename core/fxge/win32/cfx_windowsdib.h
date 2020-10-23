// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_CFX_WINDOWSDIB_H_
#define CORE_FXGE_WIN32_CFX_WINDOWSDIB_H_

#include "core/fxcrt/bytestring.h"
#include "core/fxge/dib/cfx_dibitmap.h"

class CFX_WindowsDIB {
 public:
  static ByteString GetBitmapInfo(const RetainPtr<CFX_DIBitmap>& pBitmap);

  CFX_WindowsDIB() = delete;
  CFX_WindowsDIB(const CFX_WindowsDIB&) = delete;
  CFX_WindowsDIB& operator=(const CFX_WindowsDIB&) = delete;
};

#endif  // CORE_FXGE_WIN32_CFX_WINDOWSDIB_H_
