// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
CFWL_App::CFWL_App() : m_pIface(IFWL_App::Create(nullptr)), m_pTheme(nullptr) {}
CFWL_App::~CFWL_App() {
  if (m_pTheme) {
    m_pTheme->Finalize();
    delete m_pTheme;
    m_pTheme = NULL;
  }
  m_pIface->Release();
}
FWL_ERR CFWL_App::Initialize() {
  m_pTheme = new CFWL_Theme;
  m_pTheme->Initialize();
  m_pIface->SetThemeProvider(m_pTheme);
  return m_pIface->Initialize();
}
FWL_ERR CFWL_App::Exit(int32_t iExitCode) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->Exit(iExitCode);
}
