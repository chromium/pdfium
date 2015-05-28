// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXGE_FX_GE_APPLE_H_
#define CORE_INCLUDE_FXGE_FX_GE_APPLE_H_

#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
class CFX_QuartzDevice : public CFX_RenderDevice
{
public:
    CFX_QuartzDevice();
    ~CFX_QuartzDevice();
    FX_BOOL Attach(CGContextRef context, FX_INT32 nDeviceClass = FXDC_DISPLAY);
    FX_BOOL Attach(CFX_DIBitmap* pBitmap);
    FX_BOOL Create(FX_INT32 width, FX_INT32 height, FXDIB_Format format);

    CGContextRef GetContext();

protected:
    CGContextRef m_pContext;
    FX_BOOL m_bOwnedBitmap;
};
#endif

#endif  // CORE_INCLUDE_FXGE_FX_GE_APPLE_H_
