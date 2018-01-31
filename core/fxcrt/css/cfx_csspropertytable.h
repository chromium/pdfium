// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSPROPERTYTABLE_H_
#define CORE_FXCRT_CSS_CFX_CSSPROPERTYTABLE_H_

#include <memory>
#include <vector>

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_cssnumbervalue.h"
#include "core/fxcrt/css/cfx_cssvalue.h"
#include "core/fxcrt/string_view_template.h"

class CFX_CSSPropertyTable {
 public:
  struct Entry {
    CFX_CSSProperty eName;
    const wchar_t* pszName;
    uint32_t dwHash;
    uint32_t dwType;
  };

  static const Entry* GetByName(WideStringView nam);
  static const Entry* GetByEnum(CFX_CSSProperty property);
};

#endif  // CORE_FXCRT_CSS_CFX_CSSPROPERTYTABLE_H_
