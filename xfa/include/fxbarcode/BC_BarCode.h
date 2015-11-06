// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FXBARCODE_BC_BARCODE_H_
#define XFA_INCLUDE_FXBARCODE_BC_BARCODE_H_

#include "core/include/fxcrt/fx_string.h"
#include "core/include/fxcrt/fx_system.h"
#include "core/include/fxge/fx_dib.h"

class CBC_Reader;
class CBC_Writer;
class CFX_Font;
class CFX_RenderDevice;

enum BC_TEXT_LOC {
  BC_TEXT_LOC_NONE = 0,
  BC_TEXT_LOC_ABOVE,
  BC_TEXT_LOC_BELOW,
  BC_TEXT_LOC_ABOVEEMBED,
  BC_TEXT_LOC_BELOWEMBED
};

enum BC_CHAR_ENCODING { CHAR_ENCODING_UTF8 = 0, CHAR_ENCODING_UNICODE };

enum BC_TYPE {
  BC_UNKNOWN = -1,
  BC_CODE39 = 0,
  BC_CODABAR,
  BC_CODE128,
  BC_CODE128_B,
  BC_CODE128_C,
  BC_EAN8,
  BC_UPCA,
  BC_EAN13,
  BC_QR_CODE,
  BC_PDF417,
  BC_DATAMATRIX
};

void BC_Library_Init();
void BC_Library_Destory();

class CBC_CodeBase {
 public:
  CBC_CodeBase();
  virtual ~CBC_CodeBase();
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
                                int32_t hight,
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
class CBC_OneCode : public CBC_CodeBase {
 public:
  CBC_OneCode();
  virtual ~CBC_OneCode();
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
                                int32_t hight,
                                int32_t& e) = 0;
  virtual CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e) = 0;
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
class CBC_Code39 : public CBC_OneCode {
 public:
  CBC_Code39();
  CBC_Code39(FX_BOOL usingCheckDigit);
  CBC_Code39(FX_BOOL usingCheckDigit, FX_BOOL extendedMode);
  virtual ~CBC_Code39();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_CODE39; }
  FX_BOOL SetTextLocation(BC_TEXT_LOC location);
  FX_BOOL SetWideNarrowRatio(int32_t ratio);

 private:
  CFX_WideString m_renderContents;
};
class CBC_Codabar : public CBC_OneCode {
 public:
  CBC_Codabar();
  virtual ~CBC_Codabar();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_CODABAR; }
  FX_BOOL SetStartChar(FX_CHAR start);
  FX_BOOL SetEndChar(FX_CHAR end);
  FX_BOOL SetTextLocation(BC_TEXT_LOC location);
  FX_BOOL SetWideNarrowRatio(int32_t ratio);

 private:
  CFX_WideString m_renderContents;
};
class CBC_Code128 : public CBC_OneCode {
 public:
  CBC_Code128(BC_TYPE type);
  virtual ~CBC_Code128();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_CODE128; }
  FX_BOOL SetTextLocation(BC_TEXT_LOC loction);

 private:
  CFX_WideString m_renderContents;
};
class CBC_EAN8 : public CBC_OneCode {
 public:
  CBC_EAN8();
  virtual ~CBC_EAN8();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_EAN8; }

 private:
  CFX_WideString Preprocess(const CFX_WideStringC& contents);
  CFX_WideString m_renderContents;
};
class CBC_EAN13 : public CBC_OneCode {
 public:
  CBC_EAN13();
  virtual ~CBC_EAN13();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_EAN13; }

 private:
  CFX_WideString Preprocess(const CFX_WideStringC& contents);
  CFX_WideString m_renderContents;
};
class CBC_UPCA : public CBC_OneCode {
 public:
  CBC_UPCA();
  virtual ~CBC_UPCA();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_UPCA; }

 private:
  CFX_WideString Preprocess(const CFX_WideStringC& contents);
  CFX_WideString m_renderContents;
};
class CBC_QRCode : public CBC_CodeBase {
 public:
  CBC_QRCode();
  virtual ~CBC_QRCode();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_QR_CODE; }
  FX_BOOL SetVersion(int32_t version);
  FX_BOOL SetErrorCorrectionLevel(int32_t level);
};
class CBC_PDF417I : public CBC_CodeBase {
 public:
  CBC_PDF417I();
  virtual ~CBC_PDF417I();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_PDF417; }
  FX_BOOL SetErrorCorrectionLevel(int32_t level);
  void SetTruncated(FX_BOOL truncated);
};
class CBC_DataMatrix : public CBC_CodeBase {
 public:
  CBC_DataMatrix();
  virtual ~CBC_DataMatrix();
  FX_BOOL Encode(const CFX_WideStringC& contents, FX_BOOL isDevice, int32_t& e);
  FX_BOOL RenderDevice(CFX_RenderDevice* device,
                       const CFX_Matrix* matirx,
                       int32_t& e);
  FX_BOOL RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e);
  CFX_WideString Decode(uint8_t* buf, int32_t width, int32_t hight, int32_t& e);
  CFX_WideString Decode(CFX_DIBitmap* pBitmap, int32_t& e);
  BC_TYPE GetType() { return BC_DATAMATRIX; }
};

#endif  // XFA_INCLUDE_FXBARCODE_BC_BARCODE_H_
