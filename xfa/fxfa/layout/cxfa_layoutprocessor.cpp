// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"

#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
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

CXFA_LayoutProcessor::CXFA_LayoutProcessor() = default;

CXFA_LayoutProcessor::~CXFA_LayoutProcessor() = default;

void CXFA_LayoutProcessor::SetForceRelayout(bool bForceRestart) {
  m_bNeedLayout = bForceRestart;
}

int32_t CXFA_LayoutProcessor::StartLayout(bool bForceRestart) {
  if (!bForceRestart && !IsNeedLayout())
    return 100;

  m_pContentLayoutProcessor.reset();
  m_nProgressCounter = 0;
  CXFA_Node* pFormPacketNode =
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pFormPacketNode)
    return -1;

  CXFA_Subform* pFormRoot =
      pFormPacketNode->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
  if (!pFormRoot)
    return -1;

  if (!m_pViewLayoutProcessor)
    m_pViewLayoutProcessor = pdfium::MakeUnique<CXFA_ViewLayoutProcessor>(this);
  if (!m_pViewLayoutProcessor->InitLayoutPage(pFormRoot))
    return -1;

  if (!m_pViewLayoutProcessor->PrepareFirstPage(pFormRoot))
    return -1;

  m_pContentLayoutProcessor = pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
      pFormRoot, m_pViewLayoutProcessor.get());
  m_nProgressCounter = 1;
  return 0;
}

int32_t CXFA_LayoutProcessor::DoLayout() {
  if (m_nProgressCounter < 1)
    return -1;

  CXFA_ContentLayoutProcessor::Result eStatus;
  CXFA_Node* pFormNode = m_pContentLayoutProcessor->GetFormNode();
  float fPosX =
      pFormNode->JSObject()->GetMeasureInUnit(XFA_Attribute::X, XFA_Unit::Pt);
  float fPosY =
      pFormNode->JSObject()->GetMeasureInUnit(XFA_Attribute::Y, XFA_Unit::Pt);
  do {
    float fAvailHeight = m_pViewLayoutProcessor->GetAvailHeight();
    eStatus =
        m_pContentLayoutProcessor->DoLayout(true, fAvailHeight, fAvailHeight);
    if (eStatus != CXFA_ContentLayoutProcessor::Result::kDone)
      m_nProgressCounter++;

    RetainPtr<CXFA_ContentLayoutItem> pLayoutItem =
        m_pContentLayoutProcessor->ExtractLayoutItem();
    if (pLayoutItem)
      pLayoutItem->m_sPos = CFX_PointF(fPosX, fPosY);

    m_pViewLayoutProcessor->SubmitContentItem(pLayoutItem, eStatus);
  } while (eStatus != CXFA_ContentLayoutProcessor::Result::kDone);

  if (eStatus == CXFA_ContentLayoutProcessor::Result::kDone) {
    m_pViewLayoutProcessor->FinishPaginatedPageSets();
    m_pViewLayoutProcessor->SyncLayoutData();
    m_bNeedLayout = false;
    m_rgChangedContainers.clear();
  }
  return 100 *
         (eStatus == CXFA_ContentLayoutProcessor::Result::kDone
              ? m_nProgressCounter
              : m_nProgressCounter - 1) /
         m_nProgressCounter;
}

bool CXFA_LayoutProcessor::IncrementLayout() {
  if (m_bNeedLayout) {
    StartLayout(true);
    return DoLayout() == 100;
  }
  return m_rgChangedContainers.empty();
}

int32_t CXFA_LayoutProcessor::CountPages() const {
  return m_pViewLayoutProcessor ? m_pViewLayoutProcessor->GetPageCount() : 0;
}

CXFA_ViewLayoutItem* CXFA_LayoutProcessor::GetPage(int32_t index) const {
  return m_pViewLayoutProcessor ? m_pViewLayoutProcessor->GetPage(index)
                                : nullptr;
}

CXFA_LayoutItem* CXFA_LayoutProcessor::GetLayoutItem(CXFA_Node* pFormItem) {
  return pFormItem->JSObject()->GetLayoutItem();
}

void CXFA_LayoutProcessor::AddChangedContainer(CXFA_Node* pContainer) {
  if (!pdfium::ContainsValue(m_rgChangedContainers, pContainer))
    m_rgChangedContainers.push_back(pContainer);
}

CXFA_ViewLayoutItem* CXFA_LayoutProcessor::GetRootLayoutItem() const {
  return m_pViewLayoutProcessor ? m_pViewLayoutProcessor->GetRootLayoutItem()
                                : nullptr;
}

bool CXFA_LayoutProcessor::IsNeedLayout() {
  return m_bNeedLayout || !m_rgChangedContainers.empty();
}
