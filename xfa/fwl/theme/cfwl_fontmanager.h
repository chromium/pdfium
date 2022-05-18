// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_FONTMANAGER_H_
#define XFA_FWL_THEME_CFWL_FONTMANAGER_H_

#include "core/fxcrt/retain_ptr.h"

class CFGAS_GEFont;

class CFWL_FontManager final {
 public:
  static CFWL_FontManager* GetInstance();
  static void DestroyInstance();

  RetainPtr<CFGAS_GEFont> GetFWLFont();

 private:
  CFWL_FontManager();
  ~CFWL_FontManager();

  RetainPtr<CFGAS_GEFont> m_pFWLFont;
};

#endif  // XFA_FWL_THEME_CFWL_FONTMANAGER_H_
