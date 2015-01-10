// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODA128WRITER_H_
#define _BC_ONEDCODA128WRITER_H_
class CBC_OneDimWriter;
class CBC_OnedCoda128Writer;
class CBC_OnedCode128Writer : public CBC_OneDimWriter
{
public:
    CBC_OnedCode128Writer();
    CBC_OnedCode128Writer(BC_TYPE type);
    virtual ~CBC_OnedCode128Writer();
    FX_BYTE *					Encode(const CFX_ByteString &contents, BCFORMAT format,
                                       FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e);
    FX_BYTE *					Encode(const CFX_ByteString &contents, BCFORMAT format,
                                       FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE *					Encode(const CFX_ByteString &contents, FX_INT32 &outLength , FX_INT32 &e);
    FX_BOOL						CheckContentValidity(FX_WSTR contents);
    CFX_WideString				FilterContents(FX_WSTR contents);
    FX_BOOL						SetTextLocation(BC_TEXT_LOC location);
    BC_TYPE						GetType();
private:
    FX_BOOL IsDigits(const CFX_ByteString &contents, FX_INT32 start, FX_INT32 length);
    FX_INT32 Encode128B(const CFX_ByteString &contents, CFX_PtrArray &patterns);
    FX_INT32 Encode128C(const CFX_ByteString &contents, CFX_PtrArray &patterns);
    BC_TYPE						m_codeFormat;
    const static	FX_INT32	CODE_START_B;
    const static	FX_INT32	CODE_START_C;
    const static	FX_INT32	CODE_CODE_B;
    const static	FX_INT32	CODE_CODE_C;
    const static	FX_INT32	CODE_STOP;
};
#endif
