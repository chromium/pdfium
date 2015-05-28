// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_ANDROID_FPF_SKIAMODULE_H_
#define CORE_SRC_FXGE_ANDROID_FPF_SKIAMODULE_H_

#if _FX_OS_ == _FX_ANDROID_
class CFPF_SkiaFontMgr;
class CFPF_SkiaDeviceModule : public IFPF_DeviceModule
{
public:
    CFPF_SkiaDeviceModule() : m_pFontMgr(NULL) {}
    virtual ~CFPF_SkiaDeviceModule();
    virtual void				Destroy();
    virtual IFPF_FontMgr*		GetFontMgr();
protected:
    CFPF_SkiaFontMgr	*m_pFontMgr;
};
#endif

#endif  // CORE_SRC_FXGE_ANDROID_FPF_SKIAMODULE_H_
