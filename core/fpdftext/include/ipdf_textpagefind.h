// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFTEXT_INCLUDE_IPDF_TEXTPAGEFIND_H_
#define CORE_FPDFTEXT_INCLUDE_IPDF_TEXTPAGEFIND_H_

#include "core/fpdftext/include/ipdf_textpage.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"

class IPDF_TextPageFind {
 public:
  static IPDF_TextPageFind* CreatePageFind(const IPDF_TextPage* pTextPage);
  virtual ~IPDF_TextPageFind() {}

  virtual FX_BOOL FindFirst(const CFX_WideString& findwhat,
                            int flags,
                            int startPos = 0) = 0;
  virtual FX_BOOL FindNext() = 0;
  virtual FX_BOOL FindPrev() = 0;
  virtual void GetRectArray(CFX_RectArray& rects) const = 0;
  virtual int GetCurOrder() const = 0;
  virtual int GetMatchedCount() const = 0;
};

#endif  // CORE_FPDFTEXT_INCLUDE_IPDF_TEXTPAGEFIND_H_
