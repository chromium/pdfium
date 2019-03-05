// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_TRAVERSESTRATEGY_CONTENTAREACONTAINERLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_TRAVERSESTRATEGY_CONTENTAREACONTAINERLAYOUTITEM_H_

#include "xfa/fxfa/layout/cxfa_containerlayoutitem.h"

class CXFA_TraverseStrategy_ContentAreaContainerLayoutItem {
 public:
  static CXFA_ContainerLayoutItem* GetFirstChild(
      CXFA_ContainerLayoutItem* pLayoutItem) {
    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetFirstChild(); pChildItem;
         pChildItem = pChildItem->GetNextSibling()) {
      if (CXFA_ContainerLayoutItem* pContainer =
              pChildItem->AsContainerLayoutItem()) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ContainerLayoutItem* GetNextSibling(
      CXFA_ContainerLayoutItem* pLayoutItem) {
    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetNextSibling();
         pChildItem; pChildItem = pChildItem->GetNextSibling()) {
      if (CXFA_ContainerLayoutItem* pContainer =
              pChildItem->AsContainerLayoutItem()) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ContainerLayoutItem* GetParent(
      CXFA_ContainerLayoutItem* pLayoutItem) {
    return ToContainerLayoutItem(pLayoutItem->GetParent());
  }
};

#endif  // XFA_FXFA_LAYOUT_CXFA_TRAVERSESTRATEGY_CONTENTAREACONTAINERLAYOUTITEM_H_
