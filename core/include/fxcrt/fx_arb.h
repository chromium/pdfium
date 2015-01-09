// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_ARABIC_
#define _FX_ARABIC_
class IFX_BidiChar
{
public:
    static IFX_BidiChar*	Create();
    virtual void			Release() = 0;
    virtual void			SetPolicy(FX_BOOL bSeparateNeutral = TRUE) = 0;
    virtual FX_BOOL			AppendChar(FX_WCHAR wch) = 0;
    virtual FX_BOOL			EndChar() = 0;
    virtual FX_INT32		GetBidiInfo(FX_INT32 &iStart, FX_INT32 &iCount) = 0;
    virtual void			Reset() = 0;

protected:
    ~IFX_BidiChar() { }
};
#endif
