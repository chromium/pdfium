// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCRT_FX_ARABIC_H_
#define CORE_SRC_FXCRT_FX_ARABIC_H_

class CFX_BidiChar FX_FINAL : public IFX_BidiChar
{
public:
    CFX_BidiChar();
    virtual void		Release() FX_OVERRIDE
    {
        delete this;
    }
    virtual void		SetPolicy(FX_BOOL bSeparateNeutral = TRUE) FX_OVERRIDE
    {
        m_bSeparateNeutral = bSeparateNeutral;
    }
    virtual FX_BOOL		AppendChar(FX_WCHAR wch) FX_OVERRIDE;
    virtual FX_BOOL		EndChar() FX_OVERRIDE;
    virtual FX_INT32	GetBidiInfo(FX_INT32 &iStart, FX_INT32 &iCount) FX_OVERRIDE;
    virtual void		Reset() FX_OVERRIDE;

private:
    ~CFX_BidiChar() { }
    FX_BOOL		m_bSeparateNeutral;
    FX_INT32	m_iCurStart;
    FX_INT32	m_iCurCount;
    FX_INT32	m_iCurBidi;
    FX_INT32	m_iLastBidi;
    FX_INT32	m_iLastStart;
    FX_INT32	m_iLastCount;
};

#endif  // CORE_SRC_FXCRT_FX_ARABIC_H_
