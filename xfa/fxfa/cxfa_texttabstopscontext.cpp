// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_texttabstopscontext.h"

#include <algorithm>

CXFA_TextTabstopsContext::CXFA_TextTabstopsContext() = default;

CXFA_TextTabstopsContext::~CXFA_TextTabstopsContext() = default;

void CXFA_TextTabstopsContext::Append(uint32_t dwAlign, float fTabstops) {
  XFA_TABSTOPS tabstop;
  tabstop.dwAlign = dwAlign;
  tabstop.fTabstops = fTabstops;

  auto it = std::lower_bound(tabstops_.begin(), tabstops_.end(), tabstop);
  tabstops_.insert(it, tabstop);
}

void CXFA_TextTabstopsContext::RemoveAll() {
  tabstops_.clear();
}

void CXFA_TextTabstopsContext::Reset() {
  tab_index_ = -1;
  has_tabstops_ = false;
  tab_width_ = 0;
  left_ = 0;
}
