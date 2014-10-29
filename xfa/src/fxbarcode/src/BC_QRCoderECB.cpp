// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_QRCoderECB.h"
CBC_QRCoderECB::CBC_QRCoderECB(FX_INT32 count, FX_INT32 dataCodeWords)
{
    m_dataCodeWords = dataCodeWords;
    m_count = count;
}
CBC_QRCoderECB::~CBC_QRCoderECB()
{
}
FX_INT32 CBC_QRCoderECB::GetCount()
{
    return m_count;
}
FX_INT32 CBC_QRCoderECB::GetDataCodeWords()
{
    return m_dataCodeWords;
}
