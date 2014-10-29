// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_WRITER_H_
#define _BC_WRITER_H_
class CBC_Writer;
class CBC_Writer : public CFX_Object
{
public:
    CBC_Writer();
    virtual ~CBC_Writer();
    virtual FX_BYTE		*Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e) = 0;
    virtual FX_BYTE		*Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e) = 0;
    virtual FX_BOOL		SetCharEncoding(FX_INT32 encoding);
    virtual FX_BOOL		SetModuleHeight(FX_INT32 moduleHeight);
    virtual FX_BOOL		SetModuleWidth(FX_INT32 moduleWidth);
    virtual FX_BOOL		SetHeight(FX_INT32 height);
    virtual FX_BOOL		SetWidth(FX_INT32 width);
    virtual void		SetBackgroundColor(FX_ARGB backgroundColor);
    virtual void		SetBarcodeColor(FX_ARGB foregroundColor);
protected:
    CFX_DIBitmap*	CreateDIBitmap(FX_INT32 width, FX_INT32 height);
    FX_INT32		m_CharEncoding;
    FX_INT32		m_ModuleHeight;
    FX_INT32		m_ModuleWidth;
    FX_INT32		m_Height;
    FX_INT32		m_Width;
    FXDIB_Format	m_colorSpace;
    FX_ARGB			m_barColor;
    FX_ARGB			m_backgroundColor;
};
#endif
