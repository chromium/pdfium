// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_layoutpseudomodel.h"

#include <set>

#include "core/fxcrt/fx_coordinates.h"
#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cscript_layoutpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_containerlayoutitem.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_layoutitem.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_contentlayoutitem.h"

CJX_LayoutPseudoModel::CJX_LayoutPseudoModel(CScript_LayoutPseudoModel* model)
    : CJX_Object(model) {}

CJX_LayoutPseudoModel::~CJX_LayoutPseudoModel() {}

void CJX_LayoutPseudoModel::Ready(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;
  if (bSetting) {
    ThrowException(L"Unable to set ready value.");
    return;
  }

  int32_t iStatus = pNotify->GetLayoutStatus();
  pValue->SetBoolean(iStatus >= 2);
}

void CJX_LayoutPseudoModel::HWXY(CFXJSE_Arguments* pArguments,
                                 XFA_LAYOUTMODEL_HWXY layoutModel) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    const wchar_t* methodName = nullptr;
    switch (layoutModel) {
      case XFA_LAYOUTMODEL_H:
        methodName = L"h";
        break;
      case XFA_LAYOUTMODEL_W:
        methodName = L"w";
        break;
      case XFA_LAYOUTMODEL_X:
        methodName = L"x";
        break;
      case XFA_LAYOUTMODEL_Y:
        methodName = L"y";
        break;
    }
    ThrowParamCountMismatchException(methodName);
    return;
  }

  CXFA_Node* pNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pNode)
    return;

  WideString wsUnit(L"pt");
  if (iLength >= 2) {
    ByteString bsUnit = pArguments->GetUTF8String(1);
    if (!bsUnit.IsEmpty())
      wsUnit = WideString::FromUTF8(bsUnit.AsStringView());
  }

  int32_t iIndex = iLength >= 3 ? pArguments->GetInt32(2) : 0;

  CXFA_LayoutProcessor* pDocLayout = GetDocument()->GetDocLayout();
  if (!pDocLayout)
    return;

  CXFA_LayoutItem* pLayoutItem = pDocLayout->GetLayoutItem(pNode);
  if (!pLayoutItem)
    return;

  while (iIndex > 0 && pLayoutItem) {
    pLayoutItem = pLayoutItem->GetNext();
    iIndex--;
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (!pLayoutItem) {
    pValue->SetFloat(0);
    return;
  }

  CXFA_Measurement measure;
  CFX_RectF rtRect = pLayoutItem->GetRect(true);
  switch (layoutModel) {
    case XFA_LAYOUTMODEL_H:
      measure.Set(rtRect.height, XFA_Unit::Pt);
      break;
    case XFA_LAYOUTMODEL_W:
      measure.Set(rtRect.width, XFA_Unit::Pt);
      break;
    case XFA_LAYOUTMODEL_X:
      measure.Set(rtRect.left, XFA_Unit::Pt);
      break;
    case XFA_LAYOUTMODEL_Y:
      measure.Set(rtRect.top, XFA_Unit::Pt);
      break;
  }

  float fValue = measure.ToUnit(
      CXFA_Measurement::GetUnitFromString(wsUnit.AsStringView()));
  fValue = FXSYS_round(fValue * 1000) / 1000.0f;
  pValue->SetFloat(fValue);
}

void CJX_LayoutPseudoModel::H(CFXJSE_Arguments* pArguments) {
  HWXY(pArguments, XFA_LAYOUTMODEL_H);
}

void CJX_LayoutPseudoModel::W(CFXJSE_Arguments* pArguments) {
  HWXY(pArguments, XFA_LAYOUTMODEL_W);
}

void CJX_LayoutPseudoModel::X(CFXJSE_Arguments* pArguments) {
  HWXY(pArguments, XFA_LAYOUTMODEL_X);
}

void CJX_LayoutPseudoModel::Y(CFXJSE_Arguments* pArguments) {
  HWXY(pArguments, XFA_LAYOUTMODEL_Y);
}

void CJX_LayoutPseudoModel::NumberedPageCount(CFXJSE_Arguments* pArguments,
                                              bool bNumbered) {
  CXFA_LayoutProcessor* pDocLayout = GetDocument()->GetDocLayout();
  if (!pDocLayout)
    return;

  int32_t iPageCount = 0;
  int32_t iPageNum = pDocLayout->CountPages();
  if (bNumbered) {
    for (int32_t i = 0; i < iPageNum; i++) {
      CXFA_ContainerLayoutItem* pLayoutPage = pDocLayout->GetPage(i);
      if (!pLayoutPage)
        continue;

      CXFA_Node* pMasterPage = pLayoutPage->GetMasterPage();
      if (pMasterPage->JSNode()->GetInteger(XFA_Attribute::Numbered))
        iPageCount++;
    }
  } else {
    iPageCount = iPageNum;
  }

  pArguments->GetReturnValue()->SetInteger(iPageCount);
}

void CJX_LayoutPseudoModel::PageCount(CFXJSE_Arguments* pArguments) {
  NumberedPageCount(pArguments, true);
}

void CJX_LayoutPseudoModel::PageSpan(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"pageSpan");
    return;
  }

  CXFA_Node* pNode = nullptr;
  if (iLength >= 1)
    pNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pNode)
    return;

  CXFA_LayoutProcessor* pDocLayout = GetDocument()->GetDocLayout();
  if (!pDocLayout)
    return;

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  CXFA_LayoutItem* pLayoutItem = pDocLayout->GetLayoutItem(pNode);
  if (!pLayoutItem) {
    pValue->SetInteger(-1);
    return;
  }

  int32_t iLast = pLayoutItem->GetLast()->GetPage()->GetPageIndex();
  int32_t iFirst = pLayoutItem->GetFirst()->GetPage()->GetPageIndex();
  int32_t iPageSpan = iLast - iFirst + 1;
  pValue->SetInteger(iPageSpan);
}

void CJX_LayoutPseudoModel::Page(CFXJSE_Arguments* pArguments) {
  PageInternals(pArguments, false);
}

std::vector<CXFA_Node*> CJX_LayoutPseudoModel::GetObjArray(
    CXFA_LayoutProcessor* pDocLayout,
    int32_t iPageNo,
    const WideString& wsType,
    bool bOnPageArea) {
  CXFA_ContainerLayoutItem* pLayoutPage = pDocLayout->GetPage(iPageNo);
  if (!pLayoutPage)
    return std::vector<CXFA_Node*>();

  std::vector<CXFA_Node*> retArray;
  if (wsType == L"pageArea") {
    if (pLayoutPage->m_pFormNode)
      retArray.push_back(pLayoutPage->m_pFormNode);
    return retArray;
  }
  if (wsType == L"contentArea") {
    for (CXFA_LayoutItem* pItem = pLayoutPage->m_pFirstChild; pItem;
         pItem = pItem->m_pNextSibling) {
      if (pItem->m_pFormNode->GetElementType() == XFA_Element::ContentArea)
        retArray.push_back(pItem->m_pFormNode);
    }
    return retArray;
  }
  std::set<CXFA_Node*> formItems;
  if (wsType.IsEmpty()) {
    if (pLayoutPage->m_pFormNode)
      retArray.push_back(pLayoutPage->m_pFormNode);

    for (CXFA_LayoutItem* pItem = pLayoutPage->m_pFirstChild; pItem;
         pItem = pItem->m_pNextSibling) {
      if (pItem->m_pFormNode->GetElementType() == XFA_Element::ContentArea) {
        retArray.push_back(pItem->m_pFormNode);
        if (!bOnPageArea) {
          CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItem,
                                    CXFA_TraverseStrategy_ContentLayoutItem>
          iterator(static_cast<CXFA_ContentLayoutItem*>(pItem->m_pFirstChild));
          for (CXFA_ContentLayoutItem* pItemChild = iterator.GetCurrent();
               pItemChild; pItemChild = iterator.MoveToNext()) {
            if (!pItemChild->IsContentLayoutItem()) {
              continue;
            }
            XFA_Element eType = pItemChild->m_pFormNode->GetElementType();
            if (eType != XFA_Element::Field && eType != XFA_Element::Draw &&
                eType != XFA_Element::Subform && eType != XFA_Element::Area) {
              continue;
            }
            if (pdfium::ContainsValue(formItems, pItemChild->m_pFormNode))
              continue;

            formItems.insert(pItemChild->m_pFormNode);
            retArray.push_back(pItemChild->m_pFormNode);
          }
        }
      } else {
        if (bOnPageArea) {
          CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItem,
                                    CXFA_TraverseStrategy_ContentLayoutItem>
          iterator(static_cast<CXFA_ContentLayoutItem*>(pItem));
          for (CXFA_ContentLayoutItem* pItemChild = iterator.GetCurrent();
               pItemChild; pItemChild = iterator.MoveToNext()) {
            if (!pItemChild->IsContentLayoutItem()) {
              continue;
            }
            XFA_Element eType = pItemChild->m_pFormNode->GetElementType();
            if (eType != XFA_Element::Field && eType != XFA_Element::Draw &&
                eType != XFA_Element::Subform && eType != XFA_Element::Area) {
              continue;
            }
            if (pdfium::ContainsValue(formItems, pItemChild->m_pFormNode))
              continue;
            formItems.insert(pItemChild->m_pFormNode);
            retArray.push_back(pItemChild->m_pFormNode);
          }
        }
      }
    }
    return retArray;
  }

  XFA_Element eType = XFA_Element::Unknown;
  if (wsType == L"field")
    eType = XFA_Element::Field;
  else if (wsType == L"draw")
    eType = XFA_Element::Draw;
  else if (wsType == L"subform")
    eType = XFA_Element::Subform;
  else if (wsType == L"area")
    eType = XFA_Element::Area;

  if (eType != XFA_Element::Unknown) {
    for (CXFA_LayoutItem* pItem = pLayoutPage->m_pFirstChild; pItem;
         pItem = pItem->m_pNextSibling) {
      if (pItem->m_pFormNode->GetElementType() == XFA_Element::ContentArea) {
        if (!bOnPageArea) {
          CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItem,
                                    CXFA_TraverseStrategy_ContentLayoutItem>
          iterator(static_cast<CXFA_ContentLayoutItem*>(pItem->m_pFirstChild));
          for (CXFA_ContentLayoutItem* pItemChild = iterator.GetCurrent();
               pItemChild; pItemChild = iterator.MoveToNext()) {
            if (!pItemChild->IsContentLayoutItem())
              continue;
            if (pItemChild->m_pFormNode->GetElementType() != eType)
              continue;
            if (pdfium::ContainsValue(formItems, pItemChild->m_pFormNode))
              continue;

            formItems.insert(pItemChild->m_pFormNode);
            retArray.push_back(pItemChild->m_pFormNode);
          }
        }
      } else {
        if (bOnPageArea) {
          CXFA_NodeIteratorTemplate<CXFA_ContentLayoutItem,
                                    CXFA_TraverseStrategy_ContentLayoutItem>
          iterator(static_cast<CXFA_ContentLayoutItem*>(pItem));
          for (CXFA_ContentLayoutItem* pItemChild = iterator.GetCurrent();
               pItemChild; pItemChild = iterator.MoveToNext()) {
            if (!pItemChild->IsContentLayoutItem())
              continue;
            if (pItemChild->m_pFormNode->GetElementType() != eType)
              continue;
            if (pdfium::ContainsValue(formItems, pItemChild->m_pFormNode))
              continue;

            formItems.insert(pItemChild->m_pFormNode);
            retArray.push_back(pItemChild->m_pFormNode);
          }
        }
      }
    }
  }
  return retArray;
}

void CJX_LayoutPseudoModel::PageContent(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    ThrowParamCountMismatchException(L"pageContent");
    return;
  }

  int32_t iIndex = 0;
  WideString wsType;
  bool bOnPageArea = false;
  if (iLength >= 1)
    iIndex = pArguments->GetInt32(0);

  if (iLength >= 2) {
    ByteString bsType = pArguments->GetUTF8String(1);
    wsType = WideString::FromUTF8(bsType.AsStringView());
  }
  if (iLength >= 3)
    bOnPageArea = pArguments->GetInt32(2) == 0 ? false : true;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_LayoutProcessor* pDocLayout = GetDocument()->GetDocLayout();
  if (!pDocLayout)
    return;

  auto pArrayNodeList = pdfium::MakeUnique<CXFA_ArrayNodeList>(GetDocument());
  pArrayNodeList->SetArrayNodeList(
      GetObjArray(pDocLayout, iIndex, wsType, bOnPageArea));
  pArguments->GetReturnValue()->SetObject(
      pArrayNodeList.release(),
      GetDocument()->GetScriptContext()->GetJseNormalClass());
}

void CJX_LayoutPseudoModel::AbsPageCount(CFXJSE_Arguments* pArguments) {
  NumberedPageCount(pArguments, false);
}

void CJX_LayoutPseudoModel::AbsPageCountInBatch(CFXJSE_Arguments* pArguments) {
  pArguments->GetReturnValue()->SetInteger(0);
}

void CJX_LayoutPseudoModel::SheetCountInBatch(CFXJSE_Arguments* pArguments) {
  pArguments->GetReturnValue()->SetInteger(0);
}

void CJX_LayoutPseudoModel::Relayout(CFXJSE_Arguments* pArguments) {
  CXFA_Node* pRootNode = GetDocument()->GetRoot();
  CXFA_Node* pFormRoot = pRootNode->GetFirstChildByClass(XFA_Element::Form);
  CXFA_Node* pContentRootNode = pFormRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
  CXFA_LayoutProcessor* pLayoutProcessor = GetDocument()->GetLayoutProcessor();
  if (pContentRootNode)
    pLayoutProcessor->AddChangedContainer(pContentRootNode);

  pLayoutProcessor->SetForceReLayout(true);
}

void CJX_LayoutPseudoModel::AbsPageSpan(CFXJSE_Arguments* pArguments) {
  PageSpan(pArguments);
}

void CJX_LayoutPseudoModel::AbsPageInBatch(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"absPageInBatch");
    return;
  }

  pArguments->GetReturnValue()->SetInteger(0);
}

void CJX_LayoutPseudoModel::SheetInBatch(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"sheetInBatch");
    return;
  }

  pArguments->GetReturnValue()->SetInteger(0);
}

void CJX_LayoutPseudoModel::Sheet(CFXJSE_Arguments* pArguments) {
  PageInternals(pArguments, true);
}

void CJX_LayoutPseudoModel::RelayoutPageArea(CFXJSE_Arguments* pArguments) {}

void CJX_LayoutPseudoModel::SheetCount(CFXJSE_Arguments* pArguments) {
  NumberedPageCount(pArguments, false);
}

void CJX_LayoutPseudoModel::AbsPage(CFXJSE_Arguments* pArguments) {
  PageInternals(pArguments, true);
}

void CJX_LayoutPseudoModel::PageInternals(CFXJSE_Arguments* pArguments,
                                          bool bAbsPage) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(bAbsPage ? L"absPage" : L"page");
    return;
  }

  CXFA_Node* pNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (!pNode)
    pValue->SetInteger(0);

  CXFA_LayoutProcessor* pDocLayout = GetDocument()->GetDocLayout();
  if (!pDocLayout)
    return;

  CXFA_LayoutItem* pLayoutItem = pDocLayout->GetLayoutItem(pNode);
  if (!pLayoutItem) {
    pValue->SetInteger(-1);
    return;
  }

  int32_t iPage = pLayoutItem->GetFirst()->GetPage()->GetPageIndex();
  pValue->SetInteger(bAbsPage ? iPage : iPage + 1);
}
