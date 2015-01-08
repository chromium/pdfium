// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
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

#include "barcode.h"
#include "include/BC_Binarizer.h"
#include "include/BC_LuminanceSource.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_GlobalHistogramBinarizer.h"
const FX_INT32 LUMINANCE_BITS = 5;
const FX_INT32 LUMINANCE_SHIFT = 8 - LUMINANCE_BITS;
const FX_INT32 LUMINANCE_BUCKETS = 1 << LUMINANCE_BITS;
CBC_GlobalHistogramBinarizer::CBC_GlobalHistogramBinarizer(CBC_LuminanceSource *source): CBC_Binarizer(source)
{
}
CBC_GlobalHistogramBinarizer::~CBC_GlobalHistogramBinarizer()
{
}
CBC_CommonBitArray *CBC_GlobalHistogramBinarizer::GetBlackRow(FX_INT32 y, CBC_CommonBitArray *row, FX_INT32 &e)
{
    CBC_LuminanceSource *source = GetLuminanceSource();
    FX_INT32 width = source->GetWidth();
    CBC_AutoPtr<CBC_CommonBitArray> result(FX_NEW CBC_CommonBitArray(width));
    InitArrays(width);
    CFX_ByteArray *localLuminances = source->GetRow(y, m_luminance, e);
    if (e != BCExceptionNO) {
        return result.release();
    }
    CFX_Int32Array localBuckets;
    localBuckets.Copy(m_buckets);
    FX_INT32 x;
    for (x = 0; x < width; x++) {
        FX_INT32 pixel = (*localLuminances)[x] & 0xff;
        localBuckets[pixel >> LUMINANCE_SHIFT]++;
    }
    FX_INT32 blackPoint = EstimateBlackPoint(localBuckets, e);
    if (e != BCExceptionNO) {
        return result.release();
    }
    FX_INT32 left = (*localLuminances)[0] & 0xff;
    FX_INT32 center = (*localLuminances)[1] & 0xff;
    for (x = 1; x < width - 1; x++) {
        FX_INT32 right = (*localLuminances)[x + 1] & 0xff;
        FX_INT32 luminance = ((center << 2) - left - right) >> 1;
        if (luminance < blackPoint) {
            result->Set(x);
        }
        left = center;
        center = right;
    }
    return  result.release();
}
CBC_CommonBitMatrix *CBC_GlobalHistogramBinarizer::GetBlackMatrix(FX_INT32 &e)
{
    CBC_LuminanceSource *source = GetLuminanceSource();
    FX_INT32 width = source->GetWidth();
    FX_INT32 height = source->GetHeight();
    CBC_CommonBitMatrix *BitMatrixTemp = FX_NEW CBC_CommonBitMatrix();
    BitMatrixTemp->Init(width, height);
    CBC_AutoPtr<CBC_CommonBitMatrix> matrix(BitMatrixTemp);
    InitArrays(width);
    CFX_Int32Array localBuckets;
    localBuckets.Copy(m_buckets);
    FX_INT32 y;
    for (y = 1; y < 5; y++) {
        FX_INT32 row = height * y / 5;
        CFX_ByteArray *localLuminances = source->GetRow(row, m_luminance, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        FX_INT32 right = (width << 2) / 5;
        FX_INT32 x;
        for (x = width / 5; x < right; x++) {
            FX_INT32 pixel = (*localLuminances)[x] & 0xff;
            localBuckets[pixel >> LUMINANCE_SHIFT]++;
        }
    }
    FX_INT32 blackPoint = EstimateBlackPoint(localBuckets, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_AutoPtr<CFX_ByteArray> localLuminances(source->GetMatrix());
    for (y = 0; y < height; y++) {
        FX_INT32 offset = y * width;
        for (FX_INT32 x = 0; x < width; x++) {
            FX_INT32 pixel = (*localLuminances)[offset + x] & 0xff;
            if (pixel < blackPoint) {
                matrix->Set(x, y);
            }
        }
    }
    return matrix.release();
}
void CBC_GlobalHistogramBinarizer::InitArrays(FX_INT32 luminanceSize)
{
    if(m_luminance.GetSize() < luminanceSize) {
        m_luminance.SetSize(luminanceSize);
    }
    if(m_buckets.GetSize() <= 0) {
        m_buckets.SetSize(LUMINANCE_BUCKETS);
    } else {
        FX_INT32 x;
        for(x = 0; x < LUMINANCE_BUCKETS; x++) {
            m_buckets[x] = 0;
        }
    }
}
FX_INT32 CBC_GlobalHistogramBinarizer::EstimateBlackPoint(CFX_Int32Array &buckets, FX_INT32 &e)
{
    FX_INT32 numBuckets = buckets.GetSize();
    FX_INT32 maxBucketCount = 0;
    FX_INT32 firstPeak = 0;
    FX_INT32 firstPeakSize = 0;
    FX_INT32 x;
    for (x = 0; x < numBuckets; x++) {
        if (buckets[x] > firstPeakSize) {
            firstPeak = x;
            firstPeakSize = buckets[x];
        }
        if (buckets[x] > maxBucketCount) {
            maxBucketCount = buckets[x];
        }
    }
    FX_INT32 secondPeak = 0;
    FX_INT32 secondPeakScore = 0;
    for (x = 0; x < numBuckets; x++) {
        FX_INT32 distanceToBiggest = x - firstPeak;
        FX_INT32 score = buckets[x] * distanceToBiggest * distanceToBiggest;
        if (score > secondPeakScore) {
            secondPeak = x;
            secondPeakScore = score;
        }
    }
    if (firstPeak > secondPeak) {
        FX_INT32 temp = firstPeak;
        firstPeak = secondPeak;
        secondPeak = temp;
    }
    if (secondPeak - firstPeak <= numBuckets >> 4) {
        e = BCExceptionRead;
        return 0;
    }
    FX_INT32 bestValley = secondPeak - 1;
    FX_INT32 bestValleyScore = -1;
    for (x = secondPeak - 1; x > firstPeak; x--) {
        FX_INT32 fromFirst = x - firstPeak;
        FX_INT32 score = fromFirst * fromFirst * (secondPeak - x) * (maxBucketCount - buckets[x]);
        if (score > bestValleyScore) {
            bestValley = x;
            bestValleyScore = score;
        }
    }
    return bestValley << LUMINANCE_SHIFT;
}
