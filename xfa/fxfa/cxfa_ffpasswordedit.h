// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFPASSWORDEDIT_H_
#define XFA_FXFA_CXFA_FFPASSWORDEDIT_H_

#include "xfa/fxfa/cxfa_fftextedit.h"

class CXFA_WidgetAcc;

class CXFA_FFPasswordEdit : public CXFA_FFTextEdit {
 public:
  explicit CXFA_FFPasswordEdit(CXFA_Node* pNode);
  ~CXFA_FFPasswordEdit() override;

  // CXFA_FFTextEdit
  bool LoadWidget() override;
  void UpdateWidgetProperty() override;
};

#endif  // XFA_FXFA_CXFA_FFPASSWORDEDIT_H_
