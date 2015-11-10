// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_ANDROID_FPF_SKIAMODULE_H_
#define CORE_SRC_FXGE_ANDROID_FPF_SKIAMODULE_H_

#if _FX_OS_ == _FX_ANDROID_

#include "core/include/fxge/fpf.h"

class CFPF_SkiaFontMgr;

class CFPF_SkiaDeviceModule : public IFPF_DeviceModule {
 public:
  CFPF_SkiaDeviceModule() : m_pFontMgr(nullptr) {}
  ~CFPF_SkiaDeviceModule() override;

  // IFPF_DeviceModule
  void Destroy() override;
  IFPF_FontMgr* GetFontMgr() override;

 protected:
  CFPF_SkiaFontMgr* m_pFontMgr;
};
#endif

#endif  // CORE_SRC_FXGE_ANDROID_FPF_SKIAMODULE_H_
