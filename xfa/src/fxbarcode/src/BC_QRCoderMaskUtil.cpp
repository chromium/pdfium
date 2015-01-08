// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
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
#include "include/BC_CommonByteMatrix.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
#include "include/BC_QRCoder.h"
#include "include/BC_QRCoderMaskUtil.h"
CBC_QRCoderMaskUtil::CBC_QRCoderMaskUtil()
{
}
CBC_QRCoderMaskUtil::~CBC_QRCoderMaskUtil()
{
}
FX_INT32 CBC_QRCoderMaskUtil::ApplyMaskPenaltyRule1(CBC_CommonByteMatrix* matrix)
{
    return ApplyMaskPenaltyRule1Internal(matrix, TRUE) +
           ApplyMaskPenaltyRule1Internal(matrix, FALSE);
}
FX_INT32 CBC_QRCoderMaskUtil::ApplyMaskPenaltyRule2(CBC_CommonByteMatrix* matrix)
{
    FX_INT32 penalty = 0;
    FX_BYTE* array = matrix->GetArray();
    FX_INT32 width = matrix->GetWidth();
    FX_INT32 height = matrix->GetHeight();
    for(FX_INT32 y = 0; y < height - 1; y++) {
        for(FX_INT32 x = 0; x < width - 1; x++) {
            FX_INT32 value = array[y * width + x];
            if(value == array[y * width + x + 1] &&
                    value == array[(y + 1) * width + x] &&
                    value == array[(y + 1) * width + x + 1]) {
                penalty ++;
            }
        }
    }
    return 3 * penalty;
}
FX_INT32 CBC_QRCoderMaskUtil::ApplyMaskPenaltyRule3(CBC_CommonByteMatrix* matrix)
{
    FX_INT32 penalty = 0;
    FX_BYTE* array = matrix->GetArray();
    FX_INT32 width = matrix->GetWidth();
    FX_INT32 height = matrix->GetHeight();
    for (FX_INT32 y = 0; y < height; ++y) {
        for (FX_INT32 x = 0; x < width; ++x) {
            if (x == 0 && ((y >= 0 && y <= 6) || (y >= height - 7 && y <= height - 1))) {
                continue;
            }
            if (x == width - 7 && (y >= 0 && y <= 6)) {
                continue;
            }
            if (y == 0 && ((x >= 0 && x <= 6) || (x >= width - 7 && x <= width - 1))) {
                continue;
            }
            if (y == height - 7 && (x >= 0 && x <= 6)) {
                continue;
            }
            if (x + 6 < width &&
                    array[y * width + x] == 1 &&
                    array[y * width + x +  1] == 0 &&
                    array[y * width + x +  2] == 1 &&
                    array[y * width + x +  3] == 1 &&
                    array[y * width + x +  4] == 1 &&
                    array[y * width + x +  5] == 0 &&
                    array[y * width + x +  6] == 1 &&
                    ((x + 10 < width &&
                      array[y * width + x +  7] == 0 &&
                      array[y * width + x +  8] == 0 &&
                      array[y * width + x +  9] == 0 &&
                      array[y * width + x + 10] == 0) ||
                     (x - 4 >= 0 &&
                      array[y * width + x -  1] == 0 &&
                      array[y * width + x -  2] == 0 &&
                      array[y * width + x -  3] == 0 &&
                      array[y * width + x -  4] == 0))) {
                penalty += 40;
            }
            if (y + 6 < height &&
                    array[y * width + x] == 1  &&
                    array[(y +  1) * width + x] == 0  &&
                    array[(y +  2) * width + x] == 1  &&
                    array[(y +  3) * width + x] == 1  &&
                    array[(y +  4) * width + x] == 1  &&
                    array[(y +  5) * width + x] == 0  &&
                    array[(y +  6) * width + x] == 1 &&
                    ((y + 10 < height &&
                      array[(y +  7) * width + x] == 0 &&
                      array[(y +  8) * width + x] == 0 &&
                      array[(y +  9) * width + x] == 0 &&
                      array[(y + 10) * width + x] == 0) ||
                     (y - 4 >= 0 &&
                      array[(y -  1) * width + x] == 0 &&
                      array[(y -  2) * width + x] == 0 &&
                      array[(y -  3) * width + x] == 0 &&
                      array[(y -  4) * width + x] == 0))) {
                penalty += 40;
            }
        }
    }
    return penalty;
}
FX_INT32 CBC_QRCoderMaskUtil::ApplyMaskPenaltyRule4(CBC_CommonByteMatrix* matrix)
{
    FX_INT32 numDarkCells = 0;
    FX_BYTE* array = matrix->GetArray();
    FX_INT32 width = matrix->GetWidth();
    FX_INT32 height = matrix->GetHeight();
    for (FX_INT32 y = 0; y < height; ++y) {
        for (FX_INT32 x = 0; x < width; ++x) {
            if (array[y * width + x] == 1) {
                numDarkCells += 1;
            }
        }
    }
    FX_INT32 numTotalCells = matrix->GetHeight() * matrix->GetWidth();
    double darkRatio = (double) numDarkCells / numTotalCells;
    return abs( (FX_INT32) (darkRatio * 100 - 50) / 5 ) * 5 * 10;
}
FX_BOOL CBC_QRCoderMaskUtil::GetDataMaskBit(FX_INT32 maskPattern, FX_INT32 x, FX_INT32 y, FX_INT32 &e)
{
    if(!CBC_QRCoder::IsValidMaskPattern(maskPattern)) {
        e = (BCExceptionInvalidateMaskPattern);
        BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
    }
    FX_INT32 intermediate = 0, temp = 0;
    switch(maskPattern) {
        case 0:
            intermediate = (y + x) & 0x1;
            break;
        case 1:
            intermediate = y & 0x1;
            break;
        case 2:
            intermediate = x % 3;
            break;
        case 3:
            intermediate = (y + x) % 3;
            break;
        case 4:
            intermediate = ((y >> 1) + (x / 3)) & 0x1;
            break;
        case 5:
            temp = y * x;
            intermediate = (temp & 0x1) + (temp % 3);
            break;
        case 6:
            temp = y * x;
            intermediate = (((temp & 0x1) + (temp % 3)) & 0x1);
            break;
        case 7:
            temp = y * x;
            intermediate = (((temp % 3) + ((y + x) & 0x1)) & 0x1);
            break;
        default: {
                e = BCExceptionInvalidateMaskPattern;
                BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
            }
    }
    return intermediate == 0;
}
FX_INT32 CBC_QRCoderMaskUtil::ApplyMaskPenaltyRule1Internal(CBC_CommonByteMatrix* matrix, FX_BOOL isHorizontal)
{
    FX_INT32 penalty = 0;
    FX_INT32 numSameBitCells = 0;
    FX_INT32 prevBit = -1;
    FX_INT32 width = matrix->GetWidth();
    FX_INT32 height = matrix->GetHeight();
    FX_INT32 iLimit = isHorizontal ? height : width;
    FX_INT32 jLimit = isHorizontal ? width : height;
    FX_BYTE* array = matrix->GetArray();
    for (FX_INT32 i = 0; i < iLimit; ++i) {
        for (FX_INT32 j = 0; j < jLimit; ++j) {
            FX_INT32 bit = isHorizontal ? array[i * width + j] : array[j * width + i];
            if (bit == prevBit) {
                numSameBitCells += 1;
                if (numSameBitCells == 5) {
                    penalty += 3;
                } else if (numSameBitCells > 5) {
                    penalty += 1;
                }
            } else {
                numSameBitCells = 1;
                prevBit = bit;
            }
        }
        numSameBitCells = 0;
    }
    return penalty;
}
