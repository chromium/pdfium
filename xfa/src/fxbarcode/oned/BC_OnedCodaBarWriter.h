// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODABARWRITER_H_
#define _BC_ONEDCODABARWRITER_H_
enum BC_TEXT_LOC;
class CBC_OneDimWriter;
class CBC_OnedCodaBarWriter;
class CBC_OnedCodaBarWriter : public CBC_OneDimWriter
{
public:
    CBC_OnedCodaBarWriter();
    virtual ~CBC_OnedCodaBarWriter();
    FX_BYTE* Encode(const CFX_ByteString &contents, FX_INT32 &outLength, FX_INT32 &e);
    FX_BYTE *Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE *Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e);
    CFX_WideString          encodedContents(FX_WSTR contents);
    FX_BOOL					CheckContentValidity(FX_WSTR contents);
    CFX_WideString			FilterContents(FX_WSTR contents);
    FX_BOOL					SetStartChar(FX_CHAR start);
    FX_BOOL					SetEndChar(FX_CHAR end);
    void					SetDataLength(FX_INT32 length);
    FX_BOOL					SetTextLocation(BC_TEXT_LOC location);
    FX_BOOL					SetWideNarrowRatio(FX_INT32 ratio);
    FX_BOOL					FindChar(FX_WCHAR ch, FX_BOOL isContent);
private:
    void RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength, FX_BOOL isDevice, FX_INT32 &e);
    const static FX_CHAR START_END_CHARS[];
    const static FX_CHAR CONTENT_CHARS[];
    FX_CHAR			m_chStart;
    FX_CHAR			m_chEnd;
    FX_INT32		m_iWideNarrRatio;

};
#endif
