// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_CBC_CODE128_H_
#define XFA_FXBARCODE_CBC_CODE128_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "core/include/fxge/fx_dib.h"
#include "xfa/fxbarcode/cbc_onecode.h"

class CBC_Code128 : public CBC_OneCode {
 public:
  explicit CBC_Code128(BC_TYPE type);
  virtual ~CBC_Code128();

  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e) override;
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e) override;
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e) override;

  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e) override;
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) override;

  BC_TYPE GetType() override { return BC_CODE128; }

  FX_BOOL SetTextLocation(BC_TEXT_LOC loction);

 private:
  CFX_WideString m_renderContents;
};

#endif  // XFA_FXBARCODE_CBC_CODE128_H_
