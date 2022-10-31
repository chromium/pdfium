// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFLISTBOX_H_
#define XFA_FXFA_CXFA_FFLISTBOX_H_

#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/prefinalizer.h"
#include "xfa/fxfa/cxfa_ffdropdown.h"

class CXFA_FFListBox final : public CXFA_FFDropDown {
  CPPGC_USING_PRE_FINALIZER(CXFA_FFListBox, PreFinalize);

 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFListBox() override;

  void PreFinalize();

  // CXFA_FFField:
  void Trace(cppgc::Visitor* visitor) const override;
  bool LoadWidget() override;
  [[nodiscard]] bool OnKillFocus(CXFA_FFWidget* pNewWidget) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;
  FormFieldType GetFormFieldType() override;

  // CXFA_FFDropDown:
  void InsertItem(const WideString& wsLabel, int32_t nIndex) override;
  void DeleteItem(int32_t nIndex) override;

  void OnSelectChanged(CFWL_Widget* pWidget);
  void SetItemState(int32_t nIndex, bool bSelected);

 private:
  explicit CXFA_FFListBox(CXFA_Node* pNode);

  bool CommitData() override;
  bool UpdateFWLData() override;
  bool IsDataChanged() override;

  uint32_t GetAlignment();

  cppgc::Member<IFWL_WidgetDelegate> m_pOldDelegate;
};

#endif  // XFA_FXFA_CXFA_FFLISTBOX_H_
