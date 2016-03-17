// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cagg_graphics.h"

CAGG_Graphics::CAGG_Graphics() : m_owner(nullptr) {}

FX_ERR CAGG_Graphics::Create(CFX_Graphics* owner,
                             int32_t width,
                             int32_t height,
                             FXDIB_Format format) {
  if (owner->m_renderDevice)
    return FX_ERR_Parameter_Invalid;
  if (m_owner)
    return FX_ERR_Property_Invalid;

  CFX_FxgeDevice* device = new CFX_FxgeDevice;
  device->Create(width, height, format);
  m_owner = owner;
  m_owner->m_renderDevice = device;
  m_owner->m_renderDevice->GetBitmap()->Clear(0xFFFFFFFF);
  return FX_ERR_Succeeded;
}

CAGG_Graphics::~CAGG_Graphics() {
  if (m_owner->m_renderDevice)
    delete (CFX_FxgeDevice*)m_owner->m_renderDevice;
  m_owner = nullptr;
}
