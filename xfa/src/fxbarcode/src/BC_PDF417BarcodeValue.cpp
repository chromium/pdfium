// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_PDF417Common.h"
#include "include/BC_PDF417BarcodeValue.h"
CBC_BarcodeValue::CBC_BarcodeValue()
{
}
CBC_BarcodeValue::~CBC_BarcodeValue()
{
}
void CBC_BarcodeValue::setValue(FX_INT32 value)
{
    FX_INT32 confidence = 0;
    for (FX_INT32 i = 0; i < m_keys.GetSize(); i++) {
        if (m_keys.GetAt(i) == value) {
            confidence = m_values.GetAt(i);
            m_values.SetAt(i, confidence + 1);
            return;
        }
    }
    confidence = 1;
    m_keys.Add(value);
    m_values.Add(confidence);
}
CFX_Int32Array* CBC_BarcodeValue::getValue()
{
    FX_INT32 maxConfidence = -1;
    CFX_Int32Array* result = FX_NEW CFX_Int32Array;
    for (FX_INT32 i = 0; i < m_keys.GetSize(); i++) {
        if (m_values.GetAt(i) > maxConfidence) {
            maxConfidence = m_values.GetAt(i);
            result->RemoveAll();
            result->Add(m_keys.GetAt(i));
        } else if (m_values.GetAt(i) == maxConfidence) {
            result->Add(m_keys.GetAt(i));
        }
    }
    return result;
}
FX_INT32 CBC_BarcodeValue::getConfidence(FX_INT32 value)
{
    for (FX_INT32 i = 0; i < m_keys.GetSize(); i++)
        if (m_keys.GetAt(i) == value) {
            return m_values.GetAt(i);
        }
    return -1;
}
