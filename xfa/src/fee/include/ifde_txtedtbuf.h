// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _IFDE_TXTEDTBUF_H
#define _IFDE_TXTEDTBUF_H
#define FDE_DEFCHUNKLENGTH	(1024)
class IFDE_TxtEdtBuf;
class IFDE_TxtEdtBuf
{
public:
    virtual void		Release() = 0;

    virtual FX_BOOL		SetChunkSize(FX_INT32 nChunkSize) = 0;
    virtual FX_INT32	GetChunkSize() const = 0;
    virtual FX_INT32	GetTextLength() const = 0;
    virtual void		SetText(const CFX_WideString &wsText) = 0;
    virtual void		GetText(CFX_WideString &wsText) const = 0;
    virtual FX_WCHAR	GetCharByIndex(FX_INT32 nIndex) const = 0;
    virtual void		GetRange(CFX_WideString &wsText, FX_INT32 nBegin, FX_INT32 nCount = - 1) const = 0;

    virtual void		Insert(FX_INT32 nPos, FX_LPCWSTR lpText, FX_INT32 nLength = 1) = 0;
    virtual void		Delete(FX_INT32 nIndex, FX_INT32 nLength = 1) = 0;

    virtual void		Clear(FX_BOOL bRelease = TRUE) = 0;

    virtual FX_BOOL		Optimize(IFX_Pause * pPause = NULL) = 0;
};
#endif
