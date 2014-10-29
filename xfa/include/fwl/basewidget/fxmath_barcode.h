// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXMATH_BARCODE_H_
#define _FXMATH_BARCODE_H_
class IFX_Barcode
{
public:
    virtual void			Release() = 0;
    virtual BC_TYPE			GetType() = 0;
    virtual FX_BOOL			Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e) = 0;
    virtual FX_BOOL			RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e) = 0;
    virtual FX_BOOL			RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e) = 0;
    virtual CFX_WideString	Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 height, FX_INT32 &errorCode) = 0;
    virtual CFX_WideString	Decode(CFX_DIBitmap *pBitmap, FX_INT32 &errorCode) = 0;
    virtual FX_BOOL			SetCharEncoding(BC_CHAR_ENCODING encoding) = 0;
    virtual FX_BOOL			SetModuleHeight(FX_INT32 moduleHeight) = 0;
    virtual FX_BOOL			SetModuleWidth(FX_INT32 moduleWidth) = 0;
    virtual FX_BOOL			SetHeight(FX_INT32 height) = 0;
    virtual FX_BOOL			SetWidth(FX_INT32 width) = 0;
    virtual FX_BOOL			CheckContentValidity(FX_WSTR contents) = 0;
    virtual FX_BOOL			SetPrintChecksum(FX_BOOL checksum) = 0;
    virtual FX_BOOL			SetDataLength(FX_INT32 length) = 0;
    virtual FX_BOOL			SetCalChecksum(FX_INT32 state) = 0;
    virtual FX_BOOL			SetFont(CFX_Font* pFont) = 0;
    virtual FX_BOOL			SetFontSize(FX_FLOAT size) = 0;
    virtual FX_BOOL			SetFontStyle(FX_INT32 style) = 0;
    virtual FX_BOOL			SetFontColor(FX_ARGB color) = 0;
    virtual FX_BOOL			SetTextLocation(BC_TEXT_LOC location) = 0;
    virtual FX_BOOL			SetWideNarrowRatio(FX_INT32 ratio) = 0;
    virtual FX_BOOL			SetStartChar(FX_CHAR start) = 0;
    virtual FX_BOOL			SetEndChar(FX_CHAR end) = 0;
    virtual FX_BOOL			SetVersion(FX_INT32 version) = 0;
    virtual FX_BOOL			SetErrorCorrectionLevel (FX_INT32 level) = 0;
    virtual FX_BOOL			SetTruncated(FX_BOOL truncated) = 0;
};
IFX_Barcode*				FX_Barcode_Create(BC_TYPE type);
#endif
