// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERFORMATINFORMATION_H_
#define _BC_QRCODERFORMATINFORMATION_H_
class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderFormatInformation {
 private:
  const static int32_t FORMAT_INFO_MASK_QR;
  const static int32_t FORMAT_INFO_DECODE_LOOKUP[32][2];
  const static int32_t BITS_SET_IN_HALF_BYTE[16];
  CBC_QRCoderErrorCorrectionLevel* m_errorCorrectLevl;
  uint8_t m_dataMask;

 public:
  CBC_QRCoderFormatInformation(int32_t formatInfo);
  virtual ~CBC_QRCoderFormatInformation();
  uint8_t GetDataMask();
  CBC_QRCoderErrorCorrectionLevel* GetErrorCorrectionLevel();

  static int32_t NumBitsDiffering(int32_t a, int32_t b);
  static CBC_QRCoderFormatInformation* DecodeFormatInformation(
      int32_t maskedFormatInfo);
  static CBC_QRCoderFormatInformation* DoDecodeFormatInformation(
      int32_t maskedFormatInfo);
};
#endif
