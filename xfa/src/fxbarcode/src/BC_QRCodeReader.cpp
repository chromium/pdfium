// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_QRCodeReader.h"
#include "include/BC_LuminanceSource.h"
#include "include/BC_BufferedImageLuminanceSource.h"
#include "include/BC_Binarizer.h"
#include "include/BC_GlobalHistogramBinarizer.h"
#include "include/BC_BinaryBitmap.h"
#include "include/BC_QRCodeReader.h"
#include "include/BC_QRCoderMode.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_QRCoderDecoder.h"
#include "include/BC_QRDetector.h"
#include "include/BC_QRDetectorResult.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
#include "include/BC_QRDataMask.h"
#include "include/BC_ReedSolomonGF256.h"
#include "include/BC_QRCoderVersion.h"
#include "include/BC_CommonDecoderResult.h"
#include "include/BC_QRCodeReader.h"
CBC_QRCodeReader::CBC_QRCodeReader(): m_decoder(NULL)
{
}
void CBC_QRCodeReader::Init()
{
    m_decoder = FX_NEW CBC_QRCoderDecoder;
    m_decoder->Init();
}
CBC_QRCodeReader::~CBC_QRCodeReader()
{
    if(m_decoder != NULL) {
        delete m_decoder;
    }
    m_decoder = NULL;
}
CFX_ByteString CBC_QRCodeReader::Decode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e)
{
    CBC_CommonBitMatrix *matrix = image->GetMatrix(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CBC_QRDetector detector(matrix);
    CBC_QRDetectorResult* qdr = detector.Detect(hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CBC_AutoPtr<CBC_QRDetectorResult> detectorResult(qdr);
    CBC_CommonDecoderResult* qdr2 = m_decoder->Decode(detectorResult->GetBits(), 0, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CBC_AutoPtr<CBC_CommonDecoderResult> decodeResult(qdr2);
    return (decodeResult->GetText());
}
CFX_ByteString CBC_QRCodeReader::Decode(const CFX_WideString &filename, FX_INT32 hints, FX_INT32 byteModeDecode, FX_INT32 &e)
{
    CBC_BufferedImageLuminanceSource source(filename);
    source.Init(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CBC_GlobalHistogramBinarizer binarizer(&source);
    CBC_BinaryBitmap bitmap(&binarizer);
    CFX_ByteString bs = Decode(&bitmap, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return bs;
}
CFX_ByteString CBC_QRCodeReader::Decode(CFX_DIBitmap *pBitmap, FX_INT32 hints, FX_INT32 byteModeDecode, FX_INT32 &e)
{
    CBC_BufferedImageLuminanceSource source(pBitmap);
    CBC_GlobalHistogramBinarizer binarizer(&source);
    CBC_BinaryBitmap bitmap(&binarizer);
    CFX_ByteString bs = Decode(&bitmap, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return bs;
}
CFX_ByteString CBC_QRCodeReader::Decode(CBC_BinaryBitmap *image, FX_INT32 &e)
{
    CFX_ByteString bs = Decode(image, 0, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return bs;
}
void CBC_QRCodeReader::ReleaseAll()
{
    if(CBC_ReedSolomonGF256 ::QRCodeFild) {
        delete CBC_ReedSolomonGF256 ::QRCodeFild;
        CBC_ReedSolomonGF256 ::QRCodeFild = NULL;
    }
    if(CBC_ReedSolomonGF256 ::DataMatrixField) {
        delete CBC_ReedSolomonGF256 ::DataMatrixField;
        CBC_ReedSolomonGF256 ::DataMatrixField = NULL;
    }
    CBC_QRCoderMode::Destroy();
    CBC_QRCoderErrorCorrectionLevel::Destroy();
    CBC_QRDataMask::Destroy();
    CBC_QRCoderVersion::Destroy();
}
