// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSDATATABLE_H_
#define CORE_FXCRT_CSS_CFX_CSSDATATABLE_H_

#include <memory>
#include <vector>

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_cssnumbervalue.h"
#include "core/fxcrt/css/cfx_cssvalue.h"
#include "core/fxcrt/fx_system.h"

struct CFX_CSSPropertyTable {
  CFX_CSSProperty eName;
  const wchar_t* pszName;
  uint32_t dwHash;
  uint32_t dwType;
};

const CFX_CSSPropertyTable* CFX_GetCSSPropertyByName(
    const WideStringView& wsName);
const CFX_CSSPropertyTable* CFX_GetCSSPropertyByEnum(CFX_CSSProperty eName);

#endif  // CORE_FXCRT_CSS_CFX_CSSDATATABLE_H_
