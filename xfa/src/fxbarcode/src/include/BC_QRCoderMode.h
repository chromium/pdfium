// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERMODE_H_
#define _BC_QRCODERMODE_H_
class CBC_QRCoderVersion;
class CBC_QRCoderMode;
class CBC_QRCoderMode  : public CFX_Object
{
private:
    FX_INT32* m_characterCountBitsForVersions;
    FX_INT32 m_bits;
    CFX_ByteString m_name;
    CBC_QRCoderMode(FX_INT32 *characterCountBitsForVersions, FX_INT32 x1, FX_INT32 x2, FX_INT32 x3, FX_INT32 bits, CFX_ByteString name);
    CBC_QRCoderMode();
public:
    static CBC_QRCoderMode* sBYTE;
    static CBC_QRCoderMode* sNUMERIC;
    static CBC_QRCoderMode* sALPHANUMERIC;
    static CBC_QRCoderMode* sKANJI;
    static CBC_QRCoderMode* sECI;
    static CBC_QRCoderMode* sGBK;
    static CBC_QRCoderMode* sTERMINATOR;
    static CBC_QRCoderMode* sFNC1_FIRST_POSITION;
    static CBC_QRCoderMode* sFNC1_SECOND_POSITION;
    static CBC_QRCoderMode* sSTRUCTURED_APPEND;
    virtual ~CBC_QRCoderMode();

    static void Initialize();
    static void Finalize();
    static CBC_QRCoderMode* ForBits(FX_INT32 bits, FX_INT32 &e);
    FX_INT32 GetCharacterCountBits(CBC_QRCoderVersion* version, FX_INT32 &e);
    FX_INT32 GetBits();
    CFX_ByteString GetName();
    static void Destroy();
};
#endif
