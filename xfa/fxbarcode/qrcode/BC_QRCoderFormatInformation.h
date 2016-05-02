// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_QRCODE_BC_QRCODERFORMATINFORMATION_H_
#define XFA_FXBARCODE_QRCODE_BC_QRCODERFORMATINFORMATION_H_

#include <stdint.h>

class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderFormatInformation {
 public:
  explicit CBC_QRCoderFormatInformation(int32_t formatInfo);
  ~CBC_QRCoderFormatInformation();

  uint8_t GetDataMask() const;
  CBC_QRCoderErrorCorrectionLevel* GetErrorCorrectionLevel();

  static int32_t NumBitsDiffering(int32_t a, int32_t b);
  static CBC_QRCoderFormatInformation* DecodeFormatInformation(
      int32_t maskedFormatInfo);

 private:
  CBC_QRCoderErrorCorrectionLevel* m_errorCorrectLevel;
  const uint8_t m_dataMask;
};

#endif  // XFA_FXBARCODE_QRCODE_BC_QRCODERFORMATINFORMATION_H_
