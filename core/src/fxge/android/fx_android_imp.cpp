// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fx_fpf.h"
#if _FX_OS_ == _FX_ANDROID_
void CFX_GEModule::InitPlatform()
{
    IFPF_DeviceModule *pDeviceModule = FPF_GetDeviceModule();
    if (!pDeviceModule) {
        return;
    }
    IFPF_FontMgr *pFontMgr = pDeviceModule->GetFontMgr();
    if (pFontMgr) {
        CFX_AndroidFontInfo *pFontInfo = new CFX_AndroidFontInfo;
        pFontInfo->Init(pFontMgr);
        m_pFontMgr->SetSystemFontInfo(pFontInfo);
    }
    m_pPlatformData = pDeviceModule;
}
void CFX_GEModule::DestroyPlatform()
{
    if (m_pPlatformData) {
        ((IFPF_DeviceModule*)m_pPlatformData)->Destroy();
    }
}
#endif
