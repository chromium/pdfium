// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417HIGHLEVELENCODER_H_
#define _BC_PDF417HIGHLEVELENCODER_H_

#include "BC_PDF417Compaction.h"

class CBC_PDF417HighLevelEncoder : public CFX_Object
{
public:
    static CFX_WideString encodeHighLevel(CFX_WideString msg, Compaction compaction, FX_INT32 &e);
    static void Inverse();
    static void Initialize();
    static void Finalize();
private:
    static FX_INT32 TEXT_COMPACTION;
    static FX_INT32 BYTE_COMPACTION;
    static FX_INT32 NUMERIC_COMPACTION;
    static FX_INT32 SUBMODE_PUNCTUATION;
    static FX_INT32 LATCH_TO_TEXT;
    static FX_INT32 LATCH_TO_BYTE_PADDED;
    static FX_INT32 LATCH_TO_NUMERIC;
    static FX_INT32 SHIFT_TO_BYTE;
    static FX_INT32 LATCH_TO_BYTE;
    static FX_BYTE TEXT_MIXED_RAW[];
    static FX_BYTE TEXT_PUNCTUATION_RAW[];
    static FX_INT32 MIXED[128];
    static FX_INT32 PUNCTUATION[128];
    static FX_INT32 encodeText(CFX_WideString msg, FX_INT32 startpos, FX_INT32 count, CFX_WideString &sb, FX_INT32 initialSubmode);
    static void encodeBinary(CFX_ByteArray* bytes, FX_INT32 startpos, FX_INT32 count, FX_INT32 startmode, CFX_WideString &sb);
    static void encodeNumeric(CFX_WideString msg, FX_INT32 startpos, FX_INT32 count, CFX_WideString &sb);
    static FX_BOOL isDigit(FX_WCHAR ch);
    static FX_BOOL isAlphaUpper(FX_WCHAR ch);
    static FX_BOOL isAlphaLower(FX_WCHAR ch);
    static FX_BOOL isMixed(FX_WCHAR ch);
    static FX_BOOL isPunctuation(FX_WCHAR ch);
    static FX_BOOL isText(FX_WCHAR ch);
    static FX_INT32 determineConsecutiveDigitCount(CFX_WideString msg, FX_INT32 startpos);
    static FX_INT32 determineConsecutiveTextCount(CFX_WideString msg, FX_INT32 startpos);
    static FX_INT32 determineConsecutiveBinaryCount(CFX_WideString msg, CFX_ByteArray* bytes, FX_INT32 startpos, FX_INT32 &e);

    friend class PDF417HighLevelEncoder_EncodeNumeric_Test;
    friend class PDF417HighLevelEncoder_EncodeBinary_Test;
    friend class PDF417HighLevelEncoder_EncodeText_Test;
    friend class PDF417HighLevelEncoder_ConsecutiveDigitCount_Test;
    friend class PDF417HighLevelEncoder_ConsecutiveTextCount_Test;
    friend class PDF417HighLevelEncoder_ConsecutiveBinaryCount_Test;
};

#endif
