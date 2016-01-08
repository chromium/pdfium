// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERWRITER_H_
#define _BC_QRCODERWRITER_H_
#include "xfa/src/fxbarcode/BC_TwoDimWriter.h"
class CBC_TwoDimWriter;
class CBC_MultiBarCodes;
class CBC_QRCoderWriter;
class CBC_QRCodeWriter : public CBC_TwoDimWriter {
 public:
  CBC_QRCodeWriter();
  virtual ~CBC_QRCodeWriter();
  uint8_t* Encode(const CFX_WideString& contents,
                  int32_t ecLevel,
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
                  BCFORMAT format,
                  int32_t& outWidth,
                  int32_t& outHeight,
                  int32_t& e);
  FX_BOOL SetVersion(int32_t version);
  FX_BOOL SetErrorCorrectionLevel(int32_t level);
  static void ReleaseAll();

 private:
  int32_t m_iVersion;
};
#endif
