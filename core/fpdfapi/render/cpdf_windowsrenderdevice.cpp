// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_windowsrenderdevice.h"

#include "core/fxcodec/basic/basicmodule.h"
#include "core/fxcodec/fax/faxmodule.h"
#include "core/fxcodec/flate/flatemodule.h"
#include "core/fxcodec/jpeg/jpegmodule.h"
#include "core/fxge/win32/cfx_psrenderer.h"

namespace {

constexpr EncoderIface kEncoderIface = {
    BasicModule::A85Encode, FaxModule::FaxEncode, FlateModule::Encode,
    JpegModule::JpegEncode, BasicModule::RunLengthEncode};

}  // namespace

CPDF_WindowsRenderDevice::CPDF_WindowsRenderDevice(HDC hDC)
    : CFX_WindowsRenderDevice(hDC, &kEncoderIface) {}

CPDF_WindowsRenderDevice::~CPDF_WindowsRenderDevice() = default;
