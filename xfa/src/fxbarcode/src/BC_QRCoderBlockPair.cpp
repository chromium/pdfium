// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_CommonByteArray.h"
#include "include/BC_QRCoderBlockPair.h"
CBC_QRCoderBlockPair::CBC_QRCoderBlockPair(CBC_CommonByteArray* data, CBC_CommonByteArray* errorCorrection)
{
    m_dataBytes = data;
    m_errorCorrectionBytes = errorCorrection;
}
CBC_QRCoderBlockPair::~CBC_QRCoderBlockPair()
{
    if(m_dataBytes != NULL) {
        delete m_dataBytes;
        m_dataBytes = NULL;
    }
    if(m_errorCorrectionBytes != NULL) {
        delete m_errorCorrectionBytes;
        m_errorCorrectionBytes = NULL;
    }
}
CBC_CommonByteArray* CBC_QRCoderBlockPair::GetDataBytes()
{
    return m_dataBytes;
}
CBC_CommonByteArray* CBC_QRCoderBlockPair::GetErrorCorrectionBytes()
{
    return m_errorCorrectionBytes;
}
