// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_LuminanceSource.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_Binarizer.h"
CBC_Binarizer::CBC_Binarizer(CBC_LuminanceSource *source)
{
    m_source = source;
}
CBC_Binarizer::~CBC_Binarizer()
{
}
CBC_LuminanceSource *CBC_Binarizer::GetLuminanceSource()
{
    return m_source;
}
