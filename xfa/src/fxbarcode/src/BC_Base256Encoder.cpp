// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2006-2007 Jeremias Maerki.
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
#include "include/BC_Encoder.h"
#include "include/BC_Dimension.h"
#include "include/BC_SymbolShapeHint.h"
#include "include/BC_SymbolInfo.h"
#include "include/BC_EncoderContext.h"
#include "include/BC_HighLevelEncoder.h"
#include "include/BC_Base256Encoder.h"
CBC_Base256Encoder::CBC_Base256Encoder()
{
}
CBC_Base256Encoder::~CBC_Base256Encoder()
{
}
FX_INT32 CBC_Base256Encoder::getEncodingMode()
{
    return BASE256_ENCODATION;
}
void CBC_Base256Encoder::Encode(CBC_EncoderContext &context, FX_INT32 &e)
{
    CFX_WideString buffer;
    buffer += (FX_WCHAR)'\0';
    while (context.hasMoreCharacters()) {
        FX_WCHAR c = context.getCurrentChar();
        buffer += c;
        context.m_pos++;
        FX_INT32 newMode = CBC_HighLevelEncoder::lookAheadTest(context.m_msg, context.m_pos, getEncodingMode());
        if (newMode != getEncodingMode()) {
            context.signalEncoderChange(newMode);
            break;
        }
    }
    FX_INT32 dataCount = buffer.GetLength() - 1;
    FX_CHAR buf[128];
#if defined(_FX_WINAPI_PARTITION_APP_)
    memset(buf, 0, sizeof(FX_CHAR) * 128);
    _itoa_s(dataCount, buf, 128, 10);
#else
    FXSYS_itoa(dataCount, buf, 10);
#endif
    buffer.SetAt(0, FX_WCHAR(*buf) - '0');
    FX_INT32 lengthFieldSize = 1;
    FX_INT32 currentSize = context.getCodewordCount() + dataCount + lengthFieldSize;
    context.updateSymbolInfo(currentSize, e);
    if (e != BCExceptionNO) {
        return;
    }
    FX_BOOL mustPad = (context.m_symbolInfo->m_dataCapacity - currentSize) > 0;
    if (context.hasMoreCharacters() || mustPad) {
        if (dataCount <= 249) {
            buffer.SetAt(0, (FX_WCHAR) dataCount);
        } else if (dataCount > 249 && dataCount <= 1555) {
            buffer.SetAt(0, (FX_WCHAR) ((dataCount / 250) + 249));
            buffer.Insert(1, (FX_WCHAR) (dataCount % 250));
        } else {
            e = BCExceptionIllegalStateMessageLengthInvalid;
            return;
        }
    }
    for (FX_INT32 i = 0, c = buffer.GetLength(); i < c; i++) {
        context.writeCodeword(randomize255State(buffer.GetAt(i), context.getCodewordCount() + 1));
    }
}
FX_WCHAR CBC_Base256Encoder::randomize255State(FX_WCHAR ch, FX_INT32 codewordPosition)
{
    FX_INT32 pseudoRandom = ((149 * codewordPosition) % 255) + 1;
    FX_INT32 tempVariable = ch + pseudoRandom;
    if (tempVariable <= 255) {
        return (FX_WCHAR) tempVariable;
    } else {
        return (FX_WCHAR) (tempVariable - 256);
    }
}
