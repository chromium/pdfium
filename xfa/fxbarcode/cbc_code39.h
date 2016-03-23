// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_CBC_CODE39_H_
#define XFA_FXBARCODE_CBC_CODE39_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "core/include/fxge/fx_dib.h"
#include "xfa/fxbarcode/cbc_onecode.h"

class CBC_Code39 : public CBC_OneCode {
 public:
  CBC_Code39();
  explicit CBC_Code39(FX_BOOL usingCheckDigit);
  CBC_Code39(FX_BOOL usingCheckDigit, FX_BOOL extendedMode);
  virtual ~CBC_Code39();

  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);

  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);

  BC_TYPE GetType() { return BC_CODE39; }
  FX_BOOL SetTextLocation(BC_TEXT_LOC location);
  FX_BOOL SetWideNarrowRatio(int32_t ratio);

 private:
  CFX_WideString m_renderContents;
};

#endif  // XFA_FXBARCODE_CBC_CODE39_H_
