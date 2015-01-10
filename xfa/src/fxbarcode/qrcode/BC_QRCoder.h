// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODER_H_
#define _BC_QRCODER_H_
class CBC_QRCoderErrorCorrectionLevel ;
class CBC_QRCoderMode;
class CBC_CommonByteMatrix;
class CBC_QRCoder;
class CBC_QRCoder : public CFX_Object
{
private:
    CBC_QRCoderMode* m_mode;
    CBC_QRCoderErrorCorrectionLevel* m_ecLevel;
    FX_INT32 m_version;
    FX_INT32 m_matrixWidth;
    FX_INT32 m_maskPattern;
    FX_INT32 m_numTotalBytes;
    FX_INT32 m_numDataBytes;
    FX_INT32 m_numECBytes;
    FX_INT32 m_numRSBlocks;
    CBC_CommonByteMatrix* m_matrix;
public:
    const static FX_INT32 NUM_MASK_PATTERNS;
    CBC_QRCoder();
    virtual ~CBC_QRCoder();
    CBC_QRCoderMode* GetMode();
    CBC_QRCoderErrorCorrectionLevel* GetECLevel();
    FX_INT32 GetVersion();
    FX_INT32 GetMatrixWidth();
    FX_INT32 GetMaskPattern();
    FX_INT32 GetNumTotalBytes();
    FX_INT32 GetNumDataBytes();
    FX_INT32 GetNumECBytes();
    FX_INT32 GetNumRSBlocks();
    CBC_CommonByteMatrix* GetMatrix();
    FX_INT32 At(FX_INT32 x, FX_INT32 y, FX_INT32 &e);
    FX_BOOL IsValid();

    void SetMode(CBC_QRCoderMode* value);
    void SetECLevel(CBC_QRCoderErrorCorrectionLevel* ecLevel);
    void SetVersion(FX_INT32 version);
    void SetMatrixWidth(FX_INT32 width);
    void SetMaskPattern(FX_INT32 pattern);
    void SetNumDataBytes(FX_INT32 bytes);
    void SetNumTotalBytes(FX_INT32 value);
    void SetNumECBytes(FX_INT32 value);
    void SetNumRSBlocks(FX_INT32 block);
    void SetMatrix(CBC_CommonByteMatrix* value);
    static FX_BOOL IsValidMaskPattern(FX_INT32 maskPattern);
};
#endif
