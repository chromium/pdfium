// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_textuserdata.h"

#include "xfa/fde/css/cfde_csscomputedstyle.h"
#include "xfa/fde/css/cfde_cssstyleselector.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fxfa/app/cxfa_linkuserdata.h"

CXFA_TextUserData::CXFA_TextUserData(
    const CFX_RetainPtr<CFDE_CSSComputedStyle>& pStyle)
    : m_pStyle(pStyle) {}

CXFA_TextUserData::CXFA_TextUserData(
    const CFX_RetainPtr<CFDE_CSSComputedStyle>& pStyle,
    const CFX_RetainPtr<CXFA_LinkUserData>& pLinkData)
    : m_pStyle(pStyle), m_pLinkData(pLinkData) {}

CXFA_TextUserData::~CXFA_TextUserData() {}
