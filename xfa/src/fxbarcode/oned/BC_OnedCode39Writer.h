// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODA39WRITER_H_
#define _BC_ONEDCODA39WRITER_H_
enum BC_TEXT_LOC;
class CBC_OneDimWriter;
class CBC_OnedCoda39Writer;
class CBC_OnedCode39Writer : public CBC_OneDimWriter
{
public:
    CBC_OnedCode39Writer();
    CBC_OnedCode39Writer(FX_BOOL extendedMode);
    virtual ~CBC_OnedCode39Writer();
    FX_BYTE			*Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE			*Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e);
    FX_BYTE			*Encode(const CFX_ByteString &contents, FX_INT32 &outLength, FX_INT32 &e);
    void			RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength, FX_BOOL isDevice, FX_INT32 &e);
    CFX_WideString  encodedContents(FX_WSTR contents, FX_INT32 &e);
    FX_BOOL			CheckContentValidity(FX_WSTR contents);
    FX_BOOL         CheckExtendedContentValidity(FX_WSTR contents);
    CFX_WideString	FilterContents(FX_WSTR contents);
    CFX_WideString  FilterExtendedContents(FX_WSTR contents);
    CFX_WideString  RenderTextContents(FX_WSTR contents);
    CFX_WideString  RenderExtendedTextContents(FX_WSTR contents);
    FX_BOOL			SetTextLocation(BC_TEXT_LOC loction);
    FX_BOOL			SetWideNarrowRatio(FX_INT32 ratio);
private:
    void			ToIntArray(FX_INT32 a, FX_INT32 *toReturn);
    FX_CHAR			CalcCheckSum(const CFX_ByteString &contents, FX_INT32 &e);
    FX_INT32		m_iWideNarrRatio;
    FX_BOOL         m_extendedMode;
};
#endif
