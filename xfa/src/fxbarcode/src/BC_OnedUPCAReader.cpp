// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_OneDReader.h"
#include "include/BC_OneDimReader.h"
#include "include/BC_OnedEAN13Reader.h"
#include "include/BC_OnedUPCAReader.h"
CBC_OnedUPCAReader::CBC_OnedUPCAReader()
{
    m_ean13Reader = NULL;
}
void CBC_OnedUPCAReader::Init()
{
    m_ean13Reader = FX_NEW CBC_OnedEAN13Reader;
}
CBC_OnedUPCAReader::~CBC_OnedUPCAReader()
{
    if(m_ean13Reader != NULL) {
        delete m_ean13Reader;
    }
    m_ean13Reader = NULL;
}
CFX_ByteString CBC_OnedUPCAReader::DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e)
{
    CFX_ByteString bytestring = m_ean13Reader->DecodeRow(rowNumber, row, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CFX_ByteString temp = MaybeReturnResult(bytestring, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return temp;
}
CFX_ByteString CBC_OnedUPCAReader::DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, CFX_Int32Array *startGuardRange, FX_INT32 hints, FX_INT32 &e)
{
    CFX_ByteString bytestring = m_ean13Reader->DecodeRow(rowNumber, row, startGuardRange, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CFX_ByteString temp = MaybeReturnResult(bytestring, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return temp;
}
CFX_ByteString CBC_OnedUPCAReader::Decode(CBC_BinaryBitmap *image, FX_INT32 &e)
{
    CFX_ByteString bytestring = m_ean13Reader->Decode(image, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CFX_ByteString temp = MaybeReturnResult(bytestring, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return temp;
}
CFX_ByteString CBC_OnedUPCAReader::Decode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e)
{
    CFX_ByteString bytestring = m_ean13Reader->Decode(image, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CFX_ByteString temp = MaybeReturnResult(bytestring, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return temp;
}
FX_INT32 CBC_OnedUPCAReader::DecodeMiddle(CBC_CommonBitArray *row, CFX_Int32Array *startRange, CFX_ByteString &resultString, FX_INT32 &e)
{
    FX_INT32 temp = m_ean13Reader->DecodeMiddle(row, startRange, resultString, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    return temp;
}
CFX_ByteString CBC_OnedUPCAReader::MaybeReturnResult(CFX_ByteString &result, FX_INT32 &e)
{
    if(result[0] == '0') {
        result.Delete(0);
        return result;
    } else {
        e = BCExceptionFormatException;
        return "";
    }
    return "";
}
