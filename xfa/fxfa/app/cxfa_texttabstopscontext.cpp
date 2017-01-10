// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_texttabstopscontext.h"

CXFA_TextTabstopsContext::CXFA_TextTabstopsContext()
    : m_iTabCount(0),
      m_iTabIndex(-1),
      m_bTabstops(false),
      m_fTabWidth(0),
      m_fLeft(0) {}

CXFA_TextTabstopsContext::~CXFA_TextTabstopsContext() {}

void CXFA_TextTabstopsContext::Append(uint32_t dwAlign, FX_FLOAT fTabstops) {
  int32_t i = 0;
  for (i = 0; i < m_iTabCount; i++) {
    XFA_TABSTOPS* pTabstop = m_tabstops.GetDataPtr(i);
    if (fTabstops < pTabstop->fTabstops) {
      break;
    }
  }
  m_tabstops.InsertSpaceAt(i, 1);
  XFA_TABSTOPS tabstop;
  tabstop.dwAlign = dwAlign;
  tabstop.fTabstops = fTabstops;
  m_tabstops.SetAt(i, tabstop);
  m_iTabCount++;
}

void CXFA_TextTabstopsContext::RemoveAll() {
  m_tabstops.RemoveAll();
  m_iTabCount = 0;
}

void CXFA_TextTabstopsContext::Reset() {
  m_iTabIndex = -1;
  m_bTabstops = false;
  m_fTabWidth = 0;
  m_fLeft = 0;
}
