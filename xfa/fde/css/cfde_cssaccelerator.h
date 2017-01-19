// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSACCELERATOR_H_
#define XFA_FDE_CSS_CFDE_CSSACCELERATOR_H_

#include <memory>
#include <stack>

#include "xfa/fde/css/cfde_csstagcache.h"

class CXFA_CSSTagProvider;

class CFDE_CSSAccelerator {
 public:
  CFDE_CSSAccelerator();
  ~CFDE_CSSAccelerator();

  void OnEnterTag(CXFA_CSSTagProvider* pTag);
  void OnLeaveTag(CXFA_CSSTagProvider* pTag);

  void Clear() {
    std::stack<std::unique_ptr<CFDE_CSSTagCache>> tmp;
    stack_.swap(tmp);
  }

  CFDE_CSSTagCache* top() const {
    if (stack_.empty())
      return nullptr;
    return stack_.top().get();
  }

 private:
  std::stack<std::unique_ptr<CFDE_CSSTagCache>> stack_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSACCELERATOR_H_
