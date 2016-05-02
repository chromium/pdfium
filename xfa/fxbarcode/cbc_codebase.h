// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_CBC_CODEBASE_H_
#define XFA_FXBARCODE_CBC_CODEBASE_H_

#include "core/fxcrt/include/fx_system.h"
#include "core/fxge/include/fx_dib.h"
#include "xfa/fxbarcode/include/BC_Library.h"

class CBC_Writer;
class CBC_Reader;
class CFX_DIBitmap;
class CFX_RenderDevice;

class CBC_CodeBase {
 public:
  CBC_CodeBase();
  virtual ~CBC_CodeBase();

  virtual BC_TYPE GetType() = 0;
  virtual FX_BOOL Encode(const CFX_WideStringC& contents,
                         FX_BOOL isDevice,
                         int32_t& e) = 0;
  virtual FX_BOOL RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matrix,
                               int32_t& e) = 0;
  virtual FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) = 0;
  virtual CFX_WideString Decode(uint8_t* buf,
                                int32_t width,
                                int32_t height,
                                int32_t& e) = 0;
  virtual CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e) = 0;

  virtual FX_BOOL SetCharEncoding(int32_t encoding);
  virtual FX_BOOL SetModuleHeight(int32_t moduleHeight);
  virtual FX_BOOL SetModuleWidth(int32_t moduleWidth);

  virtual FX_BOOL SetHeight(int32_t height);
  virtual FX_BOOL SetWidth(int32_t width);
  virtual void SetBackgroundColor(FX_ARGB backgroundColor);
  virtual void SetBarcodeColor(FX_ARGB foregroundColor);

 protected:
  CBC_Writer* m_pBCWriter;
  CBC_Reader* m_pBCReader;
};

#endif  // XFA_FXBARCODE_CBC_CODEBASE_H_
