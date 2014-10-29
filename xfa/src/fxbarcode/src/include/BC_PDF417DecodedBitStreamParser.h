// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DECODEDBITSTREAMPARSER_H_
#define _BC_DECODEDBITSTREAMPARSER_H_
class CBC_CommonDecoderResult;
class CBC_PDF417ResultMetadata;
class CBC_DecodedBitStreamPaser;
class CBC_DecodedBitStreamPaser : public CFX_Object
{
public:
    CBC_DecodedBitStreamPaser();
    virtual ~CBC_DecodedBitStreamPaser();
    static void Initialize();
    static void Finalize();
    static CBC_CommonDecoderResult* decode(CFX_Int32Array &codewords, CFX_ByteString ecLevel, FX_INT32 &e);

private:
    enum Mode {
        ALPHA,
        LOWER,
        MIXED,
        PUNCT,
        ALPHA_SHIFT,
        PUNCT_SHIFT
    };
    static FX_INT32 MAX_NUMERIC_CODEWORDS;
    static FX_INT32 PL;
    static FX_INT32 LL;
    static FX_INT32 AS;
    static FX_INT32 ML;
    static FX_INT32 AL;
    static FX_INT32 PS;
    static FX_INT32 PAL;
    static FX_CHAR PUNCT_CHARS[29];
    static FX_CHAR MIXED_CHARS[30];
    static FX_INT32 EXP900[16];
    static FX_INT32 NUMBER_OF_SEQUENCE_CODEWORDS;
    static FX_INT32 decodeMacroBlock(CFX_Int32Array &codewords, FX_INT32 codeIndex, CBC_PDF417ResultMetadata* resultMetadata, FX_INT32 &e);
    static FX_INT32 textCompaction(CFX_Int32Array &codewords, FX_INT32 codeIndex, CFX_ByteString &result);
    static void decodeTextCompaction(CFX_Int32Array &textCompactionData, CFX_Int32Array &byteCompactionData, FX_INT32 length, CFX_ByteString &result);
    static FX_INT32 byteCompaction(FX_INT32 mode, CFX_Int32Array &codewords, FX_INT32 codeIndex, CFX_ByteString &result);
    static FX_INT32 numericCompaction(CFX_Int32Array &codewords, FX_INT32 codeIndex, CFX_ByteString &result, FX_INT32 &e);
    static CFX_ByteString decodeBase900toBase10(CFX_Int32Array &codewords, FX_INT32 count, FX_INT32 &e);
};
#endif
