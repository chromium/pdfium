// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_TARGET_IMP_H
#define _FWL_TARGET_IMP_H
class IFWL_Target;
class CFWL_Target;
class CFWL_Target : public CFX_Object
{
public:
    virtual FX_DWORD		Release();
    virtual IFWL_Target*	Retain();
    virtual	FX_DWORD		GetRefCount() const;
    virtual FWL_ERR			GetClassName(CFX_WideString &wsClass) const;
    virtual FX_DWORD		GetClassID() const;
    virtual FX_BOOL			IsInstance(FX_WSTR wsClass) const;
    virtual FWL_ERR			Initialize();
    virtual FWL_ERR			Finalize();
protected:
    CFWL_Target();
    virtual ~CFWL_Target();
    FX_DWORD	m_dwRefCount;
};
class IFWL_TargetData : public IFWL_Target
{
public:
    FX_LPVOID GetData();
    void	  SetData(FX_LPVOID pData);
};
#endif
