// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_BinaryBitmap.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_PDF417DetectorResult.h"
#include "include/BC_BinaryBitmap.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_PDF417Detector.h"
#include "include/BC_PDF417DetectorResult.h"
#include "include/BC_DecoderResult.h"
#include "include/BC_PDF417Codeword.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_PDF417Common.h"
#include "include/BC_PDF417BarcodeValue.h"
#include "include/BC_PDF417BarcodeMetadata.h"
#include "include/BC_PDF417BoundingBox.h"
#include "include/BC_PDF417DetectionResultColumn.h"
#include "include/BC_PDF417DetectionResultRowIndicatorColumn.h"
#include "include/BC_PDF417DetectionResult.h"
#include "include/BC_PDF417DecodedBitStreamParser.h"
#include "include/BC_PDF417CodewordDecoder.h"
#include "include/BC_PDF417DecodedBitStreamParser.h"
#include "include/BC_PDF417ECModulusPoly.h"
#include "include/BC_PDF417ECModulusGF.h"
#include "include/BC_PDF417ECErrorCorrection.h"
#include "include/BC_PDF417DecodedBitStreamParser.h"
#include "include/BC_CommonDecoderResult.h"
#include "include/BC_PDF417ScanningDecoder.h"
#include "include/BC_PDF417Reader.h"
#define    Integer_MAX_VALUE   2147483647
CBC_PDF417Reader::CBC_PDF417Reader()
{
}
CBC_PDF417Reader::~CBC_PDF417Reader()
{
}
CFX_ByteString CBC_PDF417Reader::Decode(CBC_BinaryBitmap *image, FX_INT32 &e)
{
    return Decode(image, 0, e);
}
CFX_ByteString CBC_PDF417Reader::Decode(CBC_BinaryBitmap *image, FX_BOOL multiple, FX_INT32 hints, FX_INT32 &e)
{
    CFX_ByteString results;
    CBC_PDF417DetectorResult* detectorResult = CBC_Detector::detect(image, hints, multiple, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    for (FX_INT32 i = 0; i < detectorResult->getPoints()->GetSize(); i++) {
        CFX_PtrArray* points = (CFX_PtrArray*)detectorResult->getPoints()->GetAt(i);
        CBC_CommonDecoderResult* ResultTemp = CBC_PDF417ScanningDecoder::decode(detectorResult->getBits(), (CBC_ResultPoint*)points->GetAt(4), (CBC_ResultPoint*)points->GetAt(5),
                                              (CBC_ResultPoint*)points->GetAt(6), (CBC_ResultPoint*)points->GetAt(7), getMinCodewordWidth(*points), getMaxCodewordWidth(*points), e);
        if (ResultTemp == NULL) {
            delete detectorResult;
            e = BCExceptiontNotFoundInstance;
            return "";
        }
        results += ResultTemp->GetText();
        delete ResultTemp;
    }
    delete detectorResult;
    return results;
}
CFX_ByteString CBC_PDF417Reader::Decode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e)
{
    CFX_ByteString bs = Decode(image, FALSE, 0, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return bs;
}
FX_INT32 CBC_PDF417Reader::getMaxWidth(CBC_ResultPoint* p1, CBC_ResultPoint* p2)
{
    if (p1 == NULL || p2 == NULL) {
        return 0;
    }
    return (FX_INT32) FXSYS_fabs(p1->GetX() - p2->GetX());
}
FX_INT32 CBC_PDF417Reader::getMinWidth(CBC_ResultPoint* p1, CBC_ResultPoint* p2)
{
    if (p1 == NULL || p2 == NULL) {
        return Integer_MAX_VALUE;
    }
    return (FX_INT32) FXSYS_fabs(p1->GetX() - p2->GetX());
}
FX_INT32 CBC_PDF417Reader::getMaxCodewordWidth(CFX_PtrArray& p)
{
    FX_INT32 a = getMaxWidth((CBC_ResultPoint*)p.GetAt(6), (CBC_ResultPoint*)p.GetAt(2)) * CBC_PDF417Common::MODULES_IN_CODEWORD / CBC_PDF417Common::MODULES_IN_STOP_PATTERN;
    FX_INT32 b = getMaxWidth((CBC_ResultPoint*)p.GetAt(7), (CBC_ResultPoint*)p.GetAt(3)) * CBC_PDF417Common::MODULES_IN_CODEWORD / CBC_PDF417Common::MODULES_IN_STOP_PATTERN;
    FX_INT32 c = getMaxWidth((CBC_ResultPoint*)p.GetAt(0), (CBC_ResultPoint*)p.GetAt(4)) < a ? getMaxWidth((CBC_ResultPoint*)p.GetAt(0), (CBC_ResultPoint*)p.GetAt(4)) : a;
    FX_INT32 d = getMaxWidth((CBC_ResultPoint*)p.GetAt(1), (CBC_ResultPoint*)p.GetAt(5)) < b ? getMaxWidth((CBC_ResultPoint*)p.GetAt(1), (CBC_ResultPoint*)p.GetAt(5)) : b;
    return c < d ? c : d;
}
FX_INT32 CBC_PDF417Reader::getMinCodewordWidth(CFX_PtrArray& p)
{
    FX_INT32 a = getMinWidth((CBC_ResultPoint*)p.GetAt(6), (CBC_ResultPoint*)p.GetAt(2)) * CBC_PDF417Common::MODULES_IN_CODEWORD / CBC_PDF417Common::MODULES_IN_STOP_PATTERN;
    FX_INT32 b = getMinWidth((CBC_ResultPoint*)p.GetAt(7), (CBC_ResultPoint*)p.GetAt(3)) * CBC_PDF417Common::MODULES_IN_CODEWORD / CBC_PDF417Common::MODULES_IN_STOP_PATTERN;
    FX_INT32 c = getMinWidth((CBC_ResultPoint*)p.GetAt(0), (CBC_ResultPoint*)p.GetAt(4)) < a ? getMinWidth((CBC_ResultPoint*)p.GetAt(0), (CBC_ResultPoint*)p.GetAt(4)) : a;
    FX_INT32 d = getMinWidth((CBC_ResultPoint*)p.GetAt(1), (CBC_ResultPoint*)p.GetAt(5)) < b ? getMinWidth((CBC_ResultPoint*)p.GetAt(1), (CBC_ResultPoint*)p.GetAt(5)) : b;
    return c < d ? c : d;
}
