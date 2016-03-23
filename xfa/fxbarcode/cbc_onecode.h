// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_CBC_ONECODE_H_
#define XFA_FXBARCODE_CBC_ONECODE_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fxbarcode/cbc_codebase.h"

class CFX_DIBitmap;
class CFX_Font;
class CFX_RenderDevice;

class CBC_OneCode : public CBC_CodeBase {
 public:
  CBC_OneCode();
  virtual ~CBC_OneCode();

  virtual BC_TYPE GetType() = 0;

  virtual FX_BOOL Encode(const CFX_WideStringC& contents,
                         FX_BOOL isDevice,
                         int32_t& e) = 0;
  virtual CFX_WideString Decode(uint8_t* buf,
                                int32_t width,
                                int32_t hight,
                                int32_t& e) = 0;
  virtual CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e) = 0;

  virtual FX_BOOL RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matirx,
                               int32_t& e) = 0;
  virtual FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) = 0;


  virtual FX_BOOL CheckContentValidity(const CFX_WideStringC& contents);
  virtual CFX_WideString FilterContents(const CFX_WideStringC& contents);

  virtual void SetPrintChecksum(FX_BOOL checksum);
  virtual void SetDataLength(int32_t length);
  virtual void SetCalChecksum(FX_BOOL calc);
  virtual FX_BOOL SetFont(CFX_Font* cFont);
  virtual void SetFontSize(FX_FLOAT size);
  virtual void SetFontStyle(int32_t style);
  virtual void SetFontColor(FX_ARGB color);
};

#endif  // XFA_FXBARCODE_CBC_ONECODE_H_
