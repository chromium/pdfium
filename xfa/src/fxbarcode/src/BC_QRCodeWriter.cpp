// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_TwoDimWriter.h"
#include "include/BC_Reader.h"
#include "include/BC_QRCodeWriter.h"
#include "include/BC_QRCoderEncoder.h"
#include "include/BC_QRCoder.h"
#include "include/BC_CommonByteMatrix.h"
#include "include/BC_QRCodeReader.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
CBC_QRCodeWriter::CBC_QRCodeWriter()
{
    m_bFixedSize = TRUE;
    m_iCorrectLevel = 1;
    m_iVersion = 0;
}
CBC_QRCodeWriter::~CBC_QRCodeWriter()
{
}
void CBC_QRCodeWriter::ReleaseAll()
{
    CBC_QRCodeReader::ReleaseAll();
}
FX_BOOL CBC_QRCodeWriter::SetVersion(FX_INT32 version)
{
    if (version < 0 || version > 40) {
        return FALSE;
    }
    m_iVersion = version;
    return TRUE;
}
FX_BOOL CBC_QRCodeWriter::SetErrorCorrectionLevel(FX_INT32 level)
{
    if (level < 0 || level > 3) {
        return FALSE;
    }
    m_iCorrectLevel = level;
    return TRUE;
}
FX_BYTE* CBC_QRCodeWriter::Encode(const CFX_WideString& contents, FX_INT32 ecLevel, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    CBC_QRCoderErrorCorrectionLevel *ec = NULL;
    switch(ecLevel) {
        case 0:
            ec = CBC_QRCoderErrorCorrectionLevel::L;
            break;
        case 1:
            ec = CBC_QRCoderErrorCorrectionLevel::M;
            break;
        case 2:
            ec = CBC_QRCoderErrorCorrectionLevel::Q;
            break;
        case 3:
            ec = CBC_QRCoderErrorCorrectionLevel::H;
            break;
        default: {
                e = BCExceptionUnSupportEclevel;
                BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            }
    }
    CBC_QRCoder qr;
    if (m_iVersion > 0 && m_iVersion < 41) {
        CFX_ByteString byteStr = contents.UTF8Encode();
        CBC_QRCoderEncoder::Encode(byteStr, ec, &qr, e, m_iVersion);
    } else {
        CBC_QRCoderEncoder::Encode(contents, ec, &qr, e);
    }
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    outWidth = qr.GetMatrixWidth();
    outHeight = qr.GetMatrixWidth();
    FX_BYTE* result = FX_Alloc(FX_BYTE, outWidth * outWidth);
    FXSYS_memcpy32(result, qr.GetMatrix()->GetArray(), outWidth * outHeight);
    return result;
}
FX_BYTE *CBC_QRCodeWriter::Encode(const CFX_ByteString& contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    return NULL;
}
FX_BYTE* CBC_QRCodeWriter::Encode(const CFX_ByteString& contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    return NULL;
}
