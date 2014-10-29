// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BUFFEREDIMAGELUMINANCESOURCE_H_
#define _BC_BUFFEREDIMAGELUMINANCESOURCE_H_
class CBC_LuminanceSource;
class CBC_BufferedImageLuminanceSource;
class CBC_BufferedImageLuminanceSource : public CBC_LuminanceSource
{
public:
    CBC_BufferedImageLuminanceSource(const CFX_WideString &filename);
    CBC_BufferedImageLuminanceSource(CFX_DIBitmap *pBitmap);
    virtual ~CBC_BufferedImageLuminanceSource();
    CBC_LuminanceSource *RotateCounterClockwise(FX_INT32 &e);
    CBC_LuminanceSource *Crop(FX_INT32 left, FX_INT32 top, FX_INT32 width, FX_INT32 height);

    CFX_ByteArray *GetRow(FX_INT32 y, CFX_ByteArray &row, FX_INT32 &e);
    CFX_ByteArray *GetMatrix();
    FX_BOOL IsCropSupported();
    FX_BOOL IsRotateSupported();
    virtual void Init(FX_INT32 &e);
private:
    FX_INT32 m_bytesPerLine;
    FX_INT32 m_left;
    FX_INT32 m_top;
    CFX_Int32Array m_rgbData;
    CFX_DIBitmap *m_pBitmap;
    const CFX_WideString m_filename;
};
#endif
