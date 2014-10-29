// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERENCODER_H_
#define _BC_QRCODERENCODER_H_
class Make_Pair;
class CBC_QRCoder;
class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderMode;
class CBC_QRCoderBitVector;
class CBC_CommonByteArray;
class CBC_CommonByteMatrix;
class CBC_QRCoderEncoder;
class CBC_QRCoderEncoder : public CFX_Object
{
private:
    const static FX_INT32 m_alphaNumbericTable[96];
public:
    CBC_QRCoderEncoder();
    virtual ~CBC_QRCoderEncoder();

    static void Encode(const CFX_ByteString &content, CBC_QRCoderErrorCorrectionLevel* ecLevel, CBC_QRCoder *qrCode, FX_INT32 &e, FX_INT32 versionSpecify = 0);
    static void Encode(const CFX_WideString &content, CBC_QRCoderErrorCorrectionLevel* ecLevel, CBC_QRCoder *qrCode, FX_INT32 &e);
    static void EncodeWithSpecifyVersion(const CFX_ByteString &content, CBC_QRCoderErrorCorrectionLevel* ecLevel,
                                         CBC_QRCoder *qrCode, FX_INT32 versionSpecify, FX_INT32 &e);
    static void EncodeWithAutoVersion(const CFX_ByteString &content, CBC_QRCoderErrorCorrectionLevel* ecLevel, CBC_QRCoder *qrCode, FX_INT32 &e);
    static CBC_QRCoderMode* ChooseMode(const CFX_ByteString & content, CFX_ByteString encoding);
    static FX_INT32 GetAlphaNumericCode(FX_INT32 code);
    static void AppendECI(CBC_QRCoderBitVector* bits);
    static void AppendBytes(const CFX_ByteString &content, CBC_QRCoderMode* mode, CBC_QRCoderBitVector* bits, CFX_ByteString encoding, FX_INT32 &e);
    static void AppendNumericBytes(const CFX_ByteString &content, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static void AppendAlphaNumericBytes(const CFX_ByteString &content, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static void Append8BitBytes(const CFX_ByteString &content, CBC_QRCoderBitVector* bits, CFX_ByteString encoding, FX_INT32 &e);
    static void Append8BitBytes(CFX_ByteArray &bytes, CBC_QRCoderBitVector *bits, FX_INT32 &e);
    static void AppendKanjiBytes(const CFX_ByteString &content, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static void AppendGBKBytes(const CFX_ByteString &content, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static void InitQRCode(FX_INT32 numInputBytes, FX_INT32 versionNumber,
                           CBC_QRCoderErrorCorrectionLevel* ecLevel, CBC_QRCoderMode* mode, CBC_QRCoder* qrCode, FX_INT32 &e);
    static void InitQRCode(FX_INT32 numInputBytes, CBC_QRCoderErrorCorrectionLevel* ecLevel, CBC_QRCoderMode* mode, CBC_QRCoder* qrCode, FX_INT32 &e);
    static void AppendModeInfo(CBC_QRCoderMode* mode, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static void AppendLengthInfo(FX_INT32 numLetters, FX_INT32 version, CBC_QRCoderMode* mode, CBC_QRCoderBitVector* bits, FX_INT32 &e);

    static void InterleaveWithECBytes(CBC_QRCoderBitVector* bits, FX_INT32 numTotalBytes, FX_INT32 numDataBytes, FX_INT32 numRSBlocks, CBC_QRCoderBitVector* result, FX_INT32 &e);
    static void GetNumDataBytesAndNumECBytesForBlockID(FX_INT32 numTotalBytes, FX_INT32 numDataBytes,
            FX_INT32 numRSBlocks, FX_INT32 blockID,
            FX_INT32 &numDataBytesInBlock, FX_INT32& numECBytesInBlocks);
    static CBC_CommonByteArray* GenerateECBytes(CBC_CommonByteArray* dataBytes, FX_INT32 numEcBytesInBlock, FX_INT32 &e);
    static FX_INT32 ChooseMaskPattern(CBC_QRCoderBitVector* bits, CBC_QRCoderErrorCorrectionLevel* ecLevel,
                                      FX_INT32 version, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static FX_INT32 CalculateMaskPenalty(CBC_CommonByteMatrix* matrix);
    static void TerminateBits(FX_INT32 numDataBytes, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static FX_INT32 GetSpanByVersion(CBC_QRCoderMode *modeFirst, CBC_QRCoderMode *modeSecond, FX_INT32 versionNum, FX_INT32 &e);
    static void MergeString(CFX_PtrArray &result, FX_INT32 versionNum, FX_INT32 &e);
    static void SplitString(const CFX_ByteString &content, CFX_PtrArray &result);
    static void AppendDataModeLenghInfo(CFX_PtrArray &splitResult, CBC_QRCoderBitVector &headerAndDataBits,	CBC_QRCoderMode *tempMode, CBC_QRCoder *qrCode, CFX_ByteString &encoding, FX_INT32 &e);
};
#endif
