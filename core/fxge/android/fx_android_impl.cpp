// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>
#include <utility>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/android/cfpf_skiadevicemodule.h"
#include "core/fxge/android/cfx_androidfontinfo.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"

class CAndroidPlatform : public CFX_GEModule::PlatformIface {
 public:
  CAndroidPlatform() = default;
  ~CAndroidPlatform() override {
    if (device_module_) {
      device_module_->Destroy();
    }
  }

  void Init() override { device_module_ = CFPF_GetSkiaDeviceModule(); }

  std::unique_ptr<SystemFontInfoIface> CreateDefaultSystemFontInfo() override {
    CFPF_SkiaFontMgr* font_mgr = device_module_->GetFontMgr();
    if (!font_mgr) {
      return nullptr;
    }

    auto font_info = std::make_unique<CFX_AndroidFontInfo>();
    font_info->Init(font_mgr, CFX_GEModule::Get()->GetUserFontPaths());
    return font_info;
  }

 private:
  UnownedPtr<CFPF_SkiaDeviceModule> device_module_;
};

// static
std::unique_ptr<CFX_GEModule::PlatformIface>
CFX_GEModule::PlatformIface::Create() {
  return std::make_unique<CAndroidPlatform>();
}
