// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_layout_appadapter.h"

#include "xfa/fxfa/app/xfa_ffnotify.h"
#include "xfa/fxfa/fm2js/xfa_fm2jsapi.h"
#include "xfa/fxfa/parser/xfa_docdata.h"
#include "xfa/fxfa/parser/xfa_doclayout.h"
#include "xfa/fxfa/parser/xfa_document.h"
#include "xfa/fxfa/parser/xfa_document_layout_imp.h"
#include "xfa/fxfa/parser/xfa_layout_itemlayout.h"
#include "xfa/fxfa/parser/xfa_layout_pagemgr_new.h"
#include "xfa/fxfa/parser/xfa_localemgr.h"
#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxfa/parser/xfa_parser.h"
#include "xfa/fxfa/parser/xfa_parser_imp.h"
#include "xfa/fxfa/parser/xfa_script.h"
#include "xfa/fxfa/parser/xfa_utils.h"

uint32_t XFA_GetRelevant(CXFA_Node* pFormItem, uint32_t dwParentRelvant) {
  uint32_t dwRelevant = XFA_LAYOUTSTATUS_Viewable | XFA_LAYOUTSTATUS_Printable;
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
  CXFA_FFNotify* pNotify =
      pLayoutItem->m_pFormNode->GetDocument()->GetParser()->GetNotify();
  CXFA_LayoutProcessor* pDocLayout =
      pLayoutItem->m_pFormNode->GetDocument()->GetDocLayout();
  while (pNode) {
    CXFA_LayoutItem* pNext = pNode->m_pNextSibling;
    pNode->m_pParent = nullptr;
    pNotify->OnLayoutEvent(pDocLayout, static_cast<CXFA_LayoutItem*>(pNode),
                           XFA_LAYOUTEVENT_ItemRemoving);
    XFA_ReleaseLayoutItem(pNode);
    pNode = pNext;
  }
  pNotify->OnLayoutEvent(pDocLayout, pLayoutItem, XFA_LAYOUTEVENT_ItemRemoving);
  if (pLayoutItem->m_pFormNode->GetClassID() == XFA_ELEMENT_PageArea) {
    pNotify->OnPageEvent(static_cast<CXFA_ContainerLayoutItem*>(pLayoutItem),
                         XFA_PAGEEVENT_PageRemoved);
  }
  delete pLayoutItem;
}
