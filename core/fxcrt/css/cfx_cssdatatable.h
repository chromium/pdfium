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

#define CFX_IsOnlyValue(type, enum) \
  (((type) & ~(enum)) == CFX_CSSVALUETYPE_Primitive)

struct CFX_CSSPropertyTable {
  CFX_CSSProperty eName;
  const wchar_t* pszName;
  uint32_t dwHash;
  uint32_t dwType;
};

struct CFX_CSSPropertyValueTable {
  CFX_CSSPropertyValue eName;
  const wchar_t* pszName;
  uint32_t dwHash;
};

struct CFX_CSSLengthUnitTable {
  uint16_t wHash;
  CFX_CSSNumberType wValue;
};

struct CFX_CSSCOLORTABLE {
  uint32_t dwHash;
  FX_ARGB dwValue;
};

const CFX_CSSPropertyTable* CFX_GetCSSPropertyByName(
    const CFX_WideStringC& wsName);
const CFX_CSSPropertyTable* CFX_GetCSSPropertyByEnum(CFX_CSSProperty eName);

const CFX_CSSPropertyValueTable* CFX_GetCSSPropertyValueByName(
    const CFX_WideStringC& wsName);

const CFX_CSSLengthUnitTable* CFX_GetCSSLengthUnitByName(
    const CFX_WideStringC& wsName);

const CFX_CSSCOLORTABLE* CFX_GetCSSColorByName(const CFX_WideStringC& wsName);

#endif  // CORE_FXCRT_CSS_CFX_CSSDATATABLE_H_
