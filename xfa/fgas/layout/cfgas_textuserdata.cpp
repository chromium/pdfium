// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_textuserdata.h"

#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_csscomputedstyle.h"
#include "core/fxcrt/css/cfx_cssstyleselector.h"
#include "xfa/fgas/layout/cfgas_linkuserdata.h"

CFGAS_TextUserData::CFGAS_TextUserData(
    const RetainPtr<CFX_CSSComputedStyle>& pStyle)
    : m_pStyle(pStyle) {}

CFGAS_TextUserData::CFGAS_TextUserData(
    const RetainPtr<CFX_CSSComputedStyle>& pStyle,
    const RetainPtr<CFGAS_LinkUserData>& pLinkData)
    : m_pStyle(pStyle), m_pLinkData(pLinkData) {}

CFGAS_TextUserData::~CFGAS_TextUserData() = default;
