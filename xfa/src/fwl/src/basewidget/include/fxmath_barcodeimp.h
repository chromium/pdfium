// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXMATH_BARCODEIMP_H_
#define _FXMATH_BARCODEIMP_H_
class CFX_Barcode : public IFX_Barcode, public CFX_Object
{
public:
    CFX_Barcode();
    ~CFX_Barcode();
    FX_BOOL					Crreate(BC_TYPE type);
    virtual void			Release();
    virtual BC_TYPE			GetType();
    virtual FX_BOOL			Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e);
    virtual FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e);
    virtual FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e);
    virtual CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 height, FX_INT32 &errorCode);
    virtual CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &errorCode);
    virtual FX_BOOL			SetCharEncoding(BC_CHAR_ENCODING encoding);
    virtual FX_BOOL			SetModuleHeight(FX_INT32 moduleHeight);
    virtual FX_BOOL			SetModuleWidth(FX_INT32 moduleWidth);
    virtual FX_BOOL			SetHeight(FX_INT32 height);
    virtual FX_BOOL			SetWidth(FX_INT32 width);
    virtual FX_BOOL			CheckContentValidity(FX_WSTR contents);
    virtual FX_BOOL			SetPrintChecksum(FX_BOOL checksum);
    virtual FX_BOOL			SetDataLength(FX_INT32 length);
    virtual FX_BOOL			SetCalChecksum(FX_INT32 state);
    virtual FX_BOOL			SetFont(CFX_Font* pFont);
    virtual FX_BOOL			SetFontSize(FX_FLOAT size);
    virtual FX_BOOL			SetFontStyle(FX_INT32 style);
    virtual FX_BOOL			SetFontColor(FX_ARGB color);
    virtual FX_BOOL			SetTextLocation(BC_TEXT_LOC location);
    virtual FX_BOOL			SetWideNarrowRatio(FX_INT32 ratio);
    virtual FX_BOOL			SetStartChar(FX_CHAR start);
    virtual FX_BOOL			SetEndChar(FX_CHAR end);
    virtual FX_BOOL			SetVersion(FX_INT32 version);
    virtual FX_BOOL			SetErrorCorrectionLevel(FX_INT32 level);
    virtual FX_BOOL			SetTruncated(FX_BOOL truncated);
protected:
    CBC_CodeBase*	m_pBCEngine;
};
#endif
