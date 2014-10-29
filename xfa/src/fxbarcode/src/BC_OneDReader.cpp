// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_OneDReader.h"
#include "include/BC_BinaryBitmap.h"
#include "include/BC_CommonBitArray.h"
const FX_INT32 CBC_OneDReader::INTEGER_MATH_SHIFT = 8;
const FX_INT32 CBC_OneDReader::PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << 8;
CBC_OneDReader::CBC_OneDReader()
{
}
CBC_OneDReader::~CBC_OneDReader()
{
}
CFX_ByteString CBC_OneDReader::Decode(CBC_BinaryBitmap *image, FX_INT32 &e)
{
    CFX_ByteString strtemp = Decode(image, 0, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return strtemp;
}
CFX_ByteString CBC_OneDReader::Decode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e)
{
    CFX_ByteString strtemp = DeDecode(image, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return strtemp;
}
CFX_ByteString CBC_OneDReader::DeDecode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e)
{
    FX_INT32 width = image->GetWidth();
    FX_INT32 height = image->GetHeight();
    CBC_CommonBitArray *row = NULL;
    FX_INT32 middle = height >> 1;
    FX_BOOL tryHarder = FALSE;
    FX_INT32 rowStep = FX_MAX(1, height >> (tryHarder ? 8 : 5));
    FX_INT32 maxLines;
    if (tryHarder) {
        maxLines = height;
    } else {
        maxLines = 15;
    }
    for (FX_INT32 x = 0; x < maxLines; x++) {
        FX_INT32 rowStepsAboveOrBelow = (x + 1) >> 1;
        FX_BOOL isAbove = (x & 0x01) == 0;
        FX_INT32 rowNumber = middle + rowStep * (isAbove ? rowStepsAboveOrBelow : -rowStepsAboveOrBelow);
        if (rowNumber < 0 || rowNumber >= height) {
            break;
        }
        row = image->GetBlackRow(rowNumber, NULL, e);
        if (e != BCExceptionNO) {
            e = BCExceptionNO;
            if(row != NULL) {
                delete row;
                row = NULL;
            }
            continue;
        }
        for (FX_INT32 attempt = 0; attempt < 2; attempt++) {
            if (attempt == 1) {
                row->Reverse();
            }
            CFX_ByteString result = DecodeRow(rowNumber, row, hints, e);
            if (e != BCExceptionNO) {
                e = BCExceptionNO;
                continue;
            }
            if(row != NULL) {
                delete row;
                row = NULL;
            }
            return result;
        }
        if(row != NULL) {
            delete row;
            row = NULL;
        }
    }
    e = BCExceptionNotFound;
    return "";
}
void CBC_OneDReader::RecordPattern(CBC_CommonBitArray *row, FX_INT32 start, CFX_Int32Array *counters, FX_INT32 &e)
{
    FX_INT32 numCounters = counters->GetSize();
    for (FX_INT32 i = 0; i < numCounters; i++) {
        (*counters)[i] = 0;
    }
    FX_INT32 end = row->GetSize();
    if (start >= end) {
        e = BCExceptionNotFound;
        return;
    }
    FX_BOOL isWhite = !row->Get(start);
    FX_INT32 counterPosition = 0;
    FX_INT32 j = start;
    while (j < end) {
        FX_BOOL pixel = row->Get(j);
        if (pixel ^ isWhite) {
            (*counters)[counterPosition]++;
        } else {
            counterPosition++;
            if (counterPosition == numCounters) {
                break;
            } else {
                (*counters)[counterPosition] = 1;
                isWhite = !isWhite;
            }
        }
        j++;
    }
    if (!(counterPosition == numCounters || (counterPosition == numCounters - 1 && j == end))) {
        e = BCExceptionNotFound;
        return;
    }
}
void CBC_OneDReader::RecordPatternInReverse(CBC_CommonBitArray *row, FX_INT32 start, CFX_Int32Array *counters, FX_INT32 &e)
{
    FX_INT32 numTransitionsLeft = counters->GetSize();
    FX_BOOL last = row->Get(start);
    while (start > 0 && numTransitionsLeft >= 0) {
        if (row->Get(--start) != last) {
            numTransitionsLeft--;
            last = !last;
        }
    }
    if (numTransitionsLeft >= 0) {
        e = BCExceptionNotFound;
        return;
    }
    RecordPattern(row, start + 1, counters, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
}
FX_INT32 CBC_OneDReader::PatternMatchVariance(CFX_Int32Array *counters, const FX_INT32 *pattern, FX_INT32 maxIndividualVariance)
{
    FX_INT32 numCounters = counters->GetSize();
    FX_INT32 total = 0;
    FX_INT32 patternLength = 0;
    for (FX_INT32 i = 0; i < numCounters; i++) {
        total += (*counters)[i];
        patternLength += pattern[i];
    }
    if (total < patternLength) {
#undef max
        return FXSYS_IntMax;
    }
    FX_INT32 unitBarWidth = (total << INTEGER_MATH_SHIFT) / patternLength;
    maxIndividualVariance = (maxIndividualVariance * unitBarWidth) >> INTEGER_MATH_SHIFT;
    FX_INT32 totalVariance = 0;
    for (FX_INT32 x = 0; x < numCounters; x++) {
        FX_INT32 counter = (*counters)[x] << INTEGER_MATH_SHIFT;
        FX_INT32 scaledPattern = pattern[x] * unitBarWidth;
        FX_INT32 variance = counter > scaledPattern ? counter - scaledPattern : scaledPattern - counter;
        if (variance > maxIndividualVariance) {
#undef max
            return FXSYS_IntMax;
        }
        totalVariance += variance;
    }
    return totalVariance / total;
}
