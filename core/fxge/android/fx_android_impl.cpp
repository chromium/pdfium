// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>
#include <utility>

#include "core/fxge/android/cfpf_skiadevicemodule.h"
#include "core/fxge/android/cfx_androidfontinfo.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"

class CAndroidPlatform : public CFX_GEModule::PlatformIface {
 public:
  CAndroidPlatform() = default;
  ~CAndroidPlatform() override {
    if (m_pDeviceModule)
      m_pDeviceModule->Destroy();
  }

  void Init() override { m_pDeviceModule = CFPF_GetSkiaDeviceModule(); }

  std::unique_ptr<SystemFontInfoIface> CreateDefaultSystemFontInfo() override {
    CFPF_SkiaFontMgr* pFontMgr = m_pDeviceModule->GetFontMgr();
    if (!pFontMgr)
      return nullptr;

    auto pFontInfo = std::make_unique<CFX_AndroidFontInfo>();
    pFontInfo->Init(pFontMgr);
    return pFontInfo;
  }

 private:
  CFPF_SkiaDeviceModule* m_pDeviceModule = nullptr;
};

// static
std::unique_ptr<CFX_GEModule::PlatformIface>
CFX_GEModule::PlatformIface::Create() {
  return std::make_unique<CAndroidPlatform>();
}
