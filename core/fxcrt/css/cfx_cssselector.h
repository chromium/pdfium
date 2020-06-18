// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSSELECTOR_H_
#define CORE_FXCRT_CSS_CFX_CSSSELECTOR_H_

#include <memory>
#include <utility>

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/fx_string.h"

class CFX_CSSSelector {
 public:
  static std::unique_ptr<CFX_CSSSelector> FromString(WideStringView str);

  CFX_CSSSelector(const wchar_t* psz, int32_t iLen);
  ~CFX_CSSSelector();

  CFX_CSSSelectorType type() const { return type_; }
  uint32_t name_hash() const { return name_hash_; }
  const CFX_CSSSelector* next_selector() const { return next_.get(); }

  void set_next(std::unique_ptr<CFX_CSSSelector> pNext) {
    next_ = std::move(pNext);
  }

 private:
  void set_descendent_type() { type_ = CFX_CSSSelectorType::Descendant; }

  CFX_CSSSelectorType type_ = CFX_CSSSelectorType::Element;
  const uint32_t name_hash_;
  std::unique_ptr<CFX_CSSSelector> next_;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSSELECTOR_H_
