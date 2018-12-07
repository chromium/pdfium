// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_BASIC_DATA_H_
#define XFA_FXFA_PARSER_XFA_BASIC_DATA_H_

#include <stddef.h>

#include "core/fxcrt/widestring.h"
#include "xfa/fxfa/fxfa_basic.h"

extern const XFA_AttributeValueInfo g_XFAEnumData[];
extern const size_t g_szXFAEnumCount;

extern const XFA_SCRIPTATTRIBUTEINFO g_SomAttributeData[];
extern const size_t g_szSomAttributeCount;

const XFA_SCRIPTATTRIBUTEINFO* XFA_GetScriptAttributeByName(
    XFA_Element eElement,
    WideStringView wsAttributeName);

#endif  // XFA_FXFA_PARSER_XFA_BASIC_DATA_H_
