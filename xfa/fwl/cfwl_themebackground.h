// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_THEMEBACKGROUND_H_
#define XFA_FWL_CFWL_THEMEBACKGROUND_H_

#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fwl/cfwl_themepart.h"

class CFGAS_GEGraphics;
class CFGAS_GEPath;

class CFWL_ThemeBackground final : public CFWL_ThemePart {
 public:
  CFWL_ThemeBackground(CFWL_Widget* pWidget, CFGAS_GEGraphics* pGraphics);
  ~CFWL_ThemeBackground();

  CFGAS_GEGraphics* GetGraphics() const { return m_pGraphics.Get(); }

  UnownedPtr<const CFGAS_GEPath> m_pPath;

 private:
  UnownedPtr<CFGAS_GEGraphics> const m_pGraphics;
};

#endif  // XFA_FWL_CFWL_THEMEBACKGROUND_H_
