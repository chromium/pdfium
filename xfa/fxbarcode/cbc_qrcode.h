// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_CBC_QRCODE_H_
#define XFA_FXBARCODE_CBC_QRCODE_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "core/include/fxge/fx_dib.h"
#include "xfa/fxbarcode/cbc_codebase.h"

class CBC_QRCode : public CBC_CodeBase {
 public:
  CBC_QRCode();
  virtual ~CBC_QRCode();

  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);

  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);

  BC_TYPE GetType() { return BC_QR_CODE; }

  FX_BOOL SetVersion(int32_t version);
  FX_BOOL SetErrorCorrectionLevel(int32_t level);
};

#endif  // XFA_FXBARCODE_CBC_QRCODE_H_
