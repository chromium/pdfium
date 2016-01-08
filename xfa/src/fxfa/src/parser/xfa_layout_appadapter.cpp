// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_document_layout_imp.h"
#include "xfa_layout_itemlayout.h"
#include "xfa_layout_pagemgr_new.h"
#include "xfa_layout_appadapter.h"
FX_DWORD XFA_GetRelevant(CXFA_Node* pFormItem, FX_DWORD dwParentRelvant) {
  FX_DWORD dwRelevant = XFA_LAYOUTSTATUS_Viewable | XFA_LAYOUTSTATUS_Printable;
  CFX_WideStringC wsRelevant;
  if (pFormItem->TryCData(XFA_ATTRIBUTE_Relevant, wsRelevant)) {
    if (wsRelevant == FX_WSTRC(L"+print") || wsRelevant == FX_WSTRC(L"print")) {
      dwRelevant &= ~XFA_LAYOUTSTATUS_Viewable;
    } else if (wsRelevant == FX_WSTRC(L"-print")) {
      dwRelevant &= ~XFA_LAYOUTSTATUS_Printable;
    }
  }
  if (!(dwParentRelvant & XFA_LAYOUTSTATUS_Viewable) &&
      (dwRelevant != XFA_LAYOUTSTATUS_Viewable)) {
    dwRelevant &= ~XFA_LAYOUTSTATUS_Viewable;
  }
  if (!(dwParentRelvant & XFA_LAYOUTSTATUS_Printable) &&
      (dwRelevant != XFA_LAYOUTSTATUS_Printable)) {
    dwRelevant &= ~XFA_LAYOUTSTATUS_Printable;
  }
  return dwRelevant;
}
void XFA_ReleaseLayoutItem(CXFA_LayoutItem* pLayoutItem) {
  CXFA_LayoutItem* pNode = pLayoutItem->m_pFirstChild;
  while (pNode) {
    CXFA_LayoutItem* pNext = pNode->m_pNextSibling;
    pNode->m_pParent = nullptr;
    XFA_ReleaseLayoutItem(pNode);
    pNode = pNext;
  }
  IXFA_Notify* pNotify =
      pLayoutItem->m_pFormNode->GetDocument()->GetParser()->GetNotify();
  if (pLayoutItem->m_pFormNode->GetClassID() == XFA_ELEMENT_PageArea) {
    pNotify->OnPageEvent(static_cast<CXFA_ContainerLayoutItem*>(pLayoutItem),
                         XFA_PAGEEVENT_PageRemoved);
  }
  delete pLayoutItem;
}
