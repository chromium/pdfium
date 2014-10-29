// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_CommonECI.h"
#include "include/BC_CommonCharacterSetECI.h"
CBC_CommonECI::CBC_CommonECI(FX_INT32 value)
{
    m_value = value;
}
CBC_CommonECI::~CBC_CommonECI()
{
}
FX_INT32 CBC_CommonECI::GetValue()
{
    return m_value;
}
CBC_CommonECI* CBC_CommonECI::GetEICByValue(FX_INT32 value, FX_INT32 &e)
{
    if(value < 0 || value > 999999) {
        e = BCExceptionBadECI;
        return NULL;
    }
    if(value < 900) {
    }
    return NULL;
}
