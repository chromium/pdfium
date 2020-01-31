// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_APPLE_FX_MAC_IMPL_H_
#define CORE_FXGE_APPLE_FX_MAC_IMPL_H_

#include "core/fxge/apple/fx_quartz_device.h"
#include "core/fxge/cfx_gemodule.h"

class CApplePlatform : public CFX_GEModule::PlatformIface {
 public:
  CApplePlatform();
  ~CApplePlatform() override;

  // CFX_GEModule::PlatformIface:
  void Init() override;

  CQuartz2D m_quartz2d;
};

#endif  // CORE_FXGE_APPLE_FX_MAC_IMPL_H_
