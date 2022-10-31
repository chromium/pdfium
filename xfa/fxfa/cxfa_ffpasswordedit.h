// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFPASSWORDEDIT_H_
#define XFA_FXFA_CXFA_FFPASSWORDEDIT_H_

#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/cxfa_fftextedit.h"

class CXFA_PasswordEdit;

class CXFA_FFPasswordEdit final : public CXFA_FFTextEdit {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFPasswordEdit() override;

  void Trace(cppgc::Visitor* visitor) const override;

  // CXFA_FFTextEdit
  bool LoadWidget() override;
  void UpdateWidgetProperty() override;

 private:
  CXFA_FFPasswordEdit(CXFA_Node* pNode, CXFA_PasswordEdit* password_node);

  cppgc::Member<CXFA_PasswordEdit> const password_node_;
};

#endif  // XFA_FXFA_CXFA_FFPASSWORDEDIT_H_
