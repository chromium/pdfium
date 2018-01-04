// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_widgetacciterator.h"

#include "xfa/fxfa/cxfa_widgetacc.h"

CXFA_WidgetAccIterator::CXFA_WidgetAccIterator(CXFA_Node* pTravelRoot)
    : m_ContentIterator(pTravelRoot), m_pCurWidgetAcc(nullptr) {}

CXFA_WidgetAccIterator::~CXFA_WidgetAccIterator() {}

CXFA_WidgetAcc* CXFA_WidgetAccIterator::MoveToNext() {
  CXFA_Node* pItem = m_pCurWidgetAcc ? m_ContentIterator.MoveToNext()
                                     : m_ContentIterator.GetCurrent();
  while (pItem) {
    m_pCurWidgetAcc = pItem->GetWidgetAcc();
    if (m_pCurWidgetAcc)
      return m_pCurWidgetAcc.Get();
    pItem = m_ContentIterator.MoveToNext();
  }
  return nullptr;
}

void CXFA_WidgetAccIterator::SkipTree() {
  m_ContentIterator.SkipChildrenAndMoveToNext();
  m_pCurWidgetAcc = nullptr;
}
