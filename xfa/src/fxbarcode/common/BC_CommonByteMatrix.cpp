// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
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

#include "../barcode.h"
#include "BC_CommonByteMatrix.h"
CBC_CommonByteMatrix::CBC_CommonByteMatrix(FX_INT32 width, FX_INT32 height)
{
    m_height = height;
    m_width = width;
    m_bytes = NULL;
}
void CBC_CommonByteMatrix::Init()
{
    m_bytes = FX_Alloc(FX_BYTE, m_height * m_width);
    FXSYS_memset8(m_bytes, 0xff, m_height * m_width);
}
CBC_CommonByteMatrix::~CBC_CommonByteMatrix()
{
    if(m_bytes != NULL) {
        FX_Free(m_bytes);
        m_bytes = NULL;
    }
}
FX_INT32 CBC_CommonByteMatrix::GetHeight()
{
    return m_height;
}
FX_INT32 CBC_CommonByteMatrix::GetWidth()
{
    return m_width;
}
FX_BYTE CBC_CommonByteMatrix::Get(FX_INT32 x, FX_INT32 y)
{
    return m_bytes[y * m_width + x];
}
void CBC_CommonByteMatrix::Set(FX_INT32 x, FX_INT32 y, FX_INT32 value)
{
    m_bytes[y * m_width + x] = (FX_BYTE)value;
}
void CBC_CommonByteMatrix::Set(FX_INT32 x, FX_INT32 y, FX_BYTE value)
{
    m_bytes[y * m_width + x] = value;
}
void CBC_CommonByteMatrix::clear(FX_BYTE value)
{
    FX_INT32 y;
    for(y = 0; y < m_height; y++) {
        FX_INT32 x;
        for(x = 0; x < m_width; x++) {
            m_bytes[y * m_width + x] = value;
        }
    }
}
FX_BYTE* CBC_CommonByteMatrix::GetArray()
{
    return m_bytes;
}
