// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_TWODIMWRITER_H_
#define _BC_TWODIMWRITER_H_
#include "BC_Writer.h"
class CBC_Writer;
class CBC_CommonBitMatrix;
class CBC_TwoDimWriter;
class CBC_TwoDimWriter : public CBC_Writer
{
public:
    CBC_TwoDimWriter();
    virtual ~CBC_TwoDimWriter();
    virtual FX_BYTE*	Encode(const CFX_WideString& contents, FX_INT32 ecLevel, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32  &e)
    {
        return NULL;
    };
    virtual FX_BYTE*	Encode(const CFX_ByteString& contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
    {
        return NULL;
    };
    virtual FX_BYTE*	Encode(const CFX_ByteString& contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
    {
        return NULL;
    };
    virtual void		RenderResult(FX_BYTE *code, FX_INT32 codeWidth, FX_INT32 codeHeight, FX_INT32 &e);
    virtual void        RenderBitmapResult(CFX_DIBitmap *&pOutBitmap, FX_INT32& e);
    virtual void        RenderDeviceResult(CFX_RenderDevice* device, const CFX_Matrix* matrix);
    virtual FX_BOOL		SetErrorCorrectionLevel (FX_INT32 level) = 0;
    virtual FX_INT32    GetErrorCorrectionLevel()
    {
        return m_iCorrectLevel;
    };

protected:
    FX_INT32		m_iCorrectLevel;
    FX_BOOL			m_bFixedSize;
    CBC_CommonBitMatrix*     m_output;
};
#endif
