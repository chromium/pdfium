// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_THEMETEXT_H_
#define XFA_FWL_CFWL_THEMETEXT_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/cfwl_themepart.h"

class CFWL_ThemeText final : public CFWL_ThemePart {
 public:
  CFWL_ThemeText() = default;

  FDE_TextAlignment m_iTTOAlign = FDE_TextAlignment::kTopLeft;
  CXFA_Graphics* m_pGraphics = nullptr;
  WideString m_wsText;
  FDE_TextStyle m_dwTTOStyles;
};

#endif  // XFA_FWL_CFWL_THEMETEXT_H_
