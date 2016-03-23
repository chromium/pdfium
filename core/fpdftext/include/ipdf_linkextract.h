// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFTEXT_INCLUDE_IPDF_LINKEXTRACT_H_
#define CORE_FPDFTEXT_INCLUDE_IPDF_LINKEXTRACT_H_

#include "core/fpdftext/include/ipdf_textpage.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"

class IPDF_LinkExtract {
 public:
  static IPDF_LinkExtract* CreateLinkExtract();
  virtual ~IPDF_LinkExtract() {}

  virtual FX_BOOL ExtractLinks(const IPDF_TextPage* pTextPage) = 0;
  virtual int CountLinks() const = 0;
  virtual CFX_WideString GetURL(int index) const = 0;
  virtual void GetBoundedSegment(int index, int& start, int& count) const = 0;
  virtual void GetRects(int index, CFX_RectArray& rects) const = 0;
};

#endif  // CORE_FPDFTEXT_INCLUDE_IPDF_LINKEXTRACT_H_
