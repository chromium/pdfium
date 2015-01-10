// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../barcode.h"
#include "../BC_Reader.h"
#include "../BC_LuminanceSource.h"
#include "../BC_BufferedImageLuminanceSource.h"
#include "../BC_Binarizer.h"
#include "../BC_BinaryBitmap.h"
#include "../BC_ResultPoint.h"
#include "../common/BC_GlobalHistogramBinarizer.h"
#include "../common/BC_CommonDecoderResult.h"
#include "../common/reedsolomon/BC_ReedSolomonGF256.h"
#include "BC_QRCodeReader.h"
#include "BC_QRCodeReader.h"
#include "BC_QRCoderMode.h"
#include "BC_QRCoderDecoder.h"
#include "BC_QRDetector.h"
#include "BC_QRDetectorResult.h"
#include "BC_QRCoderErrorCorrectionLevel.h"
#include "BC_QRDataMask.h"
#include "BC_QRCodeReader.h"
#include "BC_QRCoderVersion.h"
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
