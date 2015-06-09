// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_PAGE_H_
#define _JBIG2_PAGE_H_
#include "JBig2_Image.h"
struct JBig2PageInfo : public CJBig2_Object {
    FX_DWORD m_dwWidth,
             m_dwHeight;
    FX_DWORD m_dwResolutionX,
             m_dwResolutionY;
    uint8_t m_cFlags;
    FX_BOOL m_bIsStriped;
    FX_WORD m_wMaxStripeSize;
};
#endif
