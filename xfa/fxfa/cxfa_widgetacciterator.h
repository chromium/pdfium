// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_WIDGETACCITERATOR_H_
#define XFA_FXFA_CXFA_WIDGETACCITERATOR_H_

#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"

class CXFA_Node;
class CXFA_WidgetAcc;

class CXFA_WidgetAccIterator {
 public:
  explicit CXFA_WidgetAccIterator(CXFA_Node* pTravelRoot);
  ~CXFA_WidgetAccIterator();

  CXFA_WidgetAcc* MoveToNext();
  void SkipTree();

 private:
  CXFA_ContainerIterator m_ContentIterator;
  UnownedPtr<CXFA_WidgetAcc> m_pCurWidgetAcc;
};

#endif  // XFA_FXFA_CXFA_WIDGETACCITERATOR_H_
