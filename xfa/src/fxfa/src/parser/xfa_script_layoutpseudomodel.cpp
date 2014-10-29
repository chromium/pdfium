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
#include "xfa_script_layoutpseudomodel.h"
#include "xfa_document_layout_imp.h"
#include "xfa_layout_appadapter.h"
CScript_LayoutPseudoModel::CScript_LayoutPseudoModel(CXFA_Document* pDocument)
    : CXFA_OrdinaryObject(pDocument, XFA_ELEMENT_LayoutPseudoModel)
{
    m_uScriptHash = XFA_HASHCODE_Layout;
}
CScript_LayoutPseudoModel::~CScript_LayoutPseudoModel()
{
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_Ready(FXJSE_HVALUE hValue, FX_BOOL bSetting, XFA_ATTRIBUTE eAttribute)
{
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
        return;
    }
    XFA_HDOC hDoc = pNotify->GetHDOC();
    if (bSetting) {
        ThrowScriptErrorMessage(XFA_IDS_UNABLE_SET_READY);
        return;
    }
    FX_INT32 iStatus = pNotify->GetLayoutStatus();
    FXJSE_Value_SetBoolean(hValue, iStatus >= 2);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_HWXY(CFXJSE_Arguments* pArguments, XFA_LAYOUTMODEL_HWXY layoutModel)
{
    FX_INT32 iLength = pArguments->GetLength();
    if ( iLength < 1 || iLength > 3) {
        FX_LPCWSTR methodName = NULL;
        switch (layoutModel) {
            case XFA_LAYOUTMODEL_H:
                methodName = (FX_LPCWSTR)(L"h");
                break;
            case XFA_LAYOUTMODEL_W:
                methodName = (FX_LPCWSTR)(L"w");
                break;
            case XFA_LAYOUTMODEL_X:
                methodName = (FX_LPCWSTR)(L"x");
                break;
            case XFA_LAYOUTMODEL_Y:
                methodName = (FX_LPCWSTR)(L"y");
                break;
        }
        ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, methodName);
        return;
    }
    CXFA_Node* pNode = NULL;
    CFX_WideString wsUnit = FX_WSTRC(L"pt");
    FX_INT32 iIndex = 0;
    if (iLength >= 1) {
        pNode = (CXFA_Node*)pArguments->GetObject(0);
    }
    if (iLength >= 2) {
        CFX_ByteString bsUnit = pArguments->GetUTF8String(1);
        if (!bsUnit.IsEmpty()) {
            wsUnit = CFX_WideString::FromUTF8(bsUnit, bsUnit.GetLength());
        }
    }
    if (iLength >= 3) {
        iIndex = pArguments->GetInt32(2);
    }
    if (!pNode) {
        return;
    }
    IXFA_DocLayout* pDocLayout = m_pDocument->GetDocLayout();
    if (!pDocLayout) {
        return;
    }
    CFX_RectF rtRect;
    CXFA_Measurement measure;
    CXFA_LayoutItem* pLayoutItem = pDocLayout->GetLayoutItem(pNode);
    if (!pLayoutItem) {
        return;
    }
    while (iIndex > 0 && pLayoutItem) {
        pLayoutItem = pLayoutItem->GetNext();
        iIndex --;
    }
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    if (!pLayoutItem) {
        FXJSE_Value_SetFloat(hValue, 0);
        return;
    }
    pLayoutItem->GetRect(rtRect, TRUE);
    switch (layoutModel) {
        case XFA_LAYOUTMODEL_H:
            measure.Set(rtRect.height, XFA_UNIT_Pt);
            break;
        case XFA_LAYOUTMODEL_W:
            measure.Set(rtRect.width, XFA_UNIT_Pt);
            break;
        case XFA_LAYOUTMODEL_X:
            measure.Set(rtRect.left, XFA_UNIT_Pt);
            break;
        case XFA_LAYOUTMODEL_Y:
            measure.Set(rtRect.top, XFA_UNIT_Pt);
            break;
    }
    XFA_UNIT unit =	measure.GetUnit(wsUnit);
    FX_FLOAT fValue = measure.ToUnit(unit);
    fValue = FXSYS_round(fValue * 1000) / 1000.0f;
    if (hValue) {
        FXJSE_Value_SetFloat(hValue, fValue);
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_H(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_HWXY(pArguments, XFA_LAYOUTMODEL_H);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_W(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_HWXY(pArguments, XFA_LAYOUTMODEL_W);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_X(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_HWXY(pArguments, XFA_LAYOUTMODEL_X);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_Y(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_HWXY(pArguments, XFA_LAYOUTMODEL_Y);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_NumberedPageCount(CFXJSE_Arguments* pArguments, FX_BOOL bNumbered)
{
    IXFA_DocLayout* pDocLayout = m_pDocument->GetDocLayout();
    if (!pDocLayout) {
        return;
    }
    FX_INT32 iPageCount = 0;
    FX_INT32 iPageNum = pDocLayout->CountPages();
    if (bNumbered) {
        for (FX_INT32 i = 0; i < iPageNum; i++) {
            IXFA_LayoutPage* pLayoutPage = pDocLayout->GetPage(i);
            if (!pLayoutPage) {
                continue;
            }
            CXFA_Node* pMasterPage = pLayoutPage->GetMasterPage();
            if (pMasterPage->GetInteger(XFA_ATTRIBUTE_Numbered)) {
                iPageCount++;
            }
        }
    } else {
        iPageCount = iPageNum;
    }
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    if (hValue) {
        FXJSE_Value_SetInteger(hValue, iPageCount);
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_PageCount(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_NumberedPageCount(pArguments, TRUE);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_PageSpan(CFXJSE_Arguments* pArguments)
{
    FX_INT32 iLength = pArguments->GetLength();
    if (iLength != 1) {
        ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, (FX_LPCWSTR)(L"pageSpan"));
        return;
    }
    CXFA_Node* pNode = NULL;
    if (iLength >= 1) {
        pNode = (CXFA_Node*)pArguments->GetObject(0);
    }
    if (!pNode) {
        return;
    }
    IXFA_DocLayout* pDocLayout = m_pDocument->GetDocLayout();
    if (!pDocLayout) {
        return;
    }
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    CXFA_LayoutItem* pLayoutItem = pDocLayout->GetLayoutItem(pNode);
    if (!pLayoutItem) {
        FXJSE_Value_SetInteger(hValue, -1);
        return;
    }
    FX_INT32 iLast = pLayoutItem->GetLast()->GetPage()->GetPageIndex();
    FX_INT32 iFirst = pLayoutItem->GetFirst()->GetPage()->GetPageIndex();
    FX_INT32 iPageSpan = iLast - iFirst + 1;
    if (hValue) {
        FXJSE_Value_SetInteger(hValue, iPageSpan);
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_Page(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_PageImp(pArguments, FALSE);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_GetObjArray(IXFA_DocLayout* pDocLayout, FX_INT32 iPageNo, const CFX_WideString &wsType, FX_BOOL bOnPageArea, CXFA_NodeArray &retArray)
{
    CXFA_ContainerLayoutItemImpl* pLayoutPage = (CXFA_ContainerLayoutItemImpl*)pDocLayout->GetPage(iPageNo);
    if (!pLayoutPage) {
        return;
    }
    if (wsType == FX_WSTRC(L"pageArea")) {
        if (CXFA_Node* pMasterPage = pLayoutPage->m_pFormNode) {
            retArray.Add(pMasterPage);
        }
        return;
    }
    if (wsType == FX_WSTRC(L"contentArea")) {
        for(CXFA_LayoutItemImpl* pItem = pLayoutPage->m_pFirstChild; pItem; pItem = pItem->m_pNextSibling) {
            if(pItem->m_pFormNode->GetClassID() == XFA_ELEMENT_ContentArea) {
                retArray.Add(pItem->m_pFormNode);
            }
        }
        return;
    }
    CFX_MapPtrToPtr formItems;
    formItems.InitHashTable(256, TRUE);
    if (wsType.IsEmpty()) {
        if (CXFA_Node* pMasterPage = pLayoutPage->m_pFormNode) {
            retArray.Add(pMasterPage);
        }
        for(CXFA_LayoutItemImpl* pItem = pLayoutPage->m_pFirstChild; pItem; pItem = pItem->m_pNextSibling) {
            if(pItem->m_pFormNode->GetClassID() == XFA_ELEMENT_ContentArea) {
                retArray.Add(pItem->m_pFormNode);
                if(!bOnPageArea) {
                    CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItemImpl, CXFA_TraverseStrategy_ContentLayoutItem> iterator((CXFA_ContentLayoutItemImpl*)pItem->m_pFirstChild);
                    for(CXFA_ContentLayoutItemImpl* pItemChild = iterator.GetCurrent(); pItemChild; pItemChild = iterator.MoveToNext()) {
                        if(!pItemChild->IsContentLayoutItem()) {
                            continue;
                        }
                        XFA_ELEMENT eElementType = pItemChild->m_pFormNode->GetClassID();
                        if (eElementType != XFA_ELEMENT_Field && eElementType != XFA_ELEMENT_Draw
                                && eElementType != XFA_ELEMENT_Subform && eElementType != XFA_ELEMENT_Area) {
                            continue;
                        }
                        if (formItems.GetValueAt(pItemChild->m_pFormNode)) {
                            continue;
                        }
                        formItems.SetAt(pItemChild->m_pFormNode, this);
                        retArray.Add(pItemChild->m_pFormNode);
                    }
                }
            } else {
                if(bOnPageArea) {
                    CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItemImpl, CXFA_TraverseStrategy_ContentLayoutItem> iterator((CXFA_ContentLayoutItemImpl*)pItem);
                    for(CXFA_ContentLayoutItemImpl* pItemChild = iterator.GetCurrent(); pItemChild; pItemChild = iterator.MoveToNext()) {
                        if(!pItemChild->IsContentLayoutItem()) {
                            continue;
                        }
                        XFA_ELEMENT eElementType = pItemChild->m_pFormNode->GetClassID();
                        if (eElementType != XFA_ELEMENT_Field && eElementType != XFA_ELEMENT_Draw
                                && eElementType != XFA_ELEMENT_Subform && eElementType != XFA_ELEMENT_Area) {
                            continue;
                        }
                        if (formItems.GetValueAt(pItemChild->m_pFormNode)) {
                            continue;
                        }
                        formItems.SetAt(pItemChild->m_pFormNode, this);
                        retArray.Add(pItemChild->m_pFormNode);
                    }
                }
            }
        }
        return;
    }
    XFA_ELEMENT eType = XFA_ELEMENT_UNKNOWN;
    if (wsType == FX_WSTRC(L"field")) {
        eType = XFA_ELEMENT_Field;
    } else if (wsType == FX_WSTRC(L"draw")) {
        eType = XFA_ELEMENT_Draw;
    } else if (wsType == FX_WSTRC(L"subform")) {
        eType = XFA_ELEMENT_Subform;
    } else if (wsType == FX_WSTRC(L"area")) {
        eType = XFA_ELEMENT_Area;
    }
    if (eType != XFA_ELEMENT_UNKNOWN) {
        for(CXFA_LayoutItemImpl* pItem = pLayoutPage->m_pFirstChild; pItem; pItem = pItem->m_pNextSibling) {
            if(pItem->m_pFormNode->GetClassID() == XFA_ELEMENT_ContentArea) {
                if(!bOnPageArea) {
                    CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItemImpl, CXFA_TraverseStrategy_ContentLayoutItem> iterator((CXFA_ContentLayoutItemImpl*)pItem->m_pFirstChild);
                    for(CXFA_ContentLayoutItemImpl* pItemChild = iterator.GetCurrent(); pItemChild; pItemChild = iterator.MoveToNext()) {
                        if(!pItemChild->IsContentLayoutItem()) {
                            continue;
                        }
                        if (pItemChild->m_pFormNode->GetClassID() != eType) {
                            continue;
                        }
                        if (formItems.GetValueAt(pItemChild->m_pFormNode)) {
                            continue;
                        }
                        formItems.SetAt(pItemChild->m_pFormNode, this);
                        retArray.Add(pItemChild->m_pFormNode);
                    }
                }
            } else {
                if(bOnPageArea) {
                    CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItemImpl, CXFA_TraverseStrategy_ContentLayoutItem> iterator((CXFA_ContentLayoutItemImpl*)pItem);
                    for(CXFA_ContentLayoutItemImpl* pItemChild = iterator.GetCurrent(); pItemChild; pItemChild = iterator.MoveToNext()) {
                        if(!pItemChild->IsContentLayoutItem()) {
                            continue;
                        }
                        if (pItemChild->m_pFormNode->GetClassID() != eType) {
                            continue;
                        }
                        if (formItems.GetValueAt(pItemChild->m_pFormNode)) {
                            continue;
                        }
                        formItems.SetAt(pItemChild->m_pFormNode, this);
                        retArray.Add(pItemChild->m_pFormNode);
                    }
                }
            }
        }
        return;
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_PageContent(CFXJSE_Arguments* pArguments)
{
    FX_INT32 iLength = pArguments->GetLength();
    if (iLength < 1 || iLength > 3) {
        ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, (FX_LPCWSTR)(L"pageContent"));
        return;
    }
    FX_INT32 iIndex = 0;
    CFX_WideString wsType;
    FX_BOOL bOnPageArea = FALSE;
    if (iLength >= 1) {
        iIndex = pArguments->GetInt32(0);
    }
    if (iLength >= 2) {
        CFX_ByteString bsType = pArguments->GetUTF8String(1);
        wsType = CFX_WideString::FromUTF8(bsType, bsType.GetLength());
    }
    if (iLength >= 3) {
        bOnPageArea = pArguments->GetInt32(2) == 0 ? FALSE : TRUE;
    }
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
        return;
    }
    IXFA_DocLayout* pDocLayout = m_pDocument->GetDocLayout();
    if (!pDocLayout) {
        return;
    }
    CXFA_NodeArray retArray;
    Script_LayoutPseudoModel_GetObjArray(pDocLayout, iIndex, wsType, bOnPageArea, retArray);
    CXFA_ArrayNodeList* pArrayNodeList = FX_NEW CXFA_ArrayNodeList(m_pDocument);
    pArrayNodeList->SetArrayNodeList(retArray);
    FXJSE_Value_SetObject(pArguments->GetReturnValue(), (CXFA_Object*)pArrayNodeList, m_pDocument->GetScriptContext()->GetJseNormalClass());
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_AbsPageCount(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_NumberedPageCount(pArguments, FALSE);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_AbsPageCountInBatch(CFXJSE_Arguments* pArguments)
{
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
        return;
    }
    XFA_HDOC hDoc = pNotify->GetHDOC();
    FX_INT32 iPageCount = pNotify->GetDocProvider()->AbsPageCountInBatch(hDoc);
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    if (hValue) {
        FXJSE_Value_SetInteger(hValue, iPageCount);
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_SheetCountInBatch(CFXJSE_Arguments* pArguments)
{
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
        return;
    }
    XFA_HDOC hDoc = pNotify->GetHDOC();
    FX_INT32 iPageCount = pNotify->GetDocProvider()->SheetCountInBatch(hDoc);
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    if (hValue) {
        FXJSE_Value_SetInteger(hValue, iPageCount);
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_Relayout(CFXJSE_Arguments* pArguments)
{
    CXFA_Node* pRootNode = m_pDocument->GetRoot();
    CXFA_Node* pFormRoot = pRootNode->GetFirstChildByClass(XFA_ELEMENT_Form);
    FXSYS_assert(pFormRoot);
    CXFA_Node* pContentRootNode = pFormRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
    CXFA_LayoutProcessor* pLayoutProcessor = m_pDocument->GetLayoutProcessor();
    if (pContentRootNode) {
        pLayoutProcessor->AddChangedContainer(pContentRootNode);
    }
    pLayoutProcessor->SetForceReLayout(TRUE);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_AbsPageSpan(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_PageSpan(pArguments);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_AbsPageInBatch(CFXJSE_Arguments* pArguments)
{
    FX_INT32 iLength = pArguments->GetLength();
    if (iLength != 1) {
        ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, (FX_LPCWSTR)(L"absPageInBatch"));
        return;
    }
    CXFA_Node* pNode = NULL;
    if (iLength >= 1) {
        pNode = (CXFA_Node*)pArguments->GetObject(0);
    }
    if (!pNode) {
        return;
    }
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
        return;
    }
    IXFA_DocLayout* pDocLayout = m_pDocument->GetDocLayout();
    if (!pDocLayout) {
        return;
    }
    XFA_HWIDGET hWidget = pNotify->GetHWidget(pDocLayout->GetLayoutItem(pNode));
    if (!hWidget) {
        return;
    }
    XFA_HDOC hDoc = pNotify->GetHDOC();
    FX_INT32 iPageCount = pNotify->GetDocProvider()->AbsPageInBatch(hDoc, hWidget);
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    if (hValue) {
        FXJSE_Value_SetInteger(hValue, iPageCount);
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_SheetInBatch(CFXJSE_Arguments* pArguments)
{
    FX_INT32 iLength = pArguments->GetLength();
    if (iLength != 1) {
        ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, (FX_LPCWSTR)(L"sheetInBatch"));
        return;
    }
    CXFA_Node* pNode = NULL;
    if (iLength >= 1) {
        pNode = (CXFA_Node*)pArguments->GetObject(0);
    }
    if (!pNode) {
        return;
    }
    IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
    if (!pNotify) {
        return;
    }
    IXFA_DocLayout* pDocLayout = m_pDocument->GetDocLayout();
    if (!pDocLayout) {
        return;
    }
    XFA_HWIDGET hWidget = pNotify->GetHWidget(pDocLayout->GetLayoutItem(pNode));
    if (!hWidget) {
        return;
    }
    XFA_HDOC hDoc = pNotify->GetHDOC();
    FX_INT32 iPageCount = pNotify->GetDocProvider()->SheetInBatch(hDoc, hWidget);
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    if (hValue) {
        FXJSE_Value_SetInteger(hValue, iPageCount);
    }
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_Sheet(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_PageImp(pArguments, TRUE);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_RelayoutPageArea(CFXJSE_Arguments* pArguments)
{
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_SheetCount(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_NumberedPageCount(pArguments, FALSE);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_AbsPage(CFXJSE_Arguments* pArguments)
{
    Script_LayoutPseudoModel_PageImp(pArguments, TRUE);
}
void CScript_LayoutPseudoModel::Script_LayoutPseudoModel_PageImp(CFXJSE_Arguments* pArguments, FX_BOOL bAbsPage)
{
    FX_INT32 iLength = pArguments->GetLength();
    if (iLength != 1) {
        FX_LPCWSTR methodName;
        if (bAbsPage) {
            methodName = (FX_LPCWSTR)(L"absPage");
        } else {
            methodName = (FX_LPCWSTR)(L"page");
        }
        ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, methodName);
        return;
    }
    CXFA_Node* pNode = NULL;
    if (iLength >= 1) {
        pNode = (CXFA_Node*)pArguments->GetObject(0);
    }
    FX_INT32 iPage = 0;
    FXJSE_HVALUE hValue =  pArguments->GetReturnValue();
    if (!pNode && hValue) {
        FXJSE_Value_SetInteger(hValue, iPage);
    }
    IXFA_DocLayout* pDocLayout = m_pDocument->GetDocLayout();
    if (!pDocLayout) {
        return;
    }
    CXFA_LayoutItem* pLayoutItem = pDocLayout->GetLayoutItem(pNode);
    if (!pLayoutItem) {
        FXJSE_Value_SetInteger(hValue, -1);
        return;
    }
    iPage = pLayoutItem->GetFirst()->GetPage()->GetPageIndex();
    if (hValue) {
        FXJSE_Value_SetInteger(hValue, bAbsPage ? iPage : iPage + 1);
    }
}
