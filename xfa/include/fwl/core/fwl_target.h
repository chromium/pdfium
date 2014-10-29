// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_TARGET_H
#define _FWL_TARGET_H
class IFWL_Target;
class IFWL_Target
{
public:
    FX_DWORD		Release();
    IFWL_Target*	Retain();
    FX_DWORD		GetRefCount() const;
    FWL_ERR			GetClassName(CFX_WideString &wsClass) const;
    FX_DWORD		GetClassID() const;
    FX_BOOL			IsInstance(FX_WSTR wsClass) const;
    FWL_ERR			Initialize();
    FWL_ERR			Finalize();
protected:
    virtual ~IFWL_Target();
    FX_LPVOID	m_pData;
};
#endif
