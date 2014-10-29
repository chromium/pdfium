// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_CommonECI.h"
#include "include/BC_CommonCharacterSetECI.h"
void CBC_CommonCharacterSetECI::initialize()
{
}
CBC_CommonCharacterSetECI::CBC_CommonCharacterSetECI(FX_INT32 value, CFX_ByteString encodingName):
    CBC_CommonECI(value), m_encodingName(encodingName)
{
}
CBC_CommonCharacterSetECI::~CBC_CommonCharacterSetECI()
{
}
CFX_ByteString CBC_CommonCharacterSetECI::GetEncodingName()
{
    return m_encodingName;
}
void CBC_CommonCharacterSetECI::AddCharacterSet(FX_INT32 value, CFX_ByteString encodingName)
{
}
CBC_CommonCharacterSetECI* CBC_CommonCharacterSetECI::GetCharacterSetECIByValue(FX_INT32 value)
{
    return NULL;
}
CBC_CommonCharacterSetECI* CBC_CommonCharacterSetECI::GetCharacterSetECIByName(const CFX_ByteString& name)
{
    return NULL;
}
