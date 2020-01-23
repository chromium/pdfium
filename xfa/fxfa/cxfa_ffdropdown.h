// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDROPDOWN_H_
#define XFA_FXFA_CXFA_FFDROPDOWN_H_

#include "core/fxcrt/widestring.h"
#include "xfa/fxfa/cxfa_fffield.h"

class CXFA_FFComboBox;

class CXFA_FFDropDown : public CXFA_FFField {
 public:
  ~CXFA_FFDropDown() override;

  // CXFA_FFField:
  CXFA_FFDropDown* AsDropDown() override;

  virtual void InsertItem(const WideString& wsLabel, int32_t nIndex) = 0;
  virtual void DeleteItem(int32_t nIndex) = 0;
  virtual CXFA_FFComboBox* AsComboBox();

 protected:
  explicit CXFA_FFDropDown(CXFA_Node* pNode);
};

inline CXFA_FFComboBox* ToComboBox(CXFA_FFDropDown* pDropDown) {
  return pDropDown ? pDropDown->AsComboBox() : nullptr;
}

#endif  // XFA_FXFA_CXFA_FFDROPDOWN_H_
