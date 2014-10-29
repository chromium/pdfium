// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Encoder.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_Dimension.h"
#include "include/BC_SymbolShapeHint.h"
#include "include/BC_SymbolInfo.h"
#include "include/BC_EncoderContext.h"
#include "include/BC_HighLevelEncoder.h"
#include "include/BC_C40Encoder.h"
#include "include/BC_TextEncoder.h"
CBC_TextEncoder::CBC_TextEncoder()
{
}
CBC_TextEncoder::~CBC_TextEncoder()
{
}
FX_INT32 CBC_TextEncoder::getEncodingMode()
{
    return TEXT_ENCODATION;
}
FX_INT32 CBC_TextEncoder::encodeChar(FX_WCHAR c, CFX_WideString &sb, FX_INT32 &e)
{
    if (c == ' ') {
        sb += (FX_WCHAR)'\3';
        return 1;
    }
    if (c >= '0' && c <= '9') {
        sb += (FX_WCHAR) (c - 48 + 4);
        return 1;
    }
    if (c >= 'a' && c <= 'z') {
        sb += (FX_WCHAR) (c - 97 + 14);
        return 1;
    }
    if (c >= '\0' && c <= 0x1f) {
        sb += (FX_WCHAR)'\0';
        sb += c;
        return 2;
    }
    if (c >= '!' && c <= '/') {
        sb += (FX_WCHAR) '\1';
        sb += (FX_WCHAR) (c - 33);
        return 2;
    }
    if (c >= ':' && c <= '@') {
        sb += (FX_WCHAR)'\1';
        sb += (FX_WCHAR) (c - 58 + 15);
        return 2;
    }
    if (c >= '[' && c <= '_') {
        sb += (FX_WCHAR) '\1';
        sb += (FX_WCHAR) (c - 91 + 22);
        return 2;
    }
    if (c == 0x0060) {
        sb += (FX_WCHAR) '\2';
        sb += (FX_WCHAR) (c - 96);
        return 2;
    }
    if (c >= 'A' && c <= 'Z') {
        sb += (FX_WCHAR)'\2';
        sb += (FX_WCHAR) (c - 65 + 1);
        return 2;
    }
    if (c >= '{' && c <= 0x007f) {
        sb += (FX_WCHAR)'\2';
        sb += (FX_WCHAR) (c - 123 + 27);
        return 2;
    }
    if (c >= 0x0080) {
        sb += (FX_WCHAR)'\1';
        sb += (FX_WCHAR)0x001e;
        FX_INT32 len = 2;
        len += encodeChar((FX_WCHAR) (c - 128), sb, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, -1);
        return len;
    }
    CBC_HighLevelEncoder::illegalCharacter(c, e);
    return -1;
}
