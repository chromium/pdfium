// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCRT_FX_ARABIC_H_
#define CORE_SRC_FXCRT_FX_ARABIC_H_

class CFX_BidiChar final : public IFX_BidiChar
{
public:
    CFX_BidiChar();
    virtual void		Release() override
    {
        delete this;
    }
    virtual void		SetPolicy(FX_BOOL bSeparateNeutral = TRUE) override
    {
        m_bSeparateNeutral = bSeparateNeutral;
    }
    virtual FX_BOOL		AppendChar(FX_WCHAR wch) override;
    virtual FX_BOOL		EndChar() override;
    virtual int32_t 	GetBidiInfo(int32_t &iStart, int32_t &iCount) override;
    virtual void		Reset() override;

private:
    ~CFX_BidiChar() { }
    FX_BOOL	m_bSeparateNeutral;
    int32_t	m_iCurStart;
    int32_t	m_iCurCount;
    int32_t	m_iCurBidi;
    int32_t	m_iLastBidi;
    int32_t	m_iLastStart;
    int32_t	m_iLastCount;
};

#endif  // CORE_SRC_FXCRT_FX_ARABIC_H_
