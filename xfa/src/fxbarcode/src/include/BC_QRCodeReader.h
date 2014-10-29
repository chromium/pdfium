// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODEREADER_H_
#define _BC_QRCODEREADER_H_
class CBC_QRDetector;
class CBC_BinaryBitmap;
class CBC_QRCoderDecoder;
class CBC_ResultPoint;
class CBC_ReedSolomonGF256;
class CBC_QRCoderVersion;
class CBC_QRDetector;
class CBC_QRDetectorResult;
class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderMode;
class CBC_QRDataMask;
class CBC_QRCodeReader;
class CBC_QRCodeReader  : public CBC_Reader
{
private:
    CBC_QRCoderDecoder *m_decoder;
public:
    CBC_QRCodeReader();
    virtual ~CBC_QRCodeReader();
    CFX_ByteString Decode(CFX_DIBitmap *pBitmap, FX_INT32 hints, FX_INT32 byteModeDecode, FX_INT32 &e);
    CFX_ByteString Decode(const CFX_WideString &filename, FX_INT32 hints, FX_INT32 byteModeDecode, FX_INT32 &e);
    static void ReleaseAll();
    CFX_ByteString Decode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e);
    CFX_ByteString Decode(CBC_BinaryBitmap *image, FX_INT32 &e);
    virtual void Init();
};
#endif
