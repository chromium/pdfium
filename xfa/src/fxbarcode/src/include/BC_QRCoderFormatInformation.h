// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERFORMATINFORMATION_H_
#define _BC_QRCODERFORMATINFORMATION_H_
class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderFormatInformation;
class CBC_QRCoderFormatInformation  : public CFX_Object
{
private:
    const static FX_INT32 FORMAT_INFO_MASK_QR;
    const static FX_INT32 FORMAT_INFO_DECODE_LOOKUP[32][2];
    const static FX_INT32 BITS_SET_IN_HALF_BYTE[16];
    CBC_QRCoderErrorCorrectionLevel* m_errorCorrectLevl;
    FX_BYTE m_dataMask;
public:
    CBC_QRCoderFormatInformation(FX_INT32 formatInfo);
    virtual ~CBC_QRCoderFormatInformation();
    FX_BYTE GetDataMask();
    CBC_QRCoderErrorCorrectionLevel* GetErrorCorrectionLevel();

    static FX_INT32 NumBitsDiffering(FX_INT32 a, FX_INT32 b);
    static CBC_QRCoderFormatInformation* DecodeFormatInformation(FX_INT32 maskedFormatInfo);
    static CBC_QRCoderFormatInformation* DoDecodeFormatInformation(FX_INT32 maskedFormatInfo);
};
#endif
