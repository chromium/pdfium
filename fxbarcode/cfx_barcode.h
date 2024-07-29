// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_CFX_BARCODE_H_
#define FXBARCODE_CFX_BARCODE_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/widestring.h"
#include "core/fxge/dib/fx_dib.h"
#include "fxbarcode/BC_Library.h"

class CBC_CodeBase;
class CFX_Font;
class CFX_RenderDevice;
class CFX_Matrix;

class CFX_Barcode {
 public:
  ~CFX_Barcode();

  static std::unique_ptr<CFX_Barcode> Create(BC_TYPE type);
  BC_TYPE GetType();
  bool Encode(WideStringView contents);

  bool RenderDevice(CFX_RenderDevice* device, const CFX_Matrix& matrix);

  bool SetModuleHeight(int32_t moduleHeight);
  bool SetModuleWidth(int32_t moduleWidth);
  void SetHeight(int32_t height);
  void SetWidth(int32_t width);

  bool SetPrintChecksum(bool checksum);
  bool SetDataLength(int32_t length);
  bool SetCalChecksum(bool state);

  bool SetFont(CFX_Font* pFont);
  bool SetFontSize(float size);
  bool SetFontColor(FX_ARGB color);

  void SetTextLocation(BC_TEXT_LOC location);
  bool SetWideNarrowRatio(int8_t ratio);
  bool SetStartChar(char start);
  bool SetEndChar(char end);
  bool SetErrorCorrectionLevel(int32_t level);

 private:
  CFX_Barcode();

  std::unique_ptr<CBC_CodeBase> m_pBCEngine;
};

#endif  // FXBARCODE_CFX_BARCODE_H_
