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

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_Binarizer.h"
#include "xfa/src/fxbarcode/BC_LuminanceSource.h"
#include "BC_CommonBitMatrix.h"
#include "BC_CommonBitArray.h"
#include "BC_GlobalHistogramBinarizer.h"
const int32_t LUMINANCE_BITS = 5;
const int32_t LUMINANCE_SHIFT = 8 - LUMINANCE_BITS;
const int32_t LUMINANCE_BUCKETS = 1 << LUMINANCE_BITS;
CBC_GlobalHistogramBinarizer::CBC_GlobalHistogramBinarizer(
    CBC_LuminanceSource* source)
    : CBC_Binarizer(source) {}
CBC_GlobalHistogramBinarizer::~CBC_GlobalHistogramBinarizer() {}
CBC_CommonBitArray* CBC_GlobalHistogramBinarizer::GetBlackRow(
    int32_t y,
    CBC_CommonBitArray* row,
    int32_t& e) {
  CBC_LuminanceSource* source = GetLuminanceSource();
  int32_t width = source->GetWidth();
  CBC_AutoPtr<CBC_CommonBitArray> result(new CBC_CommonBitArray(width));
  InitArrays(width);
  CFX_ByteArray* localLuminances = source->GetRow(y, m_luminance, e);
  if (e != BCExceptionNO) {
    return result.release();
  }
  CFX_Int32Array localBuckets;
  localBuckets.Copy(m_buckets);
  int32_t x;
  for (x = 0; x < width; x++) {
    int32_t pixel = (*localLuminances)[x] & 0xff;
    localBuckets[pixel >> LUMINANCE_SHIFT]++;
  }
  int32_t blackPoint = EstimateBlackPoint(localBuckets, e);
  if (e != BCExceptionNO) {
    return result.release();
  }
  int32_t left = (*localLuminances)[0] & 0xff;
  int32_t center = (*localLuminances)[1] & 0xff;
  for (x = 1; x < width - 1; x++) {
    int32_t right = (*localLuminances)[x + 1] & 0xff;
    int32_t luminance = ((center << 2) - left - right) >> 1;
    if (luminance < blackPoint) {
      result->Set(x);
    }
    left = center;
    center = right;
  }
  return result.release();
}
CBC_CommonBitMatrix* CBC_GlobalHistogramBinarizer::GetBlackMatrix(int32_t& e) {
  CBC_LuminanceSource* source = GetLuminanceSource();
  int32_t width = source->GetWidth();
  int32_t height = source->GetHeight();
  CBC_CommonBitMatrix* BitMatrixTemp = new CBC_CommonBitMatrix();
  BitMatrixTemp->Init(width, height);
  CBC_AutoPtr<CBC_CommonBitMatrix> matrix(BitMatrixTemp);
  InitArrays(width);
  CFX_Int32Array localBuckets;
  localBuckets.Copy(m_buckets);
  int32_t y;
  for (y = 1; y < 5; y++) {
    int32_t row = height * y / 5;
    CFX_ByteArray* localLuminances = source->GetRow(row, m_luminance, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    int32_t right = (width << 2) / 5;
    int32_t x;
    for (x = width / 5; x < right; x++) {
      int32_t pixel = (*localLuminances)[x] & 0xff;
      localBuckets[pixel >> LUMINANCE_SHIFT]++;
    }
  }
  int32_t blackPoint = EstimateBlackPoint(localBuckets, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_AutoPtr<CFX_ByteArray> localLuminances(source->GetMatrix());
  for (y = 0; y < height; y++) {
    int32_t offset = y * width;
    for (int32_t x = 0; x < width; x++) {
      int32_t pixel = (*localLuminances)[offset + x] & 0xff;
      if (pixel < blackPoint) {
        matrix->Set(x, y);
      }
    }
  }
  return matrix.release();
}
void CBC_GlobalHistogramBinarizer::InitArrays(int32_t luminanceSize) {
  if (m_luminance.GetSize() < luminanceSize) {
    m_luminance.SetSize(luminanceSize);
  }
  if (m_buckets.GetSize() <= 0) {
    m_buckets.SetSize(LUMINANCE_BUCKETS);
  } else {
    int32_t x;
    for (x = 0; x < LUMINANCE_BUCKETS; x++) {
      m_buckets[x] = 0;
    }
  }
}
int32_t CBC_GlobalHistogramBinarizer::EstimateBlackPoint(
    CFX_Int32Array& buckets,
    int32_t& e) {
  int32_t numBuckets = buckets.GetSize();
  int32_t maxBucketCount = 0;
  int32_t firstPeak = 0;
  int32_t firstPeakSize = 0;
  int32_t x;
  for (x = 0; x < numBuckets; x++) {
    if (buckets[x] > firstPeakSize) {
      firstPeak = x;
      firstPeakSize = buckets[x];
    }
    if (buckets[x] > maxBucketCount) {
      maxBucketCount = buckets[x];
    }
  }
  int32_t secondPeak = 0;
  int32_t secondPeakScore = 0;
  for (x = 0; x < numBuckets; x++) {
    int32_t distanceToBiggest = x - firstPeak;
    int32_t score = buckets[x] * distanceToBiggest * distanceToBiggest;
    if (score > secondPeakScore) {
      secondPeak = x;
      secondPeakScore = score;
    }
  }
  if (firstPeak > secondPeak) {
    int32_t temp = firstPeak;
    firstPeak = secondPeak;
    secondPeak = temp;
  }
  if (secondPeak - firstPeak <= numBuckets >> 4) {
    e = BCExceptionRead;
    return 0;
  }
  int32_t bestValley = secondPeak - 1;
  int32_t bestValleyScore = -1;
  for (x = secondPeak - 1; x > firstPeak; x--) {
    int32_t fromFirst = x - firstPeak;
    int32_t score = fromFirst * fromFirst * (secondPeak - x) *
                    (maxBucketCount - buckets[x]);
    if (score > bestValleyScore) {
      bestValley = x;
      bestValleyScore = score;
    }
  }
  return bestValley << LUMINANCE_SHIFT;
}
