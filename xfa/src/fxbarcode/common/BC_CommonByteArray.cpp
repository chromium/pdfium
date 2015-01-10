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
#include "BC_CommonByteArray.h"
CBC_CommonByteArray::CBC_CommonByteArray()
{
    m_bytes = NULL;
    m_size = 0;
    m_index = 0;
}
CBC_CommonByteArray::CBC_CommonByteArray(FX_INT32 size)
{
    m_size = size;
    m_bytes = FX_Alloc(FX_BYTE, size);
    FXSYS_memset32(m_bytes, 0, size);
    m_index = 0;
}
CBC_CommonByteArray::CBC_CommonByteArray(FX_BYTE* byteArray, FX_INT32 size)
{
    m_size = size;
    m_bytes = FX_Alloc(FX_BYTE, size);
    FXSYS_memcpy32(m_bytes, byteArray, size);
    m_index = size;
}
CBC_CommonByteArray::~CBC_CommonByteArray()
{
    if ( m_bytes != NULL) {
        FX_Free( m_bytes );
        m_bytes = NULL;
    }
    m_index = 0;
    m_size = 0;
}
FX_INT32 CBC_CommonByteArray::At(FX_INT32 index)
{
    return m_bytes[index] & 0xff;
}
void CBC_CommonByteArray::Set(FX_INT32 index, FX_INT32 value)
{
    m_bytes[index] = (FX_BYTE) value;
}
FX_INT32 CBC_CommonByteArray::Size()
{
    return m_size;
}
FX_BOOL CBC_CommonByteArray::IsEmpty()
{
    return m_size == 0;
}
void CBC_CommonByteArray::AppendByte(FX_INT32 value)
{
    if (m_size == 0 || m_index >= m_size) {
        FX_INT32 newSize = FX_MAX(32, m_size << 1);
        Reserve(newSize);
    }
    m_bytes[m_index] = (FX_BYTE)value;
    m_index++;
}
void CBC_CommonByteArray::Reserve(FX_INT32 capacity)
{
    if (m_bytes == NULL || m_size < capacity) {
        FX_BYTE *newArray = FX_Alloc(FX_BYTE, capacity);
        FXSYS_memset32(newArray, 0, capacity);
        if (m_bytes != NULL) {
            FXSYS_memcpy32(newArray, m_bytes, m_size);
            FX_Free( m_bytes );
        }
        m_bytes = newArray;
        m_size = capacity;
    }
}
void CBC_CommonByteArray::Set(FX_BYTE* source, FX_INT32 offset, FX_INT32 count)
{
    if (m_bytes != NULL) {
        FX_Free( m_bytes );
    }
    m_bytes = FX_Alloc(FX_BYTE, count);
    m_size = count;
    FXSYS_memcpy32(m_bytes, source + offset, count);
    m_index = count;
}
void CBC_CommonByteArray::Set(CFX_ByteArray* source, FX_INT32 offset, FX_INT32 count)
{
    if (m_bytes != NULL) {
        FX_Free( m_bytes );
    }
    m_bytes =  FX_Alloc(FX_BYTE, count);
    m_size = count;
    FX_INT32 i;
    for(i = 0; i < count; i++) {
        m_bytes[i] = source->operator [](i + offset);
    }
    m_index = m_size;
}
