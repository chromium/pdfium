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
IXFA_DocLayout* IXFA_LayoutPage::GetLayout() const
{
    CXFA_ContainerLayoutItemImpl* pThis = (CXFA_ContainerLayoutItemImpl*)this;
    return pThis->m_pFormNode->GetDocument()->GetLayoutProcessor();
}
FX_INT32 IXFA_LayoutPage::GetPageIndex() const
{
    CXFA_ContainerLayoutItemImpl* pThis = (CXFA_ContainerLayoutItemImpl*)this;
    return pThis->m_pFormNode->GetDocument()->GetLayoutProcessor()->GetLayoutPageMgr()->GetPageIndex((IXFA_LayoutPage*)this);
}
void IXFA_LayoutPage::GetPageSize(CFX_SizeF &size)
{
    CXFA_ContainerLayoutItemImpl* pThis = (CXFA_ContainerLayoutItemImpl*)this;
    size.Set(0, 0);
    CXFA_Node *pMedium = pThis->m_pFormNode->GetFirstChildByClass(XFA_ELEMENT_Medium);
    if (pMedium) {
        size.x = pMedium->GetMeasure(XFA_ATTRIBUTE_Short).ToUnit(XFA_UNIT_Pt);
        size.y = pMedium->GetMeasure(XFA_ATTRIBUTE_Long).ToUnit(XFA_UNIT_Pt);
        if (pMedium->GetEnum(XFA_ATTRIBUTE_Orientation) == XFA_ATTRIBUTEENUM_Landscape) {
            size.Set(size.y, size.x);
        }
    }
}
CXFA_Node* IXFA_LayoutPage::GetMasterPage() const
{
    CXFA_ContainerLayoutItemImpl* pThis = (CXFA_ContainerLayoutItemImpl*)this;
    return pThis->m_pFormNode;
}
IXFA_LayoutPage*	CXFA_LayoutItem::GetPage() const
{
    CXFA_ContainerLayoutItemImpl* pThis = (CXFA_ContainerLayoutItemImpl*)this;
    for(CXFA_LayoutItemImpl* pCurNode = pThis; pCurNode; pCurNode = pCurNode->m_pParent) {
        if(pCurNode->m_pFormNode->GetClassID() == XFA_ELEMENT_PageArea) {
            return (IXFA_LayoutPage*)pCurNode;
        }
    }
    return NULL;
}
CXFA_Node* CXFA_LayoutItem::GetFormNode() const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    return pThis->m_pFormNode;
}
void CXFA_LayoutItem::GetRect(CFX_RectF &rtLayout, FX_BOOL bRelative) const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    CFX_PointF sPos  = pThis->m_sPos;
    CFX_SizeF  sSize = pThis->m_sSize;
    if (!bRelative) {
        for(CXFA_LayoutItemImpl* pLayoutItem = pThis->m_pParent; pLayoutItem; pLayoutItem = pLayoutItem->m_pParent) {
            if(pLayoutItem->IsContentLayoutItem()) {
                sPos += ((CXFA_ContentLayoutItemImpl*)pLayoutItem)->m_sPos;
                if(CXFA_Node* pMarginNode = pLayoutItem->m_pFormNode->GetFirstChildByClass(XFA_ELEMENT_Margin)) {
                    sPos.Add(pMarginNode->GetMeasure(XFA_ATTRIBUTE_LeftInset).ToUnit(XFA_UNIT_Pt), pMarginNode->GetMeasure(XFA_ATTRIBUTE_TopInset).ToUnit(XFA_UNIT_Pt));
                }
            } else {
                if(pLayoutItem->m_pFormNode->GetClassID() == XFA_ELEMENT_ContentArea) {
                    sPos.Add(pLayoutItem->m_pFormNode->GetMeasure(XFA_ATTRIBUTE_X).ToUnit(XFA_UNIT_Pt), pLayoutItem->m_pFormNode->GetMeasure(XFA_ATTRIBUTE_Y).ToUnit(XFA_UNIT_Pt));
                    break;
                } else if(pLayoutItem->m_pFormNode->GetClassID() == XFA_ELEMENT_PageArea) {
                    break;
                }
            }
        }
    }
    rtLayout.Set(sPos.x, sPos.y, sSize.x, sSize.y);
}
CXFA_LayoutItem* CXFA_LayoutItem::GetParent() const
{
    CXFA_LayoutItemImpl* pThis = (CXFA_LayoutItemImpl*)this;
    return (CXFA_LayoutItem*)pThis->m_pParent;
}
CXFA_LayoutItem* CXFA_LayoutItem::GetFirst() const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    CXFA_ContentLayoutItemImpl* pCurNode = pThis;
    while(pCurNode->m_pPrev) {
        pCurNode = pCurNode->m_pPrev;
    }
    return (CXFA_LayoutItem*)pCurNode;
}
CXFA_LayoutItem* CXFA_LayoutItem::GetPrev() const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    return (CXFA_LayoutItem*)pThis->m_pPrev;
}
CXFA_LayoutItem* CXFA_LayoutItem::GetNext() const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    return (CXFA_LayoutItem*)pThis->m_pNext;
}
CXFA_LayoutItem* CXFA_LayoutItem::GetLast() const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    CXFA_ContentLayoutItemImpl* pCurNode = pThis;
    while(pCurNode->m_pNext) {
        pCurNode = pCurNode->m_pNext;
    }
    return (CXFA_LayoutItem*)pCurNode;
}
FX_INT32 CXFA_LayoutItem::GetIndex() const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    FX_INT32 iIndex = 0;
    CXFA_ContentLayoutItemImpl* pCurNode = pThis;
    while(pCurNode->m_pPrev) {
        pCurNode = pCurNode->m_pPrev;
        iIndex++;
    }
    return iIndex;
}
FX_INT32 CXFA_LayoutItem::GetCount() const
{
    CXFA_ContentLayoutItemImpl* pThis = (CXFA_ContentLayoutItemImpl*)this;
    FX_INT32 iCount = 1;
    CXFA_ContentLayoutItemImpl* pCurNode = NULL;
    pCurNode = pThis;
    while(pCurNode->m_pPrev) {
        pCurNode = pCurNode->m_pPrev;
        iCount++;
    }
    pCurNode = pThis;
    while(pCurNode->m_pNext) {
        pCurNode = pCurNode->m_pNext;
        iCount++;
    }
    return iCount;
}
FX_DWORD XFA_GetRelevant(CXFA_Node* pFormItem, FX_DWORD dwParentRelvant)
{
    FX_DWORD dwRelevant = XFA_LAYOUTSTATUS_Viewable | XFA_LAYOUTSTATUS_Printable;
    CFX_WideStringC wsRelevant;
    if (pFormItem->TryCData(XFA_ATTRIBUTE_Relevant, wsRelevant)) {
        if (wsRelevant == FX_WSTRC(L"+print") || wsRelevant == FX_WSTRC(L"print")) {
            dwRelevant &= ~XFA_LAYOUTSTATUS_Viewable;
        } else if (wsRelevant == FX_WSTRC(L"-print")) {
            dwRelevant  &= ~XFA_LAYOUTSTATUS_Printable;
        }
    }
    if (!(dwParentRelvant & XFA_LAYOUTSTATUS_Viewable) && (dwRelevant != XFA_LAYOUTSTATUS_Viewable)) {
        dwRelevant &= ~XFA_LAYOUTSTATUS_Viewable;
    }
    if (!(dwParentRelvant & XFA_LAYOUTSTATUS_Printable) && (dwRelevant != XFA_LAYOUTSTATUS_Printable)) {
        dwRelevant &= ~XFA_LAYOUTSTATUS_Printable;
    }
    return dwRelevant;
}
void XFA_ReleaseLayoutItem(CXFA_LayoutItemImpl *pLayoutItem)
{
    CXFA_LayoutItemImpl *pNext, *pNode = pLayoutItem->m_pFirstChild;
    while (pNode) {
        pNext = pNode->m_pNextSibling;
        pNode->m_pParent = NULL;
        XFA_ReleaseLayoutItem(pNode);
        pNode = pNext;
    }
    delete pLayoutItem;
}
