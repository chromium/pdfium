// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
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

#include "barcode.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_QRDataMask.h"
static FX_INT32 N_DATA_MASKS = 0;
CFX_PtrArray* CBC_QRDataMask::DATA_MASKS = NULL;
void CBC_QRDataMask::Initialize()
{
    DATA_MASKS = FX_NEW CFX_PtrArray();
    N_DATA_MASKS = BuildDataMasks();
}
void CBC_QRDataMask::Finalize()
{
    Destroy();
    delete DATA_MASKS;
}
void CBC_QRDataMask::Destroy()
{
    FX_INT32 i;
    for(i = 0; i < N_DATA_MASKS; i++) {
        CBC_QRDataMask* p = (CBC_QRDataMask*)(*DATA_MASKS)[i];
        if (p) {
            delete p;
        }
    }
}
void CBC_QRDataMask::UnmaskBitMatirx(CBC_CommonBitMatrix *bits, FX_INT32 dimension)
{
    for(FX_INT32 i = 0; i < dimension; i++) {
        for(FX_INT32 j = 0; j < dimension; j++) {
            if(IsMasked(i, j)) {
                bits->Flip(j, i);
            }
        }
    }
}
CBC_QRDataMask* CBC_QRDataMask::ForReference(FX_INT32 reference, FX_INT32 &e)
{
    if(reference < 0 || reference > 7) {
        e = BCExceptionReferenceMustBeBetween0And7;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    return (CBC_QRDataMask*)(*DATA_MASKS)[reference];
}
class DataMask000 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        return ((x + y) % 2) == 0;
    }
};
class DataMask001 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        return (x % 2) == 0;
    }
};
class DataMask010 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        return y % 3 == 0;
    }
};
class DataMask011 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        return (x + y) % 3 == 0;
    }
};
class DataMask100 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        return (((x >> 1) + (y / 3)) % 2) == 0;
    }
};
class DataMask101 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        size_t temp = x * y;
        return (temp % 2) + (temp % 3) == 0;
    }
};
class DataMask110 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        size_t temp = x * y;
        return (((temp % 2) + (temp % 3)) % 2) == 0;
    }
};
class DataMask111 : public CBC_QRDataMask
{
public:
    FX_BOOL IsMasked(FX_INT32 x, FX_INT32 y)
    {
        return ((((x + y) % 2) + ((x * y) % 3)) % 2) == 0;
    }
};
FX_INT32 CBC_QRDataMask::BuildDataMasks()
{
    DATA_MASKS->Add(FX_NEW DataMask000);
    DATA_MASKS->Add(FX_NEW DataMask001);
    DATA_MASKS->Add(FX_NEW DataMask010);
    DATA_MASKS->Add(FX_NEW DataMask011);
    DATA_MASKS->Add(FX_NEW DataMask100);
    DATA_MASKS->Add(FX_NEW DataMask101);
    DATA_MASKS->Add(FX_NEW DataMask110);
    DATA_MASKS->Add(FX_NEW DataMask111);
    return DATA_MASKS->GetSize();
}
CBC_QRDataMask::CBC_QRDataMask()
{
}
CBC_QRDataMask::~CBC_QRDataMask()
{
}
