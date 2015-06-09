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
    virtual uint8_t*	Encode(const CFX_WideString& contents, int32_t ecLevel, int32_t &outWidth, int32_t &outHeight, int32_t  &e)
    {
        return NULL;
    };
    virtual uint8_t*	Encode(const CFX_ByteString& contents, BCFORMAT format, int32_t &outWidth, int32_t &outHeight, int32_t hints, int32_t &e)
    {
        return NULL;
    };
    virtual uint8_t*	Encode(const CFX_ByteString& contents, BCFORMAT format, int32_t &outWidth, int32_t &outHeight, int32_t &e)
    {
        return NULL;
    };
    virtual void		RenderResult(uint8_t *code, int32_t codeWidth, int32_t codeHeight, int32_t &e);
    virtual void        RenderBitmapResult(CFX_DIBitmap *&pOutBitmap, int32_t& e);
    virtual void        RenderDeviceResult(CFX_RenderDevice* device, const CFX_Matrix* matrix);
    virtual FX_BOOL		SetErrorCorrectionLevel (int32_t level) = 0;
    virtual int32_t    GetErrorCorrectionLevel()
    {
        return m_iCorrectLevel;
    };

protected:
    int32_t		m_iCorrectLevel;
    FX_BOOL			m_bFixedSize;
    CBC_CommonBitMatrix*     m_output;
};
#endif
