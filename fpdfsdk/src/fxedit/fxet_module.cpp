// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/fxedit/fxet_edit.h"
#include "fpdfsdk/include/fxedit/fxet_list.h"

IFX_Edit* IFX_Edit::NewEdit() {
  if (IPDF_VariableText* pVT = IPDF_VariableText::NewVariableText()) {
    return new CFX_Edit(pVT);
  }

  return NULL;
}

void IFX_Edit::DelEdit(IFX_Edit* pEdit) {
  IPDF_VariableText::DelVariableText(pEdit->GetVariableText());

  delete (CFX_Edit*)pEdit;
}

IFX_List* IFX_List::NewList() {
  return new CFX_ListCtrl();
}

void IFX_List::DelList(IFX_List* pList) {
  ASSERT(pList);
  delete (CFX_ListCtrl*)pList;
}
