// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfx_textuserdata.h"

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_csscomputedstyle.h"
#include "core/fxcrt/css/cfx_cssstyleselector.h"
#include "xfa/fgas/layout/cfx_linkuserdata.h"

CFX_TextUserData::CFX_TextUserData(
    const RetainPtr<CFX_CSSComputedStyle>& pStyle)
    : m_pStyle(pStyle) {}

CFX_TextUserData::CFX_TextUserData(
    const RetainPtr<CFX_CSSComputedStyle>& pStyle,
    const RetainPtr<CFX_LinkUserData>& pLinkData)
    : m_pStyle(pStyle), m_pLinkData(pLinkData) {}

CFX_TextUserData::~CFX_TextUserData() {}
