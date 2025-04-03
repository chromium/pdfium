// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"

#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cjx_object.h"
#include "v8/include/cppgc/heap.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/xfa_document_datamerger_imp.h"
#include "xfa/fxfa/parser/xfa_utils.h"

// static
CXFA_LayoutProcessor* CXFA_LayoutProcessor::FromDocument(
    const CXFA_Document* pXFADoc) {
  return static_cast<CXFA_LayoutProcessor*>(pXFADoc->GetLayoutProcessor());
}

CXFA_LayoutProcessor::CXFA_LayoutProcessor(cppgc::Heap* pHeap) : heap_(pHeap) {}

CXFA_LayoutProcessor::~CXFA_LayoutProcessor() = default;

void CXFA_LayoutProcessor::Trace(cppgc::Visitor* visitor) const {
  CXFA_Document::LayoutProcessorIface::Trace(visitor);
  visitor->Trace(view_layout_processor_);
  visitor->Trace(content_layout_processor_);
}

void CXFA_LayoutProcessor::SetForceRelayout() {
  need_layout_ = true;
}

int32_t CXFA_LayoutProcessor::StartLayout() {
  return NeedLayout() ? RestartLayout() : 100;
}

int32_t CXFA_LayoutProcessor::RestartLayout() {
  content_layout_processor_ = nullptr;
  progress_counter_ = 0;
  CXFA_Node* pFormPacketNode =
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pFormPacketNode)
    return -1;

  CXFA_Subform* pFormRoot =
      pFormPacketNode->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
  if (!pFormRoot)
    return -1;

  if (!view_layout_processor_) {
    view_layout_processor_ =
        cppgc::MakeGarbageCollected<CXFA_ViewLayoutProcessor>(
            GetHeap()->GetAllocationHandle(), GetHeap(), this);
  }
  if (!view_layout_processor_->InitLayoutPage(pFormRoot)) {
    return -1;
  }

  if (!view_layout_processor_->PrepareFirstPage(pFormRoot)) {
    return -1;
  }

  content_layout_processor_ =
      cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
          GetHeap()->GetAllocationHandle(), GetHeap(), pFormRoot,
          view_layout_processor_);
  progress_counter_ = 1;
  return 0;
}

int32_t CXFA_LayoutProcessor::DoLayout() {
  if (progress_counter_ < 1) {
    return -1;
  }

  CXFA_ContentLayoutProcessor::Result eStatus;
  CXFA_Node* pFormNode = content_layout_processor_->GetFormNode();
  float fPosX =
      pFormNode->JSObject()->GetMeasureInUnit(XFA_Attribute::X, XFA_Unit::Pt);
  float fPosY =
      pFormNode->JSObject()->GetMeasureInUnit(XFA_Attribute::Y, XFA_Unit::Pt);
  do {
    float fAvailHeight = view_layout_processor_->GetAvailHeight();
    eStatus =
        content_layout_processor_->DoLayout(true, fAvailHeight, fAvailHeight);
    if (eStatus != CXFA_ContentLayoutProcessor::Result::kDone)
      progress_counter_++;

    CXFA_ContentLayoutItem* pLayoutItem =
        content_layout_processor_->ExtractLayoutItem();
    if (pLayoutItem)
      pLayoutItem->s_pos_ = CFX_PointF(fPosX, fPosY);

    view_layout_processor_->SubmitContentItem(pLayoutItem, eStatus);
  } while (eStatus != CXFA_ContentLayoutProcessor::Result::kDone);

  if (eStatus == CXFA_ContentLayoutProcessor::Result::kDone) {
    view_layout_processor_->FinishPaginatedPageSets();
    view_layout_processor_->SyncLayoutData();
    has_changed_containers_ = false;
    need_layout_ = false;
  }
  return 100 *
         (eStatus == CXFA_ContentLayoutProcessor::Result::kDone
              ? progress_counter_
              : progress_counter_ - 1) /
         progress_counter_;
}

bool CXFA_LayoutProcessor::IncrementLayout() {
  if (need_layout_) {
    RestartLayout();
    return DoLayout() == 100;
  }
  return !has_changed_containers_;
}

int32_t CXFA_LayoutProcessor::CountPages() const {
  return view_layout_processor_ ? view_layout_processor_->GetPageCount() : 0;
}

CXFA_ViewLayoutItem* CXFA_LayoutProcessor::GetPage(int32_t index) const {
  return view_layout_processor_ ? view_layout_processor_->GetPage(index)
                                : nullptr;
}

CXFA_LayoutItem* CXFA_LayoutProcessor::GetLayoutItem(CXFA_Node* pFormItem) {
  return pFormItem->JSObject()->GetLayoutItem();
}

void CXFA_LayoutProcessor::SetHasChangedContainer() {
  has_changed_containers_ = true;
}

bool CXFA_LayoutProcessor::NeedLayout() const {
  return need_layout_ || has_changed_containers_;
}
