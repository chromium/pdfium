// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDREADER_H_
#define _BC_ONEDREADER_H_
class CBC_Reader;
class CBC_BinaryBitmap;
class CBC_CommonBitArray;
class CBC_OneDReader;
class CBC_OneDReader : public CBC_Reader
{
public:
    CBC_OneDReader();
    virtual ~CBC_OneDReader();
    virtual CFX_ByteString Decode(CBC_BinaryBitmap *image, FX_INT32 &e);
    virtual CFX_ByteString Decode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e);
    virtual CFX_ByteString DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e)
    {
        return "";
    }
private:
    CFX_ByteString DeDecode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e);

protected:
    const static FX_INT32 INTEGER_MATH_SHIFT;
    const static FX_INT32 PATTERN_MATCH_RESULT_SCALE_FACTOR;
    void RecordPattern(CBC_CommonBitArray *row, FX_INT32 start, CFX_Int32Array *counters, FX_INT32 &e);
    void RecordPatternInReverse(CBC_CommonBitArray *row, FX_INT32 start, CFX_Int32Array *counters, FX_INT32 &e);
    FX_INT32 PatternMatchVariance(CFX_Int32Array *counters, const FX_INT32 *pattern, FX_INT32 maxIndividualVariance);
};
#endif
