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
#include "../BC_TwoDimWriter.h"
#include "../common/BC_CommonBitArray.h"
#include "../common/BC_CommonBitMatrix.h"
#include "BC_PDF417Compaction.h"
#include "BC_PDF417.h"
#include "BC_PDF417BarcodeMatrix.h"
#include "BC_PDF417Writer.h"
CBC_PDF417Writer::CBC_PDF417Writer()
{
    m_bFixedSize = FALSE;
}
CBC_PDF417Writer::~CBC_PDF417Writer()
{
    m_bTruncated = TRUE;
}
FX_BOOL	CBC_PDF417Writer:: SetErrorCorrectionLevel(FX_INT32 level)
{
    if (level < 0 || level > 8) {
        return FALSE;
    }
    m_iCorrectLevel = level;
    return TRUE;
}
void CBC_PDF417Writer::SetTruncated(FX_BOOL truncated)
{
    m_bTruncated = truncated;
}
FX_BYTE* CBC_PDF417Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    if ( format != BCFORMAT_PDF_417) {
        return NULL;
    }
    CFX_WideString encodeContents = contents.UTF8Decode();
    return Encode(encodeContents, outWidth, outHeight, e );
}
FX_BYTE* CBC_PDF417Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    return NULL;
}
FX_BYTE* CBC_PDF417Writer::Encode(const CFX_WideString &contents, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    CBC_PDF417 encoder;
    FX_INT32 col = (m_Width / m_ModuleWidth - 69) / 17;
    FX_INT32 row = m_Height / (m_ModuleWidth * 20);
    if (row >= 3 && row <= 90 && col >= 1 && col <= 30) {
        encoder.setDimensions(col, col, row, row);
    } else if (col >= 1 && col <= 30) {
        encoder.setDimensions(col, col, 90, 3);
    } else if (row >= 3 && row <= 90) {
        encoder.setDimensions(30, 1, row, row);
    }
    encoder.generateBarcodeLogic(contents, m_iCorrectLevel, e);
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    FX_INT32 lineThickness = 2;
    FX_INT32 aspectRatio = 4;
    CBC_BarcodeMatrix* barcodeMatrix = encoder.getBarcodeMatrix();
    CFX_ByteArray originalScale;
    originalScale.Copy(barcodeMatrix->getScaledMatrix(lineThickness, aspectRatio * lineThickness));
    FX_INT32 width = outWidth;
    FX_INT32 height = outHeight;
    outWidth = barcodeMatrix->getWidth();
    outHeight = barcodeMatrix->getHeight();
    FX_BOOL rotated = FALSE;
    if ((height > width) ^ (outWidth < outHeight)) {
        rotateArray(originalScale, outHeight, outWidth);
        rotated = TRUE;
        FX_INT32 temp = outHeight;
        outHeight = outWidth;
        outWidth = temp;
    }
    FX_INT32 scaleX = width / outWidth;
    FX_INT32 scaleY = height / outHeight;
    FX_INT32 scale;
    if (scaleX < scaleY) {
        scale = scaleX;
    } else {
        scale = scaleY;
    }
    if (scale > 1) {
        originalScale.RemoveAll();
        originalScale.Copy(barcodeMatrix->getScaledMatrix(scale * lineThickness, scale * aspectRatio * lineThickness));
        if (rotated) {
            rotateArray(originalScale, outHeight, outWidth);
            FX_INT32 temp = outHeight;
            outHeight = outWidth;
            outWidth = temp;
        }
    }
    FX_BYTE* result = (FX_BYTE*)FX_Alloc(FX_BYTE, outHeight * outWidth);
    FXSYS_memcpy32(result, originalScale.GetData(), outHeight * outWidth);
    return result;
}
void CBC_PDF417Writer::rotateArray(CFX_ByteArray& bitarray, FX_INT32 height, FX_INT32 width)
{
    CFX_ByteArray temp;
    temp.Copy(bitarray);
    for (FX_INT32 ii = 0; ii < height; ii++) {
        FX_INT32 inverseii = height - ii - 1;
        for (FX_INT32 jj = 0; jj < width; jj++) {
            bitarray[jj * height + inverseii] = temp[ii * width + jj];
        }
    }
}
