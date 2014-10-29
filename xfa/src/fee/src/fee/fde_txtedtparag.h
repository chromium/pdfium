// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTPARAG_H
#define _FDE_TXTEDTPARAG_H
class CFDE_TxtEdtEngine;
class CFDE_TxtEdtParag;
class CFDE_TxtEdtParag : public IFDE_TxtEdtParag, public CFX_Object
{
public:
    CFDE_TxtEdtParag(CFDE_TxtEdtEngine * pEngine);
    ~CFDE_TxtEdtParag();
    virtual FX_INT32			GetTextLength() const
    {
        return m_nCharCount;
    }
    virtual	FX_INT32			GetStartIndex() const
    {
        return m_nCharStart;
    }
    virtual FX_INT32			CountLines() const
    {
        return m_nLineCount;
    }
    virtual void				GetLineRange(FX_INT32 nLineIndex, FX_INT32& nStart, FX_INT32& nCount) const;
    void	LoadParag();
    void	UnloadParag();
    void	CalcLines();
    FX_INT32	m_nCharStart;
    FX_INT32	m_nCharCount;
    FX_INT32	m_nLineCount;
private:
    FX_LPVOID			m_lpData;
    CFDE_TxtEdtEngine*	m_pEngine;
};
#endif
