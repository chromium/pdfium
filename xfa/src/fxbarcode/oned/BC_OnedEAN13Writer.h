// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDEAN13WRITER_H_
#define _BC_ONEDEAN13WRITER_H_
class CBC_OneDimWriter;
class CBC_OnedEAN13Writer;
class CBC_OnedEAN13Writer : public CBC_OneDimWriter {
 private:
  int32_t m_codeWidth;

 public:
  CBC_OnedEAN13Writer();
  virtual ~CBC_OnedEAN13Writer();

  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t& e);
  uint8_t* Encode(const CFX_ByteString& contents,
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t hints,
                  int32_t& e);
  uint8_t* Encode(const CFX_ByteString& contents,
                  int32_t& outLength,
                  int32_t& e);
  void RenderResult(const CFX_WideStringC& contents,
                    uint8_t* code,
                    int32_t codeLength,
                    FX_BOOL isDevice,
                    int32_t& e);
  FX_BOOL CheckContentValidity(const CFX_WideStringC& contents);
  CFX_WideString FilterContents(const CFX_WideStringC& contents);
  int32_t CalcChecksum(const CFX_ByteString& contents);

 protected:
  void ShowChars(const CFX_WideStringC& contents,
                 CFX_DIBitmap* pOutBitmap,
                 CFX_RenderDevice* device,
                 const CFX_Matrix* matrix,
                 int32_t barWidth,
                 int32_t multiple,
                 int32_t& e);
};
#endif
