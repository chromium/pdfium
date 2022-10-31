// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_layoutpseudomodel.h"

#include <set>
#include <utility>

#include "core/fxcrt/fx_coordinates.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "third_party/base/containers/contains.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_layoutitem.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_traversestrategy_layoutitem.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"
#include "xfa/fxfa/parser/cscript_layoutpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_form.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"

const CJX_MethodSpec CJX_LayoutPseudoModel::MethodSpecs[] = {
    {"absPage", absPage_static},
    {"absPageCount", absPageCount_static},
    {"absPageCountInBatch", absPageCountInBatch_static},
    {"absPageInBatch", absPageInBatch_static},
    {"absPageSpan", absPageSpan_static},
    {"h", h_static},
    {"page", page_static},
    {"pageContent", pageContent_static},
    {"pageCount", pageCount_static},
    {"pageSpan", pageSpan_static},
    {"relayout", relayout_static},
    {"relayoutPageArea", relayoutPageArea_static},
    {"sheet", sheet_static},
    {"sheetCount", sheetCount_static},
    {"sheetCountInBatch", sheetCountInBatch_static},
    {"sheetInBatch", sheetInBatch_static},
    {"w", w_static},
    {"x", x_static},
    {"y", y_static}};

CJX_LayoutPseudoModel::CJX_LayoutPseudoModel(CScript_LayoutPseudoModel* model)
    : CJX_Object(model) {
  DefineMethods(MethodSpecs);
}

CJX_LayoutPseudoModel::~CJX_LayoutPseudoModel() = default;

bool CJX_LayoutPseudoModel::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_LayoutPseudoModel::ready(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;
  if (bSetting) {
    ThrowException(pIsolate,
                   WideString::FromASCII("Unable to set ready value."));
    return;
  }

  CXFA_FFDocView::LayoutStatus iStatus = pNotify->GetLayoutStatus();
  const bool bReady = iStatus != CXFA_FFDocView::LayoutStatus::kNone &&
                      iStatus != CXFA_FFDocView::LayoutStatus::kStart;
  *pValue = fxv8::NewBooleanHelper(pIsolate, bReady);
}

CJS_Result CJX_LayoutPseudoModel::DoHWXYInternal(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params,
    HWXY layoutModel) {
  if (params.empty() || params.size() > 3)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* pNode = ToNode(runtime->ToXFAObject(params[0]));
  if (!pNode)
    return CJS_Result::Success();

  WideString unit = WideString::FromASCII("pt");
  if (params.size() >= 2) {
    WideString tmp_unit = runtime->ToWideString(params[1]);
    if (!tmp_unit.IsEmpty())
      unit = std::move(tmp_unit);
  }
  int32_t iIndex = params.size() >= 3 ? runtime->ToInt32(params[2]) : 0;
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(GetDocument());
  CXFA_ContentLayoutItem* pLayoutItem =
      ToContentLayoutItem(pDocLayout->GetLayoutItem(pNode));
  if (!pLayoutItem)
    return CJS_Result::Success();

  while (iIndex > 0 && pLayoutItem) {
    pLayoutItem = pLayoutItem->GetNext();
    --iIndex;
  }

  if (!pLayoutItem)
    return CJS_Result::Success(runtime->NewNumber(0.0));

  CXFA_Measurement measure;
  CFX_RectF rtRect = pLayoutItem->GetRelativeRect();
  switch (layoutModel) {
    case HWXY::kH:
      measure.Set(rtRect.height, XFA_Unit::Pt);
      break;
    case HWXY::kW:
      measure.Set(rtRect.width, XFA_Unit::Pt);
      break;
    case HWXY::kX:
      measure.Set(rtRect.left, XFA_Unit::Pt);
      break;
    case HWXY::kY:
      measure.Set(rtRect.top, XFA_Unit::Pt);
      break;
  }

  XFA_Unit eUnit = CXFA_Measurement::GetUnitFromString(unit.AsStringView());
  if (eUnit == XFA_Unit::Unknown)
    return CJS_Result::Failure(JSMessage::kValueError);

  float fValue = measure.ToUnit(eUnit);
  return CJS_Result::Success(
      runtime->NewNumber(FXSYS_roundf(fValue * 1000) / 1000.0f));
}

CJS_Result CJX_LayoutPseudoModel::h(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return DoHWXYInternal(runtime, params, HWXY::kH);
}

CJS_Result CJX_LayoutPseudoModel::w(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return DoHWXYInternal(runtime, params, HWXY::kW);
}

CJS_Result CJX_LayoutPseudoModel::x(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return DoHWXYInternal(runtime, params, HWXY::kX);
}

CJS_Result CJX_LayoutPseudoModel::y(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return DoHWXYInternal(runtime, params, HWXY::kY);
}

CJS_Result CJX_LayoutPseudoModel::AllPageCount(CFXJSE_Engine* runtime) {
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(GetDocument());
  return CJS_Result::Success(runtime->NewNumber(pDocLayout->CountPages()));
}

CJS_Result CJX_LayoutPseudoModel::NumberedPageCount(CFXJSE_Engine* runtime) {
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(GetDocument());
  int32_t iPageCount = 0;
  int32_t iPageNum = pDocLayout->CountPages();
  for (int32_t i = 0; i < iPageNum; i++) {
    CXFA_ViewLayoutItem* pLayoutPage = pDocLayout->GetPage(i);
    if (!pLayoutPage)
      continue;

    CXFA_Node* pMasterPage = pLayoutPage->GetMasterPage();
    if (pMasterPage->JSObject()->GetInteger(XFA_Attribute::Numbered))
      iPageCount++;
  }
  return CJS_Result::Success(runtime->NewNumber(iPageCount));
}

CJS_Result CJX_LayoutPseudoModel::pageCount(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return NumberedPageCount(runtime);
}

CJS_Result CJX_LayoutPseudoModel::pageSpan(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* pNode = ToNode(runtime->ToXFAObject(params[0]));
  if (!pNode)
    return CJS_Result::Success();

  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(GetDocument());
  CXFA_ContentLayoutItem* pLayoutItem =
      ToContentLayoutItem(pDocLayout->GetLayoutItem(pNode));
  if (!pLayoutItem)
    return CJS_Result::Success(runtime->NewNumber(-1));

  int32_t iLast = pLayoutItem->GetLast()->GetPage()->GetPageIndex();
  int32_t iFirst = pLayoutItem->GetFirst()->GetPage()->GetPageIndex();
  int32_t iPageSpan = iLast - iFirst + 1;
  return CJS_Result::Success(runtime->NewNumber(iPageSpan));
}

CJS_Result CJX_LayoutPseudoModel::page(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return PageInternals(runtime, params, false);
}

std::vector<CXFA_Node*> CJX_LayoutPseudoModel::GetObjArray(
    CXFA_LayoutProcessor* pDocLayout,
    int32_t iPageNo,
    const WideString& wsType,
    bool bOnPageArea) {
  CXFA_ViewLayoutItem* pLayoutPage = pDocLayout->GetPage(iPageNo);
  if (!pLayoutPage)
    return std::vector<CXFA_Node*>();

  std::vector<CXFA_Node*> retArray;
  if (wsType.EqualsASCII("pageArea")) {
    if (pLayoutPage->GetFormNode())
      retArray.push_back(pLayoutPage->GetFormNode());
    return retArray;
  }
  if (wsType.EqualsASCII("contentArea")) {
    for (CXFA_LayoutItem* pItem = pLayoutPage->GetFirstChild(); pItem;
         pItem = pItem->GetNextSibling()) {
      if (pItem->GetFormNode()->GetElementType() == XFA_Element::ContentArea)
        retArray.push_back(pItem->GetFormNode());
    }
    return retArray;
  }
  std::set<CXFA_Node*> formItems;
  if (wsType.IsEmpty()) {
    if (pLayoutPage->GetFormNode())
      retArray.push_back(pLayoutPage->GetFormNode());

    for (CXFA_LayoutItem* pItem = pLayoutPage->GetFirstChild(); pItem;
         pItem = pItem->GetNextSibling()) {
      if (pItem->GetFormNode()->GetElementType() == XFA_Element::ContentArea) {
        retArray.push_back(pItem->GetFormNode());
        if (!bOnPageArea) {
          CXFA_LayoutItemIterator iterator(pItem->GetFirstChild());
          for (CXFA_LayoutItem* pChild = iterator.GetCurrent(); pChild;
               pChild = iterator.MoveToNext()) {
            CXFA_ContentLayoutItem* pItemChild = pChild->AsContentLayoutItem();
            if (!pItemChild)
              continue;

            XFA_Element eType = pItemChild->GetFormNode()->GetElementType();
            if (eType != XFA_Element::Field && eType != XFA_Element::Draw &&
                eType != XFA_Element::Subform && eType != XFA_Element::Area) {
              continue;
            }
            if (pdfium::Contains(formItems, pItemChild->GetFormNode()))
              continue;

            formItems.insert(pItemChild->GetFormNode());
            retArray.push_back(pItemChild->GetFormNode());
          }
        }
      } else {
        if (bOnPageArea) {
          CXFA_LayoutItemIterator iterator(pItem);
          for (CXFA_LayoutItem* pChild = iterator.GetCurrent(); pChild;
               pChild = iterator.MoveToNext()) {
            CXFA_ContentLayoutItem* pItemChild = pChild->AsContentLayoutItem();
            if (!pItemChild)
              continue;

            XFA_Element eType = pItemChild->GetFormNode()->GetElementType();
            if (eType != XFA_Element::Field && eType != XFA_Element::Draw &&
                eType != XFA_Element::Subform && eType != XFA_Element::Area) {
              continue;
            }
            if (pdfium::Contains(formItems, pItemChild->GetFormNode()))
              continue;

            formItems.insert(pItemChild->GetFormNode());
            retArray.push_back(pItemChild->GetFormNode());
          }
        }
      }
    }
    return retArray;
  }

  XFA_Element eType = XFA_Element::Unknown;
  if (wsType.EqualsASCII("field"))
    eType = XFA_Element::Field;
  else if (wsType.EqualsASCII("draw"))
    eType = XFA_Element::Draw;
  else if (wsType.EqualsASCII("subform"))
    eType = XFA_Element::Subform;
  else if (wsType.EqualsASCII("area"))
    eType = XFA_Element::Area;

  if (eType != XFA_Element::Unknown) {
    for (CXFA_LayoutItem* pItem = pLayoutPage->GetFirstChild(); pItem;
         pItem = pItem->GetNextSibling()) {
      if (pItem->GetFormNode()->GetElementType() == XFA_Element::ContentArea) {
        if (!bOnPageArea) {
          CXFA_LayoutItemIterator iterator(pItem->GetFirstChild());
          for (CXFA_LayoutItem* pChild = iterator.GetCurrent(); pChild;
               pChild = iterator.MoveToNext()) {
            CXFA_ContentLayoutItem* pItemChild = pChild->AsContentLayoutItem();
            if (!pItemChild)
              continue;
            if (pItemChild->GetFormNode()->GetElementType() != eType)
              continue;
            if (pdfium::Contains(formItems, pItemChild->GetFormNode()))
              continue;

            formItems.insert(pItemChild->GetFormNode());
            retArray.push_back(pItemChild->GetFormNode());
          }
        }
      } else {
        if (bOnPageArea) {
          CXFA_LayoutItemIterator iterator(pItem);
          for (CXFA_LayoutItem* pChild = iterator.GetCurrent(); pChild;
               pChild = iterator.MoveToNext()) {
            CXFA_ContentLayoutItem* pItemChild = pChild->AsContentLayoutItem();
            if (!pItemChild)
              continue;
            if (pItemChild->GetFormNode()->GetElementType() != eType)
              continue;
            if (pdfium::Contains(formItems, pItemChild->GetFormNode()))
              continue;

            formItems.insert(pItemChild->GetFormNode());
            retArray.push_back(pItemChild->GetFormNode());
          }
        }
      }
    }
  }
  return retArray;
}

CJS_Result CJX_LayoutPseudoModel::pageContent(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 3)
    return CJS_Result::Failure(JSMessage::kParamError);

  int32_t iIndex = 0;
  if (params.size() >= 1)
    iIndex = runtime->ToInt32(params[0]);

  WideString wsType;
  if (params.size() >= 2)
    wsType = runtime->ToWideString(params[1]);

  bool bOnPageArea = false;
  if (params.size() >= 3)
    bOnPageArea = runtime->ToBoolean(params[2]);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  CXFA_Document* pDoc = GetDocument();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(pDoc);
  auto* pArrayNodeList = cppgc::MakeGarbageCollected<CXFA_ArrayNodeList>(
      pDoc->GetHeap()->GetAllocationHandle(), pDoc);
  pDoc->GetNodeOwner()->PersistList(pArrayNodeList);
  pArrayNodeList->SetArrayNodeList(
      GetObjArray(pDocLayout, iIndex, wsType, bOnPageArea));
  return CJS_Result::Success(runtime->NewNormalXFAObject(pArrayNodeList));
}

CJS_Result CJX_LayoutPseudoModel::absPageCount(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return AllPageCount(runtime);
}

CJS_Result CJX_LayoutPseudoModel::absPageCountInBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success(runtime->NewNumber(0));
}

CJS_Result CJX_LayoutPseudoModel::sheetCountInBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success(runtime->NewNumber(0));
}

CJS_Result CJX_LayoutPseudoModel::relayout(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_Node* pRootNode = GetDocument()->GetRoot();
  auto* pLayoutProcessor = GetDocument()->GetLayoutProcessor();
  CXFA_Form* pFormRoot =
      pRootNode->GetFirstChildByClass<CXFA_Form>(XFA_Element::Form);
  if (pFormRoot) {
    if (pFormRoot->GetFirstChild())
      pLayoutProcessor->SetHasChangedContainer();
  }
  pLayoutProcessor->SetForceRelayout();
  return CJS_Result::Success();
}

CJS_Result CJX_LayoutPseudoModel::absPageSpan(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return pageSpan(runtime, params);
}

CJS_Result CJX_LayoutPseudoModel::absPageInBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewNumber(0));
}

CJS_Result CJX_LayoutPseudoModel::sheetInBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewNumber(0));
}

CJS_Result CJX_LayoutPseudoModel::sheet(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return PageInternals(runtime, params, true);
}

CJS_Result CJX_LayoutPseudoModel::relayoutPageArea(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success();
}

CJS_Result CJX_LayoutPseudoModel::sheetCount(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return AllPageCount(runtime);
}

CJS_Result CJX_LayoutPseudoModel::absPage(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return PageInternals(runtime, params, true);
}

CJS_Result CJX_LayoutPseudoModel::PageInternals(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params,
    bool bAbsPage) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* pNode = ToNode(runtime->ToXFAObject(params[0]));
  if (!pNode)
    return CJS_Result::Success(runtime->NewNumber(0));

  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(GetDocument());
  CXFA_ContentLayoutItem* pLayoutItem =
      ToContentLayoutItem(pDocLayout->GetLayoutItem(pNode));
  if (!pLayoutItem)
    return CJS_Result::Success(runtime->NewNumber(-1));

  int32_t iPage = pLayoutItem->GetFirst()->GetPage()->GetPageIndex();
  return CJS_Result::Success(runtime->NewNumber(bAbsPage ? iPage : iPage + 1));
}
