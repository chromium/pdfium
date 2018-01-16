// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFLISTBOX_H_
#define XFA_FXFA_CXFA_FFLISTBOX_H_

#include "xfa/fxfa/cxfa_fffield.h"

class CXFA_FFListBox : public CXFA_FFField {
 public:
  explicit CXFA_FFListBox(CXFA_Node* pNode);
  ~CXFA_FFListBox() override;

  // CXFA_FFField
  bool LoadWidget() override;
  bool OnKillFocus(CXFA_FFWidget* pNewWidget) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CXFA_Graphics* pGraphics,
                    const CFX_Matrix& matrix) override;
  FormFieldType GetFormFieldType() override;

  void OnSelectChanged(CFWL_Widget* pWidget);
  void SetItemState(int32_t nIndex, bool bSelected);
  void InsertItem(const WideStringView& wsLabel, int32_t nIndex);
  void DeleteItem(int32_t nIndex);

 private:
  bool CommitData() override;
  bool UpdateFWLData() override;
  bool IsDataChanged() override;

  uint32_t GetAlignment();

  IFWL_WidgetDelegate* m_pOldDelegate;
};

#endif  // XFA_FXFA_CXFA_FFLISTBOX_H_
