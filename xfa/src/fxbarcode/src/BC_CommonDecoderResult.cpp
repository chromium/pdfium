// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
#include "include/BC_PDF417ResultMetadata.h"
#include "include/BC_CommonDecoderResult.h"
CBC_CommonDecoderResult::CBC_CommonDecoderResult()
{
}
void CBC_CommonDecoderResult::Init(const CFX_ByteArray &rawBytes, const CFX_ByteString &text, const CFX_Int32Array &byteSegments,  CBC_QRCoderErrorCorrectionLevel* ecLevel, FX_INT32 &e)
{
    if(text.IsEmpty()) {
        e = BCExceptionIllegalArgument;
        return;
    }
    m_rawBytes.Copy(rawBytes);
    m_text = text;
    m_byteSegments.Copy(byteSegments);
    m_ecLevel = ecLevel;
    m_other = NULL;
}
void CBC_CommonDecoderResult::Init(const CFX_ByteArray &rawBytes, const CFX_ByteString &text, const CFX_PtrArray &byteSegments, const CFX_ByteString &ecLevel, FX_INT32 &e)
{
    if(text.IsEmpty()) {
        e = BCExceptionIllegalArgument;
        return;
    }
    m_rawBytes.Copy(rawBytes);
    m_text = text;
    m_pdf417byteSegments.Copy(byteSegments);
    m_pdf417ecLevel = ecLevel;
    m_other = NULL;
}
void CBC_CommonDecoderResult::setOther(CBC_PDF417ResultMetadata* other)
{
    m_other = other;
}
CBC_CommonDecoderResult::~CBC_CommonDecoderResult()
{
    if (m_other != NULL) {
        delete m_other;
    }
}
const CFX_ByteArray& CBC_CommonDecoderResult::GetRawBytes()
{
    return m_rawBytes;
}
const CFX_Int32Array& CBC_CommonDecoderResult::GetByteSegments()
{
    return m_byteSegments;
}
const CFX_ByteString& CBC_CommonDecoderResult::GetText()
{
    return m_text;
}
CBC_QRCoderErrorCorrectionLevel* CBC_CommonDecoderResult::GetECLevel()
{
    return m_ecLevel;
}
