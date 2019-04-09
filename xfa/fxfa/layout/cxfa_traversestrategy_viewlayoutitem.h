// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_TRAVERSESTRATEGY_VIEWLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_TRAVERSESTRATEGY_VIEWLAYOUTITEM_H_

#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"

class CXFA_TraverseStrategy_ViewLayoutItem {
 public:
  static CXFA_ViewLayoutItem* GetFirstChild(CXFA_ViewLayoutItem* pLayoutItem) {
    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetFirstChild(); pChildItem;
         pChildItem = pChildItem->GetNextSibling()) {
      if (CXFA_ViewLayoutItem* pContainer = pChildItem->AsViewLayoutItem()) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ViewLayoutItem* GetNextSibling(CXFA_ViewLayoutItem* pLayoutItem) {
    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetNextSibling();
         pChildItem; pChildItem = pChildItem->GetNextSibling()) {
      if (CXFA_ViewLayoutItem* pContainer = pChildItem->AsViewLayoutItem()) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ViewLayoutItem* GetParent(CXFA_ViewLayoutItem* pLayoutItem) {
    return ToViewLayoutItem(pLayoutItem->GetParent());
  }
};

#endif  // XFA_FXFA_LAYOUT_CXFA_TRAVERSESTRATEGY_VIEWLAYOUTITEM_H_
