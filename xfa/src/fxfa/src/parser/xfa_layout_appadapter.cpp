// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_utils.h"
#include "../common/xfa_object.h"
#include "../common/xfa_document.h"
#include "../common/xfa_parser.h"
#include "../common/xfa_script.h"
#include "../common/xfa_docdata.h"
#include "../common/xfa_doclayout.h"
#include "../common/xfa_debug.h"
#include "../common/xfa_localemgr.h"
#include "../common/xfa_fm2jsapi.h"
#include "xfa_debug_parser.h"
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
  CXFA_LayoutItem* pNext, * pNode = pLayoutItem->m_pFirstChild;
  while (pNode) {
    pNext = pNode->m_pNextSibling;
    pNode->m_pParent = NULL;
    XFA_ReleaseLayoutItem(pNode);
    pNode = pNext;
  }
  delete pLayoutItem;
}
