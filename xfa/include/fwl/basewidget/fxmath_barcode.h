// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXMATH_BARCODE_H_
#define _FXMATH_BARCODE_H_
class IFX_Barcode {
 public:
  virtual ~IFX_Barcode() {}
  virtual void Release() = 0;
  virtual BC_TYPE GetType() = 0;
  virtual FX_BOOL Encode(const CFX_WideStringC& contents,
                         FX_BOOL isDevice,
                         int32_t& e) = 0;
  virtual FX_BOOL RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matirx,
                               int32_t& e) = 0;
  virtual FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) = 0;
  virtual CFX_WideString Decode(uint8_t* buf,
                                int32_t width,
                                int32_t height,
                                int32_t& errorCode) = 0;
  virtual CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& errorCode) = 0;
  virtual FX_BOOL SetCharEncoding(BC_CHAR_ENCODING encoding) = 0;
  virtual FX_BOOL SetModuleHeight(int32_t moduleHeight) = 0;
  virtual FX_BOOL SetModuleWidth(int32_t moduleWidth) = 0;
  virtual FX_BOOL SetHeight(int32_t height) = 0;
  virtual FX_BOOL SetWidth(int32_t width) = 0;
  virtual FX_BOOL CheckContentValidity(const CFX_WideStringC& contents) = 0;
  virtual FX_BOOL SetPrintChecksum(FX_BOOL checksum) = 0;
  virtual FX_BOOL SetDataLength(int32_t length) = 0;
  virtual FX_BOOL SetCalChecksum(int32_t state) = 0;
  virtual FX_BOOL SetFont(CFX_Font* pFont) = 0;
  virtual FX_BOOL SetFontSize(FX_FLOAT size) = 0;
  virtual FX_BOOL SetFontStyle(int32_t style) = 0;
  virtual FX_BOOL SetFontColor(FX_ARGB color) = 0;
  virtual FX_BOOL SetTextLocation(BC_TEXT_LOC location) = 0;
  virtual FX_BOOL SetWideNarrowRatio(int32_t ratio) = 0;
  virtual FX_BOOL SetStartChar(FX_CHAR start) = 0;
  virtual FX_BOOL SetEndChar(FX_CHAR end) = 0;
  virtual FX_BOOL SetVersion(int32_t version) = 0;
  virtual FX_BOOL SetErrorCorrectionLevel(int32_t level) = 0;
  virtual FX_BOOL SetTruncated(FX_BOOL truncated) = 0;
};
IFX_Barcode* FX_Barcode_Create(BC_TYPE type);
#endif
