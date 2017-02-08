// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSSELECTOR_H_
#define XFA_FDE_CSS_CFDE_CSSSELECTOR_H_

#include <memory>
#include <utility>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/css/fde_css.h"

class CFDE_CSSSelector {
 public:
  static std::unique_ptr<CFDE_CSSSelector> FromString(
      const CFX_WideStringC& str);

  CFDE_CSSSelector(FDE_CSSSelectorType eType,
                   const FX_WCHAR* psz,
                   int32_t iLen,
                   bool bIgnoreCase);
  ~CFDE_CSSSelector();

  FDE_CSSSelectorType GetType() const;
  uint32_t GetNameHash() const;
  CFDE_CSSSelector* GetNextSelector() const;

  void SetNext(std::unique_ptr<CFDE_CSSSelector> pNext) {
    m_pNext = std::move(pNext);
  }

 private:
  void SetType(FDE_CSSSelectorType eType) { m_eType = eType; }

  FDE_CSSSelectorType m_eType;
  uint32_t m_dwHash;
  std::unique_ptr<CFDE_CSSSelector> m_pNext;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSELECTOR_H_
