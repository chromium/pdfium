// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERVERSION_H_
#define _BC_QRCODERVERSION_H_
class CBC_QRCoderECBlocks;
class CBC_CommonBitMatrix;
class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderVersion;
class CBC_QRCoderVersion : public CFX_Object
{
private:
    const static FX_INT32 VERSION_DECODE_INFO[34];
    static CFX_PtrArray *VERSION;
    FX_INT32 m_versionNumber;
    FX_INT32 m_totalCodeWords;
    CFX_Int32Array m_alignmentPatternCenters;
    CFX_PtrArray m_ecBlocks;

    CBC_QRCoderVersion();
    CBC_QRCoderVersion(FX_INT32 versionNumber, CBC_QRCoderECBlocks* ecBlocks1, CBC_QRCoderECBlocks* ecBlocks2,
                       CBC_QRCoderECBlocks* ecBlocks3, CBC_QRCoderECBlocks* ecBlocks4);
public:
    virtual ~CBC_QRCoderVersion();
    static void Initialize();
    static void Finalize();

    FX_INT32 GetVersionNumber();
    FX_INT32 GetTotalCodeWords();
    FX_INT32 GetDimensionForVersion();
    CBC_CommonBitMatrix *BuildFunctionPattern(FX_INT32 &e);
    CFX_Int32Array* GetAlignmentPatternCenters();
    CBC_QRCoderECBlocks* GetECBlocksForLevel(CBC_QRCoderErrorCorrectionLevel *ecLevel);
    static CBC_QRCoderVersion* GetVersionForNumber(FX_INT32 versionNumber, FX_INT32 &e);
    static CBC_QRCoderVersion* GetProvisionalVersionForDimension(FX_INT32 dimension, FX_INT32 &e);
    static CBC_QRCoderVersion* DecodeVersionInformation(FX_INT32 versionBits, FX_INT32 &e);
    static void Destroy();
};
#endif
