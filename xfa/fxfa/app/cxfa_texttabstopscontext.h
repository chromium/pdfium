// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTTABSTOPSCONTEXT_H_
#define XFA_FXFA_APP_CXFA_TEXTTABSTOPSCONTEXT_H_

#include "core/fxcrt/fx_basic.h"

struct XFA_TABSTOPS {
  uint32_t dwAlign;
  FX_FLOAT fTabstops;
};

class CXFA_TextTabstopsContext {
 public:
  CXFA_TextTabstopsContext();
  ~CXFA_TextTabstopsContext();

  void Append(uint32_t dwAlign, FX_FLOAT fTabstops);
  void RemoveAll();
  void Reset();

  CFX_ArrayTemplate<XFA_TABSTOPS> m_tabstops;
  int32_t m_iTabCount;
  int32_t m_iTabIndex;
  bool m_bTabstops;
  FX_FLOAT m_fTabWidth;
  FX_FLOAT m_fLeft;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTTABSTOPSCONTEXT_H_
