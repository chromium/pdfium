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
    return content_layout_processor_;
  }
  CXFA_ViewLayoutProcessor* GetLayoutPageMgr() const {
    return view_layout_processor_;
  }

 private:
  explicit CXFA_LayoutProcessor(cppgc::Heap* pHeap);

  cppgc::Heap* GetHeap() { return heap_; }
  bool NeedLayout() const;
  int32_t RestartLayout();

  UnownedPtr<cppgc::Heap> const heap_;
  cppgc::Member<CXFA_ViewLayoutProcessor> view_layout_processor_;
  cppgc::Member<CXFA_ContentLayoutProcessor> content_layout_processor_;
  uint32_t progress_counter_ = 0;
  bool has_changed_containers_ = false;
  bool need_layout_ = true;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_LAYOUTPROCESSOR_H_
