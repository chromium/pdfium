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
#include "include/BC_EdifactEncoder.h"
CBC_EdifactEncoder::CBC_EdifactEncoder()
{
}
CBC_EdifactEncoder::~CBC_EdifactEncoder()
{
}
FX_INT32 CBC_EdifactEncoder::getEncodingMode()
{
    return EDIFACT_ENCODATION;
}
void CBC_EdifactEncoder::Encode(CBC_EncoderContext &context, FX_INT32 &e)
{
    CFX_WideString buffer;
    while (context.hasMoreCharacters()) {
        FX_WCHAR c = context.getCurrentChar();
        encodeChar(c, buffer, e);
        if (e != BCExceptionNO) {
            return;
        }
        context.m_pos++;
        FX_INT32 count = buffer.GetLength();
        if (count >= 4) {
            context.writeCodewords(encodeToCodewords(buffer, 0, e));
            if (e != BCExceptionNO) {
                return;
            }
            buffer.Delete(0, 4);
            FX_INT32 newMode = CBC_HighLevelEncoder::lookAheadTest(context.m_msg, context.m_pos, getEncodingMode());
            if (newMode != getEncodingMode()) {
                context.signalEncoderChange(ASCII_ENCODATION);
                break;
            }
        }
    }
    buffer += (FX_WCHAR)31;
    handleEOD(context, buffer, e);
}
void CBC_EdifactEncoder::handleEOD(CBC_EncoderContext &context, CFX_WideString buffer, FX_INT32 &e)
{
    FX_INT32 count = buffer.GetLength();
    if (count == 0) {
        return;
    }
    if (count == 1) {
        context.updateSymbolInfo(e);
        if (e != BCExceptionNO) {
            return;
        }
        FX_INT32 available = context.m_symbolInfo->m_dataCapacity - context.getCodewordCount();
        FX_INT32 remaining = context.getRemainingCharacters();
        if (remaining == 0 && available <= 2) {
            return;
        }
    }
    if (count > 4) {
        e = BCExceptionIllegalStateCountMustNotExceed4;
        return;
    }
    FX_INT32 restChars = count - 1;
    CFX_WideString encoded = encodeToCodewords(buffer, 0, e);
    if (e != BCExceptionNO) {
        return;
    }
    FX_BOOL endOfSymbolReached = !context.hasMoreCharacters();
    FX_BOOL restInAscii = endOfSymbolReached && restChars <= 2;
    if (restChars <= 2) {
        context.updateSymbolInfo(context.getCodewordCount() + restChars, e);
        if (e != BCExceptionNO) {
            return;
        }
        FX_INT32 available = context.m_symbolInfo->m_dataCapacity - context.getCodewordCount();
        if (available >= 3) {
            restInAscii = FALSE;
            context.updateSymbolInfo(context.getCodewordCount() + encoded.GetLength(), e);
            if (e != BCExceptionNO) {
                return;
            }
        }
    }
    if (restInAscii) {
        context.resetSymbolInfo();
        context.m_pos -= restChars;
    } else {
        context.writeCodewords(encoded);
    }
    context.signalEncoderChange(ASCII_ENCODATION);
}
void CBC_EdifactEncoder::encodeChar(FX_WCHAR c, CFX_WideString &sb, FX_INT32 &e)
{
    if (c >= ' ' && c <= '?') {
        sb += c;
    } else if (c >= '@' && c <= '^') {
        sb += (FX_WCHAR)(c - 64);
    } else {
        CBC_HighLevelEncoder::illegalCharacter(c, e);
    }
}
CFX_WideString CBC_EdifactEncoder::encodeToCodewords(CFX_WideString sb, FX_INT32 startPos, FX_INT32 &e)
{
    FX_INT32 len = sb.GetLength() - startPos;
    if (len == 0) {
        e = BCExceptionNoContents;
        return (FX_LPWSTR)"";
    }
    FX_WCHAR c1 = sb.GetAt(startPos);
    FX_WCHAR c2 = len >= 2 ? sb.GetAt(startPos + 1) : 0;
    FX_WCHAR c3 = len >= 3 ? sb.GetAt(startPos + 2) : 0;
    FX_WCHAR c4 = len >= 4 ? sb.GetAt(startPos + 3) : 0;
    FX_INT32 v = (c1 << 18) + (c2 << 12) + (c3 << 6) + c4;
    FX_WCHAR cw1 = (FX_WCHAR) ((v >> 16) & 255);
    FX_WCHAR cw2 = (FX_WCHAR) ((v >> 8) & 255);
    FX_WCHAR cw3 = (FX_WCHAR) (v & 255);
    CFX_WideString res;
    res += cw1;
    if (len >= 2) {
        res += cw2;
    }
    if (len >= 3) {
        res += cw3;
    }
    return res;
}
