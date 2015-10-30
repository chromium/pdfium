// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BUFFEREDIMAGELUMINANCESOURCE_H_
#define _BC_BUFFEREDIMAGELUMINANCESOURCE_H_
class CBC_LuminanceSource;
class CBC_BufferedImageLuminanceSource;
class CBC_BufferedImageLuminanceSource : public CBC_LuminanceSource {
 public:
  explicit CBC_BufferedImageLuminanceSource(const CFX_WideString& filename);
  explicit CBC_BufferedImageLuminanceSource(CFX_DIBitmap* pBitmap);
  virtual ~CBC_BufferedImageLuminanceSource();

  CFX_ByteArray* GetRow(int32_t y, CFX_ByteArray& row, int32_t& e);
  CFX_ByteArray* GetMatrix();
  virtual void Init(int32_t& e);

 private:
  int32_t m_bytesPerLine;
  int32_t m_left;
  int32_t m_top;
  CFX_Int32Array m_rgbData;
  CFX_DIBitmap* m_pBitmap;
  const CFX_WideString m_filename;
};
#endif
