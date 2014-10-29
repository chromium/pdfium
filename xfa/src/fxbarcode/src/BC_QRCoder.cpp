// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
#include "include/BC_QRCoderMode.h"
#include "include/BC_CommonByteMatrix.h"
#include "include/BC_QRCoder.h"
CBC_QRCoder::CBC_QRCoder()
{
    m_mode = NULL;
    m_ecLevel = NULL;
    m_version = -1;
    m_matrixWidth = -1;
    m_maskPattern = -1;
    m_numTotalBytes = -1;
    m_numDataBytes = -1;
    m_numECBytes = -1;
    m_numRSBlocks = -1;
    m_matrix = NULL;
}
CBC_QRCoder::~CBC_QRCoder()
{
    if(m_matrix != NULL) {
        delete m_matrix;
        m_matrix = NULL;
    }
    m_mode = NULL;
    m_ecLevel = NULL;
    m_version = -1;
    m_matrixWidth = -1;
    m_maskPattern = -1;
    m_numTotalBytes = -1;
    m_numDataBytes = -1;
    m_numECBytes = -1;
    m_numRSBlocks = -1;
}
CBC_QRCoderMode* CBC_QRCoder::GetMode()
{
    return m_mode;
}
CBC_QRCoderErrorCorrectionLevel* CBC_QRCoder::GetECLevel()
{
    return m_ecLevel;
}
FX_INT32 CBC_QRCoder::GetVersion()
{
    return m_version;
}
FX_INT32 CBC_QRCoder::GetMatrixWidth()
{
    return m_matrixWidth;
}
FX_INT32 CBC_QRCoder::GetMaskPattern()
{
    return m_maskPattern;
}
FX_INT32 CBC_QRCoder::GetNumTotalBytes()
{
    return m_numTotalBytes;
}
FX_INT32 CBC_QRCoder::GetNumDataBytes()
{
    return m_numDataBytes;
}
FX_INT32 CBC_QRCoder::GetNumECBytes()
{
    return m_numECBytes;
}
FX_INT32 CBC_QRCoder::GetNumRSBlocks()
{
    return m_numRSBlocks;
}
CBC_CommonByteMatrix* CBC_QRCoder::GetMatrix()
{
    return m_matrix;
}
FX_INT32 CBC_QRCoder::At(FX_INT32 x, FX_INT32 y, FX_INT32 &e)
{
    FX_INT32 value = m_matrix->Get(x, y);
    if(!(value == 0 || value == 1)) {
        e = BCExceptionValueMustBeEither0or1;
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    }
    return value;
}
FX_BOOL CBC_QRCoder::IsValid()
{
    return
        m_mode != NULL &&
        m_ecLevel != NULL &&
        m_version != -1 &&
        m_matrixWidth != -1 &&
        m_maskPattern != -1 &&
        m_numTotalBytes != -1 &&
        m_numDataBytes != -1 &&
        m_numECBytes != -1 &&
        m_numRSBlocks != -1 &&
        IsValidMaskPattern(m_maskPattern) &&
        m_numTotalBytes == m_numDataBytes + m_numECBytes &&
        m_matrix != NULL &&
        m_matrixWidth == m_matrix->GetWidth() &&
        m_matrix->GetWidth() == m_matrix->GetHeight();
}
void CBC_QRCoder::SetMode(CBC_QRCoderMode* value)
{
    m_mode = value;
}
void CBC_QRCoder::SetECLevel(CBC_QRCoderErrorCorrectionLevel* ecLevel)
{
    m_ecLevel = ecLevel;
}
void CBC_QRCoder::SetVersion(FX_INT32 version)
{
    m_version = version;
}
void CBC_QRCoder::SetMatrixWidth(FX_INT32 width)
{
    m_matrixWidth = width;
}
void CBC_QRCoder::SetMaskPattern(FX_INT32 pattern)
{
    m_maskPattern = pattern;
}
void CBC_QRCoder::SetNumDataBytes(FX_INT32 bytes)
{
    m_numDataBytes = bytes;
}
void CBC_QRCoder::SetNumTotalBytes(FX_INT32 value)
{
    m_numTotalBytes = value;
}
void CBC_QRCoder::SetNumRSBlocks(FX_INT32 block)
{
    m_numRSBlocks = block;
}
void CBC_QRCoder::SetNumECBytes(FX_INT32 value)
{
    m_numECBytes = value;
}
FX_BOOL CBC_QRCoder::IsValidMaskPattern(FX_INT32 maskPattern)
{
    return maskPattern >= 0 && maskPattern < NUM_MASK_PATTERNS;
}
void CBC_QRCoder::SetMatrix(CBC_CommonByteMatrix* value)
{
    m_matrix = value;
}
const FX_INT32 CBC_QRCoder::NUM_MASK_PATTERNS = 8;
