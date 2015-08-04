// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXMATH_BARCODEIMP_H_
#define _FXMATH_BARCODEIMP_H_
class CFX_Barcode : public IFX_Barcode {
 public:
  CFX_Barcode();
  ~CFX_Barcode();
  FX_BOOL Crreate(BC_TYPE type);
  virtual void Release();
  virtual BC_TYPE GetType();
  virtual FX_BOOL Encode(const CFX_WideStringC& contents,
                         FX_BOOL isDevice,
                         int32_t& e);
  virtual FX_BOOL RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matirx,
                               int32_t& e);
  virtual FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  virtual CFX_WideString Decode(uint8_t* buf,
                                int32_t width,
                                int32_t height,
                                int32_t& errorCode);
  virtual CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& errorCode);
  virtual FX_BOOL SetCharEncoding(BC_CHAR_ENCODING encoding);
  virtual FX_BOOL SetModuleHeight(int32_t moduleHeight);
  virtual FX_BOOL SetModuleWidth(int32_t moduleWidth);
  virtual FX_BOOL SetHeight(int32_t height);
  virtual FX_BOOL SetWidth(int32_t width);
  virtual FX_BOOL CheckContentValidity(const CFX_WideStringC& contents);
  virtual FX_BOOL SetPrintChecksum(FX_BOOL checksum);
  virtual FX_BOOL SetDataLength(int32_t length);
  virtual FX_BOOL SetCalChecksum(int32_t state);
  virtual FX_BOOL SetFont(CFX_Font* pFont);
  virtual FX_BOOL SetFontSize(FX_FLOAT size);
  virtual FX_BOOL SetFontStyle(int32_t style);
  virtual FX_BOOL SetFontColor(FX_ARGB color);
  virtual FX_BOOL SetTextLocation(BC_TEXT_LOC location);
  virtual FX_BOOL SetWideNarrowRatio(int32_t ratio);
  virtual FX_BOOL SetStartChar(FX_CHAR start);
  virtual FX_BOOL SetEndChar(FX_CHAR end);
  virtual FX_BOOL SetVersion(int32_t version);
  virtual FX_BOOL SetErrorCorrectionLevel(int32_t level);
  virtual FX_BOOL SetTruncated(FX_BOOL truncated);

 protected:
  CBC_CodeBase* m_pBCEngine;
};
#endif
