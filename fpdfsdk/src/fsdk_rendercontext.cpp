// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/fsdk_rendercontext.h"

void CRenderContext::Clear() {
  m_pDevice = NULL;
  m_pContext = NULL;
  m_pRenderer = NULL;
  m_pAnnots = NULL;
  m_pOptions = NULL;
#ifdef _WIN32_WCE
  m_pBitmap = NULL;
  m_hBitmap = NULL;
#endif
}

CRenderContext::~CRenderContext() {
  delete m_pRenderer;
  delete m_pContext;
  delete m_pDevice;
  delete m_pAnnots;
  delete m_pOptions->m_pOCContext;
  delete m_pOptions;
#ifdef _WIN32_WCE
  delete m_pBitmap;
  if (m_hBitmap)
    DeleteObject(m_hBitmap);
#endif
}

IFSDK_PAUSE_Adapter::IFSDK_PAUSE_Adapter(IFSDK_PAUSE* IPause) {
  m_IPause = IPause;
}

IFSDK_PAUSE_Adapter::~IFSDK_PAUSE_Adapter() {
}

FX_BOOL IFSDK_PAUSE_Adapter::NeedToPauseNow() {
  if (m_IPause->NeedToPauseNow) {
    return m_IPause->NeedToPauseNow(m_IPause);
  }
  return FALSE;
}
