// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffdocview.h"

#include <set>
#include <utility>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffbarcode.h"
#include "xfa/fxfa/cxfa_ffcheckbutton.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffexclgroup.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffimage.h"
#include "xfa/fxfa/cxfa_ffimageedit.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffpushbutton.h"
#include "xfa/fxfa/cxfa_ffsignature.h"
#include "xfa/fxfa/cxfa_fftext.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/cxfa_readynodeiterator.h"
#include "xfa/fxfa/cxfa_textprovider.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_acrobat.h"
#include "xfa/fxfa/parser/cxfa_binditems.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_pageset.h"
#include "xfa/fxfa/parser/cxfa_present.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/cxfa_validate.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

bool IsValidXMLNameString(const WideString& str) {
  bool first = true;
  for (const auto ch : str) {
    if (!CFX_XMLParser::IsXMLNameChar(ch, first)) {
      return false;
    }
    first = false;
  }
  return true;
}

const XFA_AttributeValue kXFAEventActivityData[] = {
    XFA_AttributeValue::Click,      XFA_AttributeValue::Change,
    XFA_AttributeValue::DocClose,   XFA_AttributeValue::DocReady,
    XFA_AttributeValue::Enter,      XFA_AttributeValue::Exit,
    XFA_AttributeValue::Full,       XFA_AttributeValue::IndexChange,
    XFA_AttributeValue::Initialize, XFA_AttributeValue::MouseDown,
    XFA_AttributeValue::MouseEnter, XFA_AttributeValue::MouseExit,
    XFA_AttributeValue::MouseUp,    XFA_AttributeValue::PostExecute,
    XFA_AttributeValue::PostOpen,   XFA_AttributeValue::PostPrint,
    XFA_AttributeValue::PostSave,   XFA_AttributeValue::PostSign,
    XFA_AttributeValue::PostSubmit, XFA_AttributeValue::PreExecute,
    XFA_AttributeValue::PreOpen,    XFA_AttributeValue::PrePrint,
    XFA_AttributeValue::PreSave,    XFA_AttributeValue::PreSign,
    XFA_AttributeValue::PreSubmit,  XFA_AttributeValue::Ready,
    XFA_AttributeValue::Unknown,
};

}  // namespace

const pdfium::span<const XFA_AttributeValue> kXFAEventActivity{
    kXFAEventActivityData};

CXFA_FFDocView::UpdateScope::UpdateScope(CXFA_FFDocView* pDocView)
    : doc_view_(pDocView) {
  doc_view_->LockUpdate();
}

CXFA_FFDocView::UpdateScope::~UpdateScope() {
  doc_view_->UnlockUpdate();
  doc_view_->UpdateDocView();
}

CXFA_FFDocView::CXFA_FFDocView(CXFA_FFDoc* pDoc) : doc_(pDoc) {}

CXFA_FFDocView::~CXFA_FFDocView() = default;

void CXFA_FFDocView::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(doc_);
  visitor->Trace(widget_handler_);
  visitor->Trace(focus_node_);
  visitor->Trace(focus_widget_);
  ContainerTrace(visitor, validate_nodes_);
  ContainerTrace(visitor, calculate_nodes_);
  ContainerTrace(visitor, new_added_nodes_);
  ContainerTrace(visitor, bind_items_);
  ContainerTrace(visitor, index_changed_subforms_);
}

void CXFA_FFDocView::InitLayout(CXFA_Node* pNode) {
  RunBindItems();
  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_Initialize, false, true);
  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_IndexChange, false, true);
}

int32_t CXFA_FFDocView::StartLayout() {
  status_ = LayoutStatus::kStart;
  doc_->GetXFADoc()->DoProtoMerge();
  doc_->GetXFADoc()->DoDataMerge();

  int32_t iStatus = GetLayoutProcessor()->StartLayout();
  if (iStatus < 0) {
    return iStatus;
  }

  CXFA_Node* pRootItem =
      ToNode(doc_->GetXFADoc()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pRootItem) {
    return iStatus;
  }

  InitLayout(pRootItem);
  InitCalculate(pRootItem);
  InitValidate(pRootItem);

  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready, true, true);
  status_ = LayoutStatus::kStart;
  return iStatus;
}

int32_t CXFA_FFDocView::DoLayout() {
  int32_t iStatus = GetLayoutProcessor()->DoLayout();
  if (iStatus != 100) {
    return iStatus;
  }

  status_ = LayoutStatus::kDoing;
  return iStatus;
}

void CXFA_FFDocView::StopLayout() {
  CXFA_Node* pRootItem =
      ToNode(doc_->GetXFADoc()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pRootItem) {
    return;
  }

  CXFA_Subform* pSubformNode =
      pRootItem->GetChild<CXFA_Subform>(0, XFA_Element::Subform, false);
  if (!pSubformNode) {
    return;
  }

  CXFA_PageSet* pPageSetNode =
      pSubformNode->GetFirstChildByClass<CXFA_PageSet>(XFA_Element::PageSet);
  if (!pPageSetNode) {
    return;
  }

  RunCalculateWidgets();
  RunValidate();

  InitLayout(pPageSetNode);
  InitCalculate(pPageSetNode);
  InitValidate(pPageSetNode);

  ExecEventActivityByDeepFirst(pPageSetNode, XFA_EVENT_Ready, true, true);
  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready, false, true);
  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_DocReady, false, true);

  RunCalculateWidgets();
  RunValidate();

  if (RunLayout()) {
    ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready, false, true);
  }

  calculate_nodes_.clear();
  if (focus_node_ && !focus_widget_) {
    SetFocusNode(focus_node_);
  }

  status_ = LayoutStatus::kEnd;
}

void CXFA_FFDocView::AddNullTestMsg(const WideString& msg) {
  null_test_msg_array_.push_back(msg);
}

void CXFA_FFDocView::ShowNullTestMsg() {
  int32_t iCount = fxcrt::CollectionSize<int32_t>(null_test_msg_array_);
  CXFA_FFApp* pApp = doc_->GetApp();
  CXFA_FFApp::CallbackIface* pAppProvider = pApp->GetAppProvider();
  if (pAppProvider && iCount) {
    int32_t remaining = iCount > 7 ? iCount - 7 : 0;
    iCount -= remaining;
    WideString wsMsg;
    for (int32_t i = 0; i < iCount; i++) {
      wsMsg += null_test_msg_array_[i] + L"\n";
    }

    if (remaining > 0) {
      wsMsg += L"\n" + WideString::Format(
                           L"Message limit exceeded. Remaining %d "
                           L"validation errors not reported.",
                           remaining);
    }
    pAppProvider->MsgBox(wsMsg, pAppProvider->GetAppTitle(),
                         static_cast<uint32_t>(AlertIcon::kStatus),
                         static_cast<uint32_t>(AlertButton::kOK));
  }
  null_test_msg_array_.clear();
}

void CXFA_FFDocView::UpdateDocView() {
  if (IsUpdateLocked()) {
    return;
  }

  LockUpdate();
  while (!new_added_nodes_.empty()) {
    CXFA_Node* pNode = new_added_nodes_.front();
    new_added_nodes_.pop_front();
    InitCalculate(pNode);
    InitValidate(pNode);
    ExecEventActivityByDeepFirst(pNode, XFA_EVENT_Ready, true, true);
  }

  RunSubformIndexChange();
  RunCalculateWidgets();
  RunValidate();

  ShowNullTestMsg();

  if (RunLayout() && layout_event_) {
    RunEventLayoutReady();
  }

  layout_event_ = false;
  calculate_nodes_.clear();
  UnlockUpdate();
}

void CXFA_FFDocView::UpdateUIDisplay(CXFA_Node* pNode, CXFA_FFWidget* pExcept) {
  CXFA_FFWidget* pWidget = GetWidgetForNode(pNode);
  CXFA_FFWidget* pNext = nullptr;
  for (; pWidget; pWidget = pNext) {
    pNext = pWidget->GetNextFFWidget();
    if (pWidget == pExcept || !pWidget->IsLoaded() ||
        (pNode->GetFFWidgetType() != XFA_FFWidgetType::kCheckButton &&
         pWidget->IsFocused())) {
      continue;
    }
    pWidget->UpdateFWLData();
    pWidget->InvalidateRect();
  }
}

int32_t CXFA_FFDocView::CountPageViews() const {
  CXFA_LayoutProcessor* pProcessor = GetLayoutProcessor();
  return pProcessor ? pProcessor->CountPages() : 0;
}

CXFA_FFPageView* CXFA_FFDocView::GetPageView(int32_t nIndex) const {
  CXFA_LayoutProcessor* pProcessor = GetLayoutProcessor();
  if (!pProcessor) {
    return nullptr;
  }

  auto* pPage = pProcessor->GetPage(nIndex);
  return pPage ? pPage->GetPageView() : nullptr;
}

CXFA_LayoutProcessor* CXFA_FFDocView::GetLayoutProcessor() const {
  return CXFA_LayoutProcessor::FromDocument(doc_->GetXFADoc());
}

bool CXFA_FFDocView::ResetSingleNodeData(CXFA_Node* pNode) {
  XFA_Element eType = pNode->GetElementType();
  if (eType != XFA_Element::Field && eType != XFA_Element::ExclGroup) {
    return false;
  }

  pNode->ResetData();
  UpdateUIDisplay(pNode, nullptr);
  CXFA_Validate* validate = pNode->GetValidateIfExists();
  if (!validate) {
    return true;
  }

  AddValidateNode(pNode);
  validate->SetFlag(XFA_NodeFlag::kNeedsInitApp);
  return true;
}

void CXFA_FFDocView::ResetNode(CXFA_Node* pNode) {
  layout_event_ = true;
  bool bChanged = false;
  CXFA_Node* pFormNode = nullptr;
  if (pNode) {
    bChanged = ResetSingleNodeData(pNode);
    pFormNode = pNode;
  } else {
    pFormNode = GetRootSubform();
  }
  if (!pFormNode) {
    return;
  }

  if (pFormNode->GetElementType() != XFA_Element::Field &&
      pFormNode->GetElementType() != XFA_Element::ExclGroup) {
    CXFA_ReadyNodeIterator it(pFormNode);
    while (CXFA_Node* next_node = it.MoveToNext()) {
      bChanged |= ResetSingleNodeData(next_node);
      if (next_node->GetElementType() == XFA_Element::ExclGroup) {
        it.SkipTree();
      }
    }
  }
  if (bChanged) {
    doc_->SetChangeMark();
  }
}

CXFA_FFWidget* CXFA_FFDocView::GetWidgetForNode(CXFA_Node* node) {
  return GetFFWidget(
      ToContentLayoutItem(GetLayoutProcessor()->GetLayoutItem(node)));
}

CXFA_FFWidgetHandler* CXFA_FFDocView::GetWidgetHandler() {
  if (!widget_handler_) {
    widget_handler_ = cppgc::MakeGarbageCollected<CXFA_FFWidgetHandler>(
        doc_->GetHeap()->GetAllocationHandle(), this);
  }
  return widget_handler_;
}

bool CXFA_FFDocView::SetFocus(CXFA_FFWidget* pNewFocus) {
  if (pNewFocus == focus_widget_) {
    return false;
  }

  if (focus_widget_) {
    CXFA_ContentLayoutItem* pItem = focus_widget_->GetLayoutItem();
    if (pItem->TestStatusBits(XFA_WidgetStatus::kVisible) &&
        !pItem->TestStatusBits(XFA_WidgetStatus::kFocused)) {
      if (!focus_widget_->IsLoaded()) {
        focus_widget_->LoadWidget();
      }
      if (!focus_widget_->OnSetFocus(focus_widget_)) {
        focus_widget_.Clear();
      }
    }
  }
  if (focus_widget_) {
    if (!focus_widget_->OnKillFocus(pNewFocus)) {
      return false;
    }
  }

  if (pNewFocus) {
    if (pNewFocus->GetLayoutItem()->TestStatusBits(
            XFA_WidgetStatus::kVisible)) {
      if (!pNewFocus->IsLoaded()) {
        pNewFocus->LoadWidget();
      }
      if (!pNewFocus->OnSetFocus(focus_widget_)) {
        pNewFocus = nullptr;
      }
    }
  }
  if (pNewFocus) {
    CXFA_Node* node = pNewFocus->GetNode();
    focus_node_ = node->IsWidgetReady() ? node : nullptr;
    focus_widget_ = pNewFocus;
  } else {
    focus_node_.Clear();
    focus_widget_.Clear();
  }
  return true;
}

void CXFA_FFDocView::SetFocusNode(CXFA_Node* node) {
  CXFA_FFWidget* pNewFocus = node ? GetWidgetForNode(node) : nullptr;
  if (!SetFocus(pNewFocus)) {
    return;
  }

  focus_node_ = node;
  if (status_ != LayoutStatus::kEnd) {
    return;
  }

  doc_->SetFocusWidget(focus_widget_);
}

void CXFA_FFDocView::DeleteLayoutItem(CXFA_FFWidget* pWidget) {
  if (focus_node_ != pWidget->GetNode()) {
    return;
  }

  focus_node_.Clear();
  focus_widget_.Clear();
}

static XFA_EventError XFA_ProcessEvent(CXFA_FFDocView* pDocView,
                                       CXFA_Node* pNode,
                                       CXFA_EventParam* pParam) {
  if (!pParam || pParam->type_ == XFA_EVENT_Unknown) {
    return XFA_EventError::kNotExist;
  }
  if (pNode && pNode->GetElementType() == XFA_Element::Draw) {
    return XFA_EventError::kNotExist;
  }

  switch (pParam->type_) {
    case XFA_EVENT_Calculate:
      return pNode->ProcessCalculate(pDocView);
    case XFA_EVENT_Validate:
      if (pDocView->GetDoc()->IsValidationsEnabled()) {
        return pNode->ProcessValidate(pDocView, 0x01);
      }
      return XFA_EventError::kDisabled;
    case XFA_EVENT_InitCalculate: {
      CXFA_Calculate* calc = pNode->GetCalculateIfExists();
      if (!calc) {
        return XFA_EventError::kNotExist;
      }
      if (pNode->IsUserInteractive()) {
        return XFA_EventError::kDisabled;
      }
      return pNode->ExecuteScript(pDocView, calc->GetScriptIfExists(), pParam);
    }
    default:
      return pNode->ProcessEvent(pDocView, kXFAEventActivity[pParam->type_],
                                 pParam);
  }
}

XFA_EventError CXFA_FFDocView::ExecEventActivityByDeepFirst(
    CXFA_Node* pFormNode,
    XFA_EVENTTYPE eEventType,
    bool bIsFormReady,
    bool bRecursive) {
  if (!pFormNode) {
    return XFA_EventError::kNotExist;
  }

  XFA_Element elementType = pFormNode->GetElementType();
  if (elementType == XFA_Element::Field) {
    if (eEventType == XFA_EVENT_IndexChange) {
      return XFA_EventError::kNotExist;
    }

    if (!pFormNode->IsWidgetReady()) {
      return XFA_EventError::kNotExist;
    }

    CXFA_EventParam eParam(eEventType);
    eParam.is_form_ready_ = bIsFormReady;
    return XFA_ProcessEvent(this, pFormNode, &eParam);
  }

  XFA_EventError iRet = XFA_EventError::kNotExist;
  if (bRecursive) {
    for (CXFA_Node* pNode = pFormNode->GetFirstContainerChild(); pNode;
         pNode = pNode->GetNextContainerSibling()) {
      elementType = pNode->GetElementType();
      if (elementType != XFA_Element::Variables &&
          elementType != XFA_Element::Draw) {
        XFA_EventErrorAccumulate(
            &iRet, ExecEventActivityByDeepFirst(pNode, eEventType, bIsFormReady,
                                                bRecursive));
      }
    }
  }
  if (!pFormNode->IsWidgetReady()) {
    return iRet;
  }

  CXFA_EventParam eParam(eEventType);
  eParam.is_form_ready_ = bIsFormReady;

  XFA_EventErrorAccumulate(&iRet, XFA_ProcessEvent(this, pFormNode, &eParam));
  return iRet;
}

CXFA_FFWidget* CXFA_FFDocView::GetWidgetByName(const WideString& wsName,
                                               CXFA_FFWidget* pRefWidget) {
  if (!IsValidXMLNameString(wsName)) {
    return nullptr;
  }
  CFXJSE_Engine* pScriptContext = doc_->GetXFADoc()->GetScriptContext();
  CXFA_Node* pRefNode = nullptr;
  if (pRefWidget) {
    CXFA_Node* node = pRefWidget->GetNode();
    pRefNode = node->IsWidgetReady() ? node : nullptr;
  }
  WideString wsExpression = (!pRefNode ? L"$form." : L"") + wsName;
  std::optional<CFXJSE_Engine::ResolveResult> maybeResult =
      pScriptContext->ResolveObjects(
          pRefNode, wsExpression.AsStringView(),
          Mask<XFA_ResolveFlag>{
              XFA_ResolveFlag::kChildren, XFA_ResolveFlag::kProperties,
              XFA_ResolveFlag::kSiblings, XFA_ResolveFlag::kParent});
  if (!maybeResult.has_value()) {
    return nullptr;
  }

  if (maybeResult.value().type == CFXJSE_Engine::ResolveResult::Type::kNodes) {
    CXFA_Node* pNode = maybeResult.value().objects.front()->AsNode();
    if (pNode && pNode->IsWidgetReady()) {
      return GetWidgetForNode(pNode);
    }
  }
  return nullptr;
}

void CXFA_FFDocView::OnPageViewEvent(CXFA_ViewLayoutItem* pSender,
                                     CXFA_FFDoc::PageViewEvent eEvent) {
  CXFA_FFPageView* pFFPageView = pSender ? pSender->GetPageView() : nullptr;
  doc_->OnPageViewEvent(pFFPageView, eEvent);
}

void CXFA_FFDocView::InvalidateRect(CXFA_FFPageView* pPageView,
                                    const CFX_RectF& rtInvalidate) {
  doc_->InvalidateRect(pPageView, rtInvalidate);
}

bool CXFA_FFDocView::RunLayout() {
  LockUpdate();
  in_layout_status_ = true;

  CXFA_LayoutProcessor* pProcessor = GetLayoutProcessor();
  if (!pProcessor->IncrementLayout() && pProcessor->StartLayout() < 100) {
    pProcessor->DoLayout();
    UnlockUpdate();
    in_layout_status_ = false;
    doc_->OnPageViewEvent(nullptr, CXFA_FFDoc::PageViewEvent::kStopLayout);
    return true;
  }

  in_layout_status_ = false;
  doc_->OnPageViewEvent(nullptr, CXFA_FFDoc::PageViewEvent::kStopLayout);
  UnlockUpdate();
  return false;
}

void CXFA_FFDocView::RunSubformIndexChange() {
  std::set<CXFA_Node*> seen;
  while (!index_changed_subforms_.empty()) {
    CXFA_Node* pSubformNode = index_changed_subforms_.front();
    index_changed_subforms_.pop_front();
    bool bInserted = seen.insert(pSubformNode).second;
    if (!bInserted || !pSubformNode->IsWidgetReady()) {
      continue;
    }

    CXFA_EventParam eParam(XFA_EVENT_IndexChange);
    pSubformNode->ProcessEvent(this, XFA_AttributeValue::IndexChange, &eParam);
  }
}

void CXFA_FFDocView::AddNewFormNode(CXFA_Node* pNode) {
  new_added_nodes_.push_back(pNode);
  InitLayout(pNode);
}

void CXFA_FFDocView::AddIndexChangedSubform(CXFA_Subform* pNode) {
  if (!pdfium::Contains(index_changed_subforms_, pNode)) {
    index_changed_subforms_.push_back(pNode);
  }
}

void CXFA_FFDocView::RunDocClose() {
  CXFA_Node* pRootItem =
      ToNode(doc_->GetXFADoc()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pRootItem) {
    return;
  }

  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_DocClose, false, true);
}

void CXFA_FFDocView::AddCalculateNode(CXFA_Node* node) {
  CXFA_Node* pCurrentNode =
      !calculate_nodes_.empty() ? calculate_nodes_.back() : nullptr;
  if (pCurrentNode != node) {
    calculate_nodes_.push_back(node);
  }
}

void CXFA_FFDocView::AddCalculateNodeNotify(CXFA_Node* pNodeChange) {
  CJX_Object::CalcData* pGlobalData = pNodeChange->JSObject()->GetCalcData();
  if (!pGlobalData) {
    return;
  }

  for (auto& pResult : pGlobalData->globals_) {
    if (!pResult->HasRemovedChildren() && pResult->IsWidgetReady()) {
      AddCalculateNode(pResult);
    }
  }
}

size_t CXFA_FFDocView::RunCalculateRecursive(size_t index) {
  while (index < calculate_nodes_.size()) {
    CXFA_Node* node = calculate_nodes_[index];

    AddCalculateNodeNotify(node);
    size_t recurse = node->JSObject()->GetCalcRecursionCount() + 1;
    node->JSObject()->SetCalcRecursionCount(recurse);
    if (recurse > 11) {
      break;
    }
    if (node->ProcessCalculate(this) == XFA_EventError::kSuccess &&
        node->IsWidgetReady()) {
      AddValidateNode(node);
    }

    index = RunCalculateRecursive(++index);
  }
  return index;
}

XFA_EventError CXFA_FFDocView::RunCalculateWidgets() {
  if (!doc_->IsCalculationsEnabled()) {
    return XFA_EventError::kDisabled;
  }

  if (!calculate_nodes_.empty()) {
    RunCalculateRecursive(0);
  }

  for (CXFA_Node* node : calculate_nodes_) {
    node->JSObject()->SetCalcRecursionCount(0);
  }

  calculate_nodes_.clear();
  return XFA_EventError::kSuccess;
}

void CXFA_FFDocView::AddValidateNode(CXFA_Node* node) {
  if (!pdfium::Contains(validate_nodes_, node)) {
    validate_nodes_.push_back(node);
  }
}

void CXFA_FFDocView::InitCalculate(CXFA_Node* pNode) {
  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_InitCalculate, false, true);
}

void CXFA_FFDocView::ProcessValueChanged(CXFA_Node* node) {
  AddValidateNode(node);
  AddCalculateNode(node);
  RunCalculateWidgets();
  RunValidate();
}

void CXFA_FFDocView::InitValidate(CXFA_Node* pNode) {
  if (!doc_->IsValidationsEnabled()) {
    return;
  }

  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_Validate, false, true);
  validate_nodes_.clear();
}

void CXFA_FFDocView::RunValidate() {
  if (!doc_->IsValidationsEnabled()) {
    return;
  }

  while (!validate_nodes_.empty()) {
    CXFA_Node* node = validate_nodes_.front();
    validate_nodes_.pop_front();
    if (!node->HasRemovedChildren()) {
      node->ProcessValidate(this, 0);
    }
  }
}

bool CXFA_FFDocView::RunEventLayoutReady() {
  CXFA_Node* pRootItem =
      ToNode(doc_->GetXFADoc()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pRootItem) {
    return false;
  }

  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready, false, true);
  RunLayout();
  return true;
}

void CXFA_FFDocView::RunBindItems() {
  while (!bind_items_.empty()) {
    CXFA_BindItems* item = bind_items_.front();
    bind_items_.pop_front();
    if (item->HasRemovedChildren()) {
      continue;
    }

    CXFA_Node* pWidgetNode = item->GetParent();
    if (!pWidgetNode || !pWidgetNode->IsWidgetReady()) {
      continue;
    }

    CFXJSE_Engine* pScriptContext =
        pWidgetNode->GetDocument()->GetScriptContext();
    WideString wsRef = item->GetRef();
    std::optional<CFXJSE_Engine::ResolveResult> maybeRS =
        pScriptContext->ResolveObjects(
            pWidgetNode, wsRef.AsStringView(),
            Mask<XFA_ResolveFlag>{
                XFA_ResolveFlag::kChildren, XFA_ResolveFlag::kProperties,
                XFA_ResolveFlag::kSiblings, XFA_ResolveFlag::kParent,
                XFA_ResolveFlag::kALL});
    pWidgetNode->DeleteItem(-1, false, false);
    if (!maybeRS.has_value() ||
        maybeRS.value().type != CFXJSE_Engine::ResolveResult::Type::kNodes ||
        maybeRS.value().objects.empty()) {
      continue;
    }
    WideString wsValueRef = item->GetValueRef();
    WideString wsLabelRef = item->GetLabelRef();
    const bool bUseValue = wsLabelRef.IsEmpty() || wsLabelRef == wsValueRef;
    const bool bLabelUseContent =
        wsLabelRef.IsEmpty() || wsLabelRef.EqualsASCII("$");
    const bool bValueUseContent =
        wsValueRef.IsEmpty() || wsValueRef.EqualsASCII("$");
    WideString wsValue;
    WideString wsLabel;
    uint32_t uValueHash = FX_HashCode_GetW(wsValueRef.AsStringView());
    for (auto& refObject : maybeRS.value().objects) {
      CXFA_Node* refNode = refObject->AsNode();
      if (!refNode) {
        continue;
      }

      if (bValueUseContent) {
        wsValue = refNode->JSObject()->GetContent(false);
      } else {
        CXFA_Node* nodeValue = refNode->GetFirstChildByName(uValueHash);
        wsValue = nodeValue ? nodeValue->JSObject()->GetContent(false)
                            : refNode->JSObject()->GetContent(false);
      }

      if (!bUseValue) {
        if (bLabelUseContent) {
          wsLabel = refNode->JSObject()->GetContent(false);
        } else {
          CXFA_Node* nodeLabel =
              refNode->GetFirstChildByName(wsLabelRef.AsStringView());
          if (nodeLabel) {
            wsLabel = nodeLabel->JSObject()->GetContent(false);
          }
        }
      } else {
        wsLabel = wsValue;
      }
      pWidgetNode->InsertItem(wsLabel, wsValue, false);
    }
  }
}

void CXFA_FFDocView::SetChangeMark() {
  if (status_ != LayoutStatus::kEnd) {
    return;
  }

  doc_->SetChangeMark();
}

CXFA_Node* CXFA_FFDocView::GetRootSubform() {
  CXFA_Node* pFormPacketNode =
      ToNode(doc_->GetXFADoc()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pFormPacketNode) {
    return nullptr;
  }

  return pFormPacketNode->GetFirstChildByClass<CXFA_Subform>(
      XFA_Element::Subform);
}
