// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_App::CFWL_App()
    : m_pAppImp(NULL)
    , m_pThemeProvider(NULL)
{
    m_pAppImp = IFWL_App::Create(NULL);
}
CFWL_App::~CFWL_App()
{
    if (m_pThemeProvider) {
        m_pThemeProvider->Finalize();
        delete m_pThemeProvider;
        m_pThemeProvider = NULL;
    }
    m_pAppImp->Release();
}
FWL_ERR CFWL_App::Initialize()
{
    m_pThemeProvider = FX_NEW CFWL_Theme;
    m_pThemeProvider->Initialize();
    m_pAppImp->SetThemeProvider((IFWL_ThemeProvider*)m_pThemeProvider);
    return m_pAppImp->Initialize();
}
CFWL_Theme*	CFWL_App::GetTheme()
{
    return m_pThemeProvider;
}
FWL_ERR	CFWL_App::Exit(FX_INT32 iExitCode )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pAppImp, FWL_ERR_Indefinite);
    return m_pAppImp->Exit(iExitCode);
}
IFWL_App* CFWL_App::GetApp()
{
    return m_pAppImp;
}
