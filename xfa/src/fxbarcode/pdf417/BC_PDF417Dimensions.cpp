// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2012 ZXing authors
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
#include "BC_PDF417Dimensions.h"
CBC_Dimensions::CBC_Dimensions(FX_INT32 minCols, FX_INT32 maxCols, FX_INT32 minRows, FX_INT32 maxRows)
{
    m_minCols = minCols;
    m_maxCols = maxCols;
    m_minRows = minRows;
    m_maxRows = maxRows;
}
CBC_Dimensions::~CBC_Dimensions()
{
}
FX_INT32 CBC_Dimensions::getMinCols()
{
    return m_minCols;
}
FX_INT32 CBC_Dimensions::getMaxCols()
{
    return m_maxCols;
}
FX_INT32 CBC_Dimensions::getMinRows()
{
    return m_minRows;
}
FX_INT32 CBC_Dimensions::getMaxRows()
{
    return m_maxRows;
}
