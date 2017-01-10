// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_textuserdata.h"

#include "xfa/fde/css/fde_css.h"
#include "xfa/fxfa/app/cxfa_linkuserdata.h"

CXFA_TextUserData::CXFA_TextUserData(IFDE_CSSComputedStyle* pStyle)
    : m_pStyle(pStyle), m_pLinkData(nullptr), m_dwRefCount(0) {
  if (m_pStyle)
    m_pStyle->Retain();
}

CXFA_TextUserData::CXFA_TextUserData(IFDE_CSSComputedStyle* pStyle,
                                     CXFA_LinkUserData* pLinkData)
    : m_pStyle(pStyle), m_pLinkData(pLinkData), m_dwRefCount(0) {
  if (m_pStyle)
    m_pStyle->Retain();
}

CXFA_TextUserData::~CXFA_TextUserData() {
  if (m_pStyle)
    m_pStyle->Release();
  if (m_pLinkData)
    m_pLinkData->Release();
}

uint32_t CXFA_TextUserData::Retain() {
  return ++m_dwRefCount;
}

uint32_t CXFA_TextUserData::Release() {
  uint32_t dwRefCount = --m_dwRefCount;
  if (dwRefCount == 0)
    delete this;
  return dwRefCount;
}
