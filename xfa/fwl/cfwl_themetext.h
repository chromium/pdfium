// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_THEMETEXT_H_
#define XFA_FWL_CFWL_THEMETEXT_H_

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "xfa/fde/cfde_data.h"
#include "xfa/fwl/cfwl_themepart.h"

class CFGAS_GEGraphics;

class CFWL_ThemeText final : public CFWL_ThemePart {
 public:
  CFWL_ThemeText(CFWL_Widget* pWidget, CFGAS_GEGraphics* pGraphics);
  ~CFWL_ThemeText();

  CFGAS_GEGraphics* GetGraphics() const { return m_pGraphics.Get(); }

  FDE_TextAlignment m_iTTOAlign = FDE_TextAlignment::kTopLeft;
  FDE_TextStyle m_dwTTOStyles;
  WideString m_wsText;

 private:
  UnownedPtr<CFGAS_GEGraphics> const m_pGraphics;
};

#endif  // XFA_FWL_CFWL_THEMETEXT_H_
