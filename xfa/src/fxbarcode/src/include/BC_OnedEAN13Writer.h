// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDEAN13WRITER_H_
#define _BC_ONEDEAN13WRITER_H_
class CBC_OneDimWriter;
class CBC_OnedEAN13Writer;
class CBC_OnedEAN13Writer : public CBC_OneDimWriter
{
private:
    FX_INT32 m_codeWidth;
public:
    CBC_OnedEAN13Writer();
    virtual ~CBC_OnedEAN13Writer();

    FX_BYTE*	Encode(const CFX_ByteString &contents, BCFORMAT format,
                       FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE*	Encode(const CFX_ByteString &contents, BCFORMAT format,
                       FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e);
    FX_BYTE*	Encode(const CFX_ByteString &contents, FX_INT32 &outLength, FX_INT32 &e);
    void		RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength, FX_BOOL isDevice, FX_INT32 &e);
    FX_BOOL			CheckContentValidity(FX_WSTR contents);
    CFX_WideString	FilterContents(FX_WSTR contents);
    FX_INT32		CalcChecksum(const CFX_ByteString &contents);
protected:
    void		ShowChars(FX_WSTR contents, CFX_DIBitmap *pOutBitmap, CFX_RenderDevice* device, const CFX_Matrix* matrix, FX_INT32 barWidth, FX_INT32 multiple, FX_INT32 &e);
};
#endif
