// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_LAYOUTPROCESSOR_H_
#define XFA_FXFA_LAYOUT_CXFA_LAYOUTPROCESSOR_H_

#include <stdint.h>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_ContentLayoutProcessor;
class CXFA_LayoutItem;
class CXFA_Node;
class CXFA_ViewLayoutItem;
class CXFA_ViewLayoutProcessor;

namespace cppgc {
class Heap;
}  // namespace cppgc

class CXFA_LayoutProcessor final : public CXFA_Document::LayoutProcessorIface {
 public:
  static CXFA_LayoutProcessor* FromDocument(const CXFA_Document* pXFADoc);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_LayoutProcessor() override;

  void Trace(cppgc::Visitor* visitor) const override;

  // CXFA_Document::LayoutProcessorIface:
  void SetForceRelayout() override;
  void SetHasChangedContainer() override;

  int32_t StartLayout();
  int32_t DoLayout();
  bool IncrementLayout();
  int32_t CountPages() const;
  CXFA_ViewLayoutItem* GetPage(int32_t index) const;
  CXFA_LayoutItem* GetLayoutItem(CXFA_Node* pFormItem);
  CXFA_ContentLayoutProcessor* GetRootContentLayoutProcessor() const {
    return m_pContentLayoutProcessor;
  }
  CXFA_ViewLayoutProcessor* GetLayoutPageMgr() const {
    return m_pViewLayoutProcessor;
  }

 private:
  explicit CXFA_LayoutProcessor(cppgc::Heap* pHeap);

  cppgc::Heap* GetHeap() { return m_pHeap; }
  bool NeedLayout() const;
  int32_t RestartLayout();

  UnownedPtr<cppgc::Heap> const m_pHeap;
  cppgc::Member<CXFA_ViewLayoutProcessor> m_pViewLayoutProcessor;
  cppgc::Member<CXFA_ContentLayoutProcessor> m_pContentLayoutProcessor;
  uint32_t m_nProgressCounter = 0;
  bool m_bHasChangedContainers = false;
  bool m_bNeedLayout = true;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_LAYOUTPROCESSOR_H_
