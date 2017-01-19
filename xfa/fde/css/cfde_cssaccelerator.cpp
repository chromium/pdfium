// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssaccelerator.h"

#include "third_party/base/ptr_util.h"

CFDE_CSSAccelerator::CFDE_CSSAccelerator() {}

CFDE_CSSAccelerator::~CFDE_CSSAccelerator() {}

void CFDE_CSSAccelerator::OnEnterTag(CXFA_CSSTagProvider* pTag) {
  stack_.push(pdfium::MakeUnique<CFDE_CSSTagCache>(top(), pTag));
}

void CFDE_CSSAccelerator::OnLeaveTag(CXFA_CSSTagProvider* pTag) {
  ASSERT(!stack_.empty());
  ASSERT(stack_.top()->GetTag() == pTag);
  stack_.pop();
}
