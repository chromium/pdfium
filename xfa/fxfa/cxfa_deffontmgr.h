// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_DEFFONTMGR_H_
#define XFA_FXFA_CXFA_DEFFONTMGR_H_

#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fgas/font/cfgas_gefont.h"

class CXFA_FFDoc;

class CXFA_DefFontMgr {
 public:
  CXFA_DefFontMgr();
  ~CXFA_DefFontMgr();

  CFX_RetainPtr<CFGAS_GEFont> GetFont(CXFA_FFDoc* hDoc,
                                      const CFX_WideStringC& wsFontFamily,
                                      uint32_t dwFontStyles,
                                      uint16_t wCodePage = 0xFFFF);
  CFX_RetainPtr<CFGAS_GEFont> GetDefaultFont(
      CXFA_FFDoc* hDoc,
      const CFX_WideStringC& wsFontFamily,
      uint32_t dwFontStyles,
      uint16_t wCodePage = 0xFFFF);

 private:
  std::vector<CFX_RetainPtr<CFGAS_GEFont>> m_CacheFonts;
};

#endif  // XFA_FXFA_CXFA_DEFFONTMGR_H_
