// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_LuminanceSource.h"
CBC_LuminanceSource::CBC_LuminanceSource(FX_INT32 width, FX_INT32 height): m_width(width), m_height(height)
{
}
CBC_LuminanceSource::~CBC_LuminanceSource()
{
}
FX_INT32 CBC_LuminanceSource::GetWidth()
{
    return m_width;
}
FX_INT32 CBC_LuminanceSource::GetHeight()
{
    return m_height;
}
