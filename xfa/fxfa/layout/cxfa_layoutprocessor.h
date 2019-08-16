// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_LAYOUTPROCESSOR_H_
#define XFA_FXFA_LAYOUT_CXFA_LAYOUTPROCESSOR_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_ContentLayoutProcessor;
class CXFA_LayoutItem;
class CXFA_Node;
class CXFA_ViewLayoutItem;
class CXFA_ViewLayoutProcessor;

class CXFA_LayoutProcessor : public CXFA_Document::LayoutProcessorIface {
 public:
  static CXFA_LayoutProcessor* FromDocument(const CXFA_Document* pXFADoc);

  CXFA_LayoutProcessor();
  ~CXFA_LayoutProcessor() override;

  // CXFA_Document::LayoutProcessorIface:
  void SetForceRelayout(bool bForceRestart) override;

  int32_t StartLayout(bool bForceRestart);
  int32_t DoLayout();
  bool IncrementLayout();
  int32_t CountPages() const;
  CXFA_ViewLayoutItem* GetPage(int32_t index) const;
  CXFA_LayoutItem* GetLayoutItem(CXFA_Node* pFormItem);
  void AddChangedContainer(CXFA_Node* pContainer);
  CXFA_ViewLayoutItem* GetRootLayoutItem() const;
  CXFA_ContentLayoutProcessor* GetRootContentLayoutProcessor() const {
    return m_pContentLayoutProcessor.get();
  }
  CXFA_ViewLayoutProcessor* GetLayoutPageMgr() const {
    return m_pViewLayoutProcessor.get();
  }

 private:
  bool NeedLayout() const;

  std::unique_ptr<CXFA_ViewLayoutProcessor> m_pViewLayoutProcessor;
  std::unique_ptr<CXFA_ContentLayoutProcessor> m_pContentLayoutProcessor;
  std::vector<CXFA_Node*> m_rgChangedContainers;
  uint32_t m_nProgressCounter = 0;
  bool m_bNeedLayout = true;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_LAYOUTPROCESSOR_H_
