// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
CBC_QRCoderErrorCorrectionLevel* CBC_QRCoderErrorCorrectionLevel::L = NULL;
CBC_QRCoderErrorCorrectionLevel* CBC_QRCoderErrorCorrectionLevel::M = NULL;
CBC_QRCoderErrorCorrectionLevel* CBC_QRCoderErrorCorrectionLevel::Q = NULL;
CBC_QRCoderErrorCorrectionLevel* CBC_QRCoderErrorCorrectionLevel::H = NULL;
CBC_QRCoderErrorCorrectionLevel::CBC_QRCoderErrorCorrectionLevel(FX_INT32 ordinal, FX_INT32 bits, FX_CHAR* name)
{
    m_name += name;
    m_ordinal = ordinal;
    m_bits = bits;
}
CBC_QRCoderErrorCorrectionLevel::~CBC_QRCoderErrorCorrectionLevel()
{
}
void CBC_QRCoderErrorCorrectionLevel::Initialize()
{
    L = FX_NEW CBC_QRCoderErrorCorrectionLevel(0, 0x01, (FX_CHAR*)"L");
    M = FX_NEW CBC_QRCoderErrorCorrectionLevel(1, 0x00, (FX_CHAR*)"M");
    Q = FX_NEW CBC_QRCoderErrorCorrectionLevel(2, 0x03, (FX_CHAR*)"Q");
    H = FX_NEW CBC_QRCoderErrorCorrectionLevel(3, 0x02, (FX_CHAR*)"H");
}
void CBC_QRCoderErrorCorrectionLevel::Finalize()
{
    delete L;
    delete M;
    delete Q;
    delete H;
}
FX_INT32 CBC_QRCoderErrorCorrectionLevel::Ordinal()
{
    return m_ordinal;
}
FX_INT32 CBC_QRCoderErrorCorrectionLevel::GetBits()
{
    return m_bits;
}
CFX_ByteString CBC_QRCoderErrorCorrectionLevel::GetName()
{
    return m_name;
}
CBC_QRCoderErrorCorrectionLevel* CBC_QRCoderErrorCorrectionLevel::ForBits(FX_INT32 bits)
{
    switch(bits) {
        case 0x00:
            return M;
        case 0x01:
            return L;
        case 0x02:
            return H;
        case 0x03:
            return Q;
        default:
            return NULL;
    }
}
void CBC_QRCoderErrorCorrectionLevel::Destroy()
{
    if(L) {
        delete CBC_QRCoderErrorCorrectionLevel::L;
        L = NULL;
    }
    if(M) {
        delete CBC_QRCoderErrorCorrectionLevel::M;
        M = NULL;
    }
    if(H) {
        delete CBC_QRCoderErrorCorrectionLevel::H;
        H = NULL;
    }
    if(Q) {
        delete CBC_QRCoderErrorCorrectionLevel::Q;
        Q = NULL;
    }
}
