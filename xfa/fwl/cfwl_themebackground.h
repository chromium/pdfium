// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_THEMEBACKGROUND_H_
#define XFA_FWL_CFWL_THEMEBACKGROUND_H_

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fwl/cfwl_themepart.h"

class CFGAS_GEGraphics;
class CFGAS_GEPath;

class CFWL_ThemeBackground final : public CFWL_ThemePart {
 public:
  FX_STACK_ALLOCATED();

  CFWL_ThemeBackground(Part iPart,
                       CFWL_Widget* pWidget,
                       CFGAS_GEGraphics* pGraphics);
  ~CFWL_ThemeBackground();

  CFGAS_GEGraphics* GetGraphics() const { return m_pGraphics.Get(); }
  const CFGAS_GEPath* GetPath() const { return m_pPath.Get(); }
  void SetPath(const CFGAS_GEPath* pPath) { m_pPath = pPath; }

 private:
  UnownedPtr<const CFGAS_GEPath> m_pPath;
  UnownedPtr<CFGAS_GEGraphics> const m_pGraphics;
};

#endif  // XFA_FWL_CFWL_THEMEBACKGROUND_H_
