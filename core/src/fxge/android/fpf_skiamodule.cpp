// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fx_fpf.h"
#if _FX_OS_ == _FX_ANDROID_
#include "fpf_skiamodule.h"
#include "fpf_skiafontmgr.h"
static IFPF_DeviceModule *gs_pPFModule = NULL;
IFPF_DeviceModule* FPF_GetDeviceModule()
{
    if (!gs_pPFModule) {
        gs_pPFModule = new CFPF_SkiaDeviceModule;
    }
    return gs_pPFModule;
}
CFPF_SkiaDeviceModule::~CFPF_SkiaDeviceModule()
{
    if (m_pFontMgr) {
        delete m_pFontMgr;
    }
}
void CFPF_SkiaDeviceModule::Destroy()
{
    if (gs_pPFModule) {
        delete (CFPF_SkiaDeviceModule*)gs_pPFModule;
        gs_pPFModule = NULL;
    }
}
IFPF_FontMgr* CFPF_SkiaDeviceModule::GetFontMgr()
{
    if (!m_pFontMgr) {
        m_pFontMgr = new CFPF_SkiaFontMgr;
        if (!m_pFontMgr->InitFTLibrary()) {
            delete m_pFontMgr;
            return NULL;
        }
    }
    return (IFPF_FontMgr*)m_pFontMgr;
}
#endif
